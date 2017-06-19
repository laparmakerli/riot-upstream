//===- ProtectionPass.cpp - Memory Access Instrumentation for RioT OS ------------===//
//
// Basic structure taken from :: 
// https://llvm.org/svn/llvm-project/safecode/trunk/lib/CommonMemorySafety/InstrumentMemoryAccesses.cpp
//
//===----------------------------------------------------------------------===//
//
// 1. 
// instruments loads, stores, and other memory intrinsics with
// load/store checks by inserting the relevant __loadcheck and/or
// __storecheck calls before the them.
//
// 
// 2. 
// add Stackpointer check after each function prologue to detect
// stackoverflows
//
//
// 3. 
// add Stackpointer check before each call site with more than 4
// arguments if we have a dynamic alloca in that function
//
//===----------------------------------------------------------------------===//




#define DEBUG_TYPE "instrument-memory-accesses"

#include "CommonMemorySafetyPasses.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Pass.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/Transforms/Instrumentation.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Operator.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Transforms/Utils/PromoteMemToReg.h"
#include <algorithm>
#include <list>



using namespace llvm;

STATISTIC(LoadsInstrumented, "Loads instrumented");
STATISTIC(StoresInstrumented, "Stores instrumented");
STATISTIC(AtomicsInstrumented, "Atomic memory intrinsics instrumented");
STATISTIC(IntrinsicsInstrumented, "Block memory intrinsics instrumented");

struct memAccess {
  Value *Address;
  uint64_t TypeSize;
  bool operator==(const memAccess& mA) const {
      return mA.Address == this->Address && mA.TypeSize == this->TypeSize;
  }
}; 

namespace {
  class ProtectionPass : public FunctionPass,
                                   public InstVisitor<ProtectionPass> {
    const DataLayout *TD;
    IRBuilder<> *Builder;
    ObjectSizeOffsetVisitor *ObjSizeVis;

    Type *VoidTy;
    PointerType *VoidPtrTy;
    IntegerType *SizeTy;
    IntegerType *Int32Ty;

    //   Instrumentation Functions
    Function *LoadCheckFunction;
    Function *StoreCheckFunction;
    Function *IrqInFunction;
    Constant *StackOverflowedFunction;

    GlobalVariable* lower_stack_bound;

    bool dynAlloca = false;

    //   collect all accesses that we have
    //   already seen
    std::list<memAccess> already_seen;

    //   collect calls with large arguments
    //   maybe we have to instrument
    //   the call later
    std::list<CallInst*> large_calls;

    void instrument(Value *Pointer, Value *AccessSize, Function *Check,
                    Instruction &I);

  public:
    static char ID;
    ProtectionPass(): FunctionPass(ID) { }
    virtual bool doInitialization(Module &M);
    virtual bool runOnFunction(Function &F);


    virtual StringRef getPassName() const {
      return "ProtectionPass";
    }

    Constant *instrFuncLoad;
    Constant *instrFuncStore;

    // Visitor methods
    void visitLoadInst(LoadInst &LI);
    void visitStoreInst(StoreInst &SI);
    void visitAtomicCmpXchgInst(AtomicCmpXchgInst &I);
    void visitAtomicRMWInst(AtomicRMWInst &I);
    void visitMemIntrinsic(MemIntrinsic &MI);

    // check if we have dynamic allocas
    void visitAllocaInst(AllocaInst &AI);

    // collect call sites. maybe we'll
    // have to instrument them later
    void visitCallInst (CallInst &I);

    // Not used by now. Maybe useful later
    void instrumentWithIRQCheck(Function &F);

    // add stackpointer check in function prologue
    void instrumentWithStackponterCheck(Function &F);

    // add stackpointer check before long call sites
    void instrumentCallSiteWithStackponterCheck(Function &F);

    // Optimization method
    bool isAllocated(Value *Address, uint64_t TypeSize);

    // Stopgap to save usart data
    bool calledByUsart(Function &F);


    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.addRequired<DominatorTreeWrapperPass>();
      AU.addRequired<TargetLibraryInfoWrapperPass>();
    }

  };
} 


bool ProtectionPass::doInitialization(Module &M) {
  VoidTy = Type::getVoidTy(M.getContext());
  VoidPtrTy = Type::getInt8PtrTy(M.getContext());
  SizeTy = IntegerType::getInt64Ty(M.getContext());
  Int32Ty = IntegerType::getInt32Ty(M.getContext());


  // add declaration for lower stack bound 

  lower_stack_bound = M.getGlobalVariable("lower_stack_bound");

  if (!lower_stack_bound){
    errs() << "######## Create lower_stack_bound\n";
    lower_stack_bound = new GlobalVariable(M, 
        Int32Ty,
        false,
        GlobalValue::ExternalLinkage,
        0,
        "lower_stack_bound");
    lower_stack_bound->setAlignment(2);
  }

  return true;
}

bool ProtectionPass::runOnFunction(Function &F) {
  
  // initialize Builder, DataLayout und Visitor

  TD = &(F.getParent()->getDataLayout());
  IRBuilder<> TheBuilder(F.getContext());
  Builder = &TheBuilder;
  const TargetLibraryInfo *TLI =
      &getAnalysis<TargetLibraryInfoWrapperPass>().getTLI();

  ObjectSizeOffsetVisitor TheObjSizeVis(*TD, TLI, F.getContext(), true);
  ObjSizeVis = &TheObjSizeVis;

  LLVMContext &Context = F.getContext();


  // Add function declarations for instrumentation functions

  Constant *instrFuncLoad = F.getParent()->getOrInsertFunction(
        "__loadcheck", VoidTy, VoidPtrTy, SizeTy, NULL
      );
  Constant *instrFuncStore = F.getParent()->getOrInsertFunction(
        "__storecheck", VoidTy, VoidPtrTy, SizeTy, NULL
      );

  //AttributeSet func_Attr = AttributeSet().addAttribute(F.getParent()->getContext(), AttributeSet::FunctionIndex,Attribute::NoInline);

  Constant *instrFunc = F.getParent()->getOrInsertFunction(
    "__stackoverflow", /*func_Attr,*/ VoidTy, NULL
  );


  // Check that the declaration was successfull

  LoadCheckFunction = F.getParent()->getFunction("__loadcheck");
  assert(LoadCheckFunction && "__loadcheck function has disappeared!\n");

  StoreCheckFunction = F.getParent()->getFunction("__storecheck");
  assert(StoreCheckFunction && "__storecheck function has disappeared!\n");

  StackOverflowedFunction = F.getParent()->getFunction("__stackoverflow");
  assert(StackOverflowedFunction && "__stackoverflow function has disappeared!\n");


  // stopgap - leave out usart functions

  if (calledByUsart(F)){
    return false;
  }

  //  IRQ Only Functions -   

  StringRef irq_handler_string_ref("irq_handler");
  StringRef tsrb_full_string_ref("tsrb_full");
  StringRef _push_string_ref("_push");
  StringRef NVIC_SetPendingIRQ_string_ref("NVIC_SetPendingIRQ");
  StringRef tsrb_add_one_string_ref("tsrb_add_one");
  StringRef uart_stdio_rx_cb_string_ref("uart_stdio_rx_cb");
  StringRef thread_yield_string_ref("uart_stdio_rx_cb");
  StringRef isr_usart1_string_ref("isr_usart1");


  //  instrumentation function

  StringRef irq_arch_in_string_ref("irq_arch_in");

  //  init functions

  StringRef reset_handler_default_string_ref("reset_handler_default");
  StringRef init_thread_blocks_string_ref("init_thread_blocks");
  StringRef pre_startup_string_ref("pre_startup");

  

  // instrument IRQ only with IRQ check after function prologue

  if ( F.getName() == isr_usart1_string_ref   
    || F.getName() == irq_handler_string_ref
    || F.getName() == tsrb_full_string_ref
    || F.getName() == _push_string_ref 
    || F.getName() == NVIC_SetPendingIRQ_string_ref
    || F.getName() == tsrb_add_one_string_ref
    || F.getName() == uart_stdio_rx_cb_string_ref
    || F.getName() == thread_yield_string_ref ) {
    instrumentWithIRQCheck(F);
    return true;
  }

  // leave out instrumentation functions and init functions

  if ( F.getName() == LoadCheckFunction->getName() || F.getName() == StoreCheckFunction->getName() 
    || F.getName() == StackOverflowedFunction->getName()  || F.getName() == irq_arch_in_string_ref 
    || F.getName() == reset_handler_default_string_ref || F.getName() == init_thread_blocks_string_ref 
    || F.getName() == pre_startup_string_ref){
    return false;
  }



  // Instrument remaining functions with store/loadchecks 
  // Visit all of the instructions in the function for instrumentation

  visit(F);

  //  insert Stackpointercheck 

  instrumentWithStackponterCheck(F);

  //  Insert SP Call Inst check

  if (dynAlloca){
    instrumentCallSiteWithStackponterCheck(F);
  }
  return true;
}



void ProtectionPass::instrument(Value *Pointer, Value *AccessSize,
                                          Function *Check, Instruction &I) {

  Builder->SetInsertPoint(&I);
  Value *VoidPointer = Builder->CreatePointerCast(Pointer, VoidPtrTy);
  CallInst *CI = Builder->CreateCall(Check, {VoidPointer, AccessSize});

}

void ProtectionPass::visitLoadInst(LoadInst &LI) {

  Value *AccessSize = ConstantInt::get(SizeTy,
                                       TD->getTypeStoreSize(LI.getType()));
  uint64_t TypeSize = TD->getTypeStoreSizeInBits(LI.getType());

  // leave out safe accesses to global variables

  GlobalVariable *G = dyn_cast<GlobalVariable>(GetUnderlyingObject(LI.getPointerOperand(), *TD));
  if (G) {

    // test if other accesses are inside allocated area

    if (isAllocated(LI.getPointerOperand(), TypeSize)){
      errs() << "Is safe!\n";
      return;
    }
  }

  // leave out safe accesses to allocas 

  AllocaInst *AI = dyn_cast_or_null<AllocaInst>(GetUnderlyingObject(LI.getPointerOperand(), *TD));  
  if (AI){       
    if (isAllocated(LI.getPointerOperand(), TypeSize)){
      errs() << "Is safe!\n";
      return;
    }
  }

  //  instrument remaining accesses

  instrument(LI.getPointerOperand(), AccessSize, LoadCheckFunction, LI);
  ++LoadsInstrumented;

}



void ProtectionPass::visitStoreInst(StoreInst &SI) {

  uint64_t Bytes = TD->getTypeStoreSize(SI.getValueOperand()->getType());
  Value *AccessSize = ConstantInt::get(SizeTy, Bytes);
  uint64_t TypeSize = TD->getTypeStoreSizeInBits(SI.getValueOperand()->getType());

  // leave out safe accesses to global variables

  GlobalVariable *G = dyn_cast<GlobalVariable>(GetUnderlyingObject(SI.getPointerOperand(), *TD));
  if (G) {

    // store accesses to .kernel section variables are never safe / Instrument it

    if ( G->getSection() == "._kernel_space"){
      instrument(SI.getPointerOperand(), AccessSize, StoreCheckFunction, SI);
      ++LoadsInstrumented;
      return;
    }

    //   test if other accesses are inside allocated area

    if (isAllocated(SI.getPointerOperand(), TypeSize)){
      errs() << "Is safe!\n";
      return;
    }
  }

  // leave out safe accesses to allocas 

  AllocaInst *AI = dyn_cast_or_null<AllocaInst>(GetUnderlyingObject(SI.getPointerOperand(), *TD));  
  if (AI) {
    if (isAllocated(SI.getPointerOperand(), TypeSize)){
      errs() << "Is safe!\n";
      return;
    }
  }

  //  instrument remaining accesses

  instrument(SI.getPointerOperand(), AccessSize, StoreCheckFunction, SI);
  ++LoadsInstrumented;

}



void ProtectionPass::visitAtomicRMWInst(AtomicRMWInst &I) {

  // Instrument an AtomicRMW instruction with a store check.
  Value *AccessSize = ConstantInt::get(SizeTy,
                                       TD->getTypeStoreSize(I.getType()));
  instrument(I.getPointerOperand(), AccessSize, StoreCheckFunction, I);
  ++AtomicsInstrumented;
}

void ProtectionPass::visitAtomicCmpXchgInst(AtomicCmpXchgInst &I) {

  // Instrument an AtomicCmpXchg instruction with a store check.
  Value *AccessSize = ConstantInt::get(SizeTy,
                                       TD->getTypeStoreSize(I.getType()));
  instrument(I.getPointerOperand(), AccessSize, StoreCheckFunction, I);
  ++AtomicsInstrumented;
}

void ProtectionPass::visitMemIntrinsic(MemIntrinsic &MI) {

  // Instrument llvm.mem[set|cpy|move].* calls with load/store checks.
  Builder->SetInsertPoint(&MI);
  Value *AccessSize = Builder->CreateIntCast(MI.getLength(), SizeTy,
                                               /*isSigned=*/false);
  // memcpy and memmove have a source memory area but memset doesn't
  if (MemTransferInst *MTI = dyn_cast<MemTransferInst>(&MI))
    instrument(MTI->getSource(), AccessSize, LoadCheckFunction, MI);

  instrument(MI.getDest(), AccessSize, StoreCheckFunction, MI);
  ++IntrinsicsInstrumented;
}






bool ProtectionPass::isAllocated(Value *Address, uint64_t TypeSize) {

  memAccess* A = new memAccess();
  A->Address = Address;
  A->TypeSize = TypeSize;

  //  Already seen this access?

  bool found = (std::find(already_seen.begin(), already_seen.end(), *A) != already_seen.end());
  if (found){
    return 1;
  }

  uint64_t TypeSize_Byte = TypeSize / 8;

  SizeOffsetType SizeOff = ObjSizeVis->compute(Address);
  if (!ObjSizeVis->bothKnown(SizeOff)){
    return false;
  } 

  uint64_t Size = SizeOff.first.getZExtValue();
  int64_t Off = SizeOff.second.getSExtValue();

  bool res = Off >= 0 && Size >= Off && Size - Off >= TypeSize_Byte;

  // add to already seen list

  if (res){
    already_seen.push_back(*A);
  }

  return res;
}


void ProtectionPass::visitAllocaInst(AllocaInst &AI) { 
  if (!isa<ConstantInt>(AI.getArraySize())){
    dynAlloca = true;
  }
}


void ProtectionPass::visitCallInst (CallInst &  I ){
  
  // Calls with less than 4 arguments (which can be
  // passed via register) don't have to be instrumented
  if (I.getNumArgOperands() < 4){
    return;
  } 
  // collect the rest of them
  // we will see later if we have to instrument it
  // (in case of dynamic allocas)
  large_calls.push_back(&I);
}




void ProtectionPass::instrumentWithIRQCheck(Function &F){

  Constant *instrFuncIRQ = F.getParent()->getOrInsertFunction(
        "irq_arch_in", Int32Ty, NULL
  );

  IrqInFunction = F.getParent()->getFunction("irq_arch_in");
  assert(StackOverflowedFunction && "irq_arch_in function has disappeared!\n");

  LLVMContext &Context = F.getContext();

    bool con = true;

    for (auto &B : F) {
      if (!con){
      break;
      } 
      for (auto &I : B) { 
        if (auto *op = dyn_cast<AllocaInst>(&I)) {
          
        } else {
          
          IntegerType *Int32Ty = IntegerType::getInt32Ty(Context);

          Builder->SetInsertPoint(&I);
          errs() << "Problemchen \n";

          CallInst *CI = Builder->CreateCall(instrFuncIRQ, {});
          CI->setDebugLoc(I.getDebugLoc());
          Value* CMPI = Builder->CreateICmpEQ(CI, ConstantInt::get(Int32Ty, 0));

          BasicBlock *Head = I.getParent();
          BasicBlock *Tail = Head->splitBasicBlock(I.getIterator());

          TerminatorInst *HeadOldTerm = Head->getTerminator();
          LLVMContext &C = Head->getContext();

          BasicBlock *ThenBlock = BasicBlock::Create(C, "ThenBlock", Head->getParent(), Tail);
          TerminatorInst *CheckTerm;
        
          CheckTerm = BranchInst::Create(Tail, ThenBlock);
          CheckTerm->setDebugLoc(I.getDebugLoc());

          Builder->SetInsertPoint(CheckTerm);
          CallInst *CI2 = Builder->CreateCall(StackOverflowedFunction, {});   // ToDo own function
          //CI->setDebugLoc(I.getDebugLoc());


          BranchInst *HeadNewTerm =
          BranchInst::Create(ThenBlock, Tail, CMPI);
          CheckTerm->setDebugLoc(I.getDebugLoc());

          ReplaceInstWithInst(HeadOldTerm, HeadNewTerm);
          con = false;
          break;
        } 
      } 

      errs() << "HIERNICHT \n";
      return;
  }
}


void ProtectionPass::instrumentWithStackponterCheck(Function &F){

  LLVMContext &Context = F.getContext();

  bool con = true;

  for (auto &B : F) {
    if (!con){
      break;
    }
    for (auto &I : B) { 
      if (auto *op = dyn_cast<AllocaInst>(&I)) {
          
      } else {

        IntegerType *Int32Ty = IntegerType::getInt32Ty(Context);

        errs() << "######### Insert Stack check at ::   " << I << "\n";

        Builder->SetInsertPoint(&I);
        con = false;

        StringRef sp("sp\00");

        Type *ArgTy = Type::getInt32Ty(Context);

        llvm::Metadata *Ops[] = { llvm::MDString::get(Context, sp) };
        llvm::MDNode *RegName = llvm::MDNode::get(Context, Ops);
        llvm::Value *Metadata = llvm::MetadataAsValue::get(Context, RegName);

        llvm::Value *read_reg =   llvm::Intrinsic::getDeclaration(F.getParent(), llvm::Intrinsic::read_register, ArgTy);
        llvm::Value *SPCall = Builder->CreateCall(read_reg, Metadata);

        LoadInst* LI_lb =  Builder->CreateLoad (lower_stack_bound, true);
        LI_lb->setDebugLoc(I.getDebugLoc());

        LI_lb->setAlignment(2);

        Value* CMPI = Builder->CreateICmpULE  (SPCall, LI_lb);

        BasicBlock *Head = I.getParent();
        BasicBlock *Tail = Head->splitBasicBlock(I.getIterator());

        TerminatorInst *HeadOldTerm = Head->getTerminator();
        LLVMContext &C = Head->getContext();

        BasicBlock *ThenBlock = BasicBlock::Create(C, "ThenBlock", Head->getParent(), Tail);
        TerminatorInst *CheckTerm;
        
        CheckTerm = BranchInst::Create(Tail, ThenBlock);
        CheckTerm->setDebugLoc(I.getDebugLoc());

        Builder->SetInsertPoint(CheckTerm);
        CallInst *CI = Builder->CreateCall(StackOverflowedFunction, {});
        //CI->setDebugLoc(I.getDebugLoc());


        BranchInst *HeadNewTerm =
        BranchInst::Create(ThenBlock, Tail, CMPI);
        CheckTerm->setDebugLoc(I.getDebugLoc());

        //HeadNewTerm->setMetadata(LLVMContext::MD_prof, BranchWeights);
        ReplaceInstWithInst(HeadOldTerm, HeadNewTerm);

        break;
      } 
    } 
  } 
}


void ProtectionPass::instrumentCallSiteWithStackponterCheck(Function &F){

  LLVMContext &Context = F.getContext();

  for (auto I : large_calls) {
    
    IntegerType *Int32Ty = IntegerType::getInt32Ty(Context);

    errs() << "######### Insert Stack check at ::   " << *I << "\n";

    Builder->SetInsertPoint(I);

    StringRef sp("sp\00");
    
    Type *ArgTy = Type::getInt32Ty(Context);

    llvm::Metadata *Ops[] = { llvm::MDString::get(Context, sp) };
    llvm::MDNode *RegName = llvm::MDNode::get(Context, Ops);
    llvm::Value *Metadata = llvm::MetadataAsValue::get(Context, RegName);

    llvm::Value *read_reg =   llvm::Intrinsic::getDeclaration(F.getParent(), llvm::Intrinsic::read_register, ArgTy);
    llvm::Value *SPCall = Builder->CreateCall(read_reg, Metadata);

    LoadInst* LI_lb =  Builder->CreateLoad (lower_stack_bound, true);
    LI_lb->setDebugLoc(I->getDebugLoc());

    LI_lb->setAlignment(2);

    Value* CMPI = Builder->CreateICmpULE  (SPCall, LI_lb);

    BasicBlock *Head = I->getParent();
    BasicBlock *Tail = Head->splitBasicBlock(I->getIterator());

    TerminatorInst *HeadOldTerm = Head->getTerminator();
    LLVMContext &C = Head->getContext();

    BasicBlock *ThenBlock = BasicBlock::Create(C, "ThenBlock", Head->getParent(), Tail);
    TerminatorInst *CheckTerm;
        
    CheckTerm = BranchInst::Create(Tail, ThenBlock);
    CheckTerm->setDebugLoc(I->getDebugLoc());

    Builder->SetInsertPoint(CheckTerm);
    CallInst *CI = Builder->CreateCall(StackOverflowedFunction, {});
    //CI->setDebugLoc(I.getDebugLoc());

    BranchInst *HeadNewTerm =
    BranchInst::Create(ThenBlock, Tail, CMPI);
    CheckTerm->setDebugLoc(I->getDebugLoc());

    //HeadNewTerm->setMetadata(LLVMContext::MD_prof, BranchWeights);
    ReplaceInstWithInst(HeadOldTerm, HeadNewTerm);
    
    break;
  }  
}





// stopgap to save usart input

bool ProtectionPass::calledByUsart(Function &F){

  StringRef irq_handler_string_ref("irq_handler");
  StringRef tsrb_full_string_ref("tsrb_full");
  StringRef _push_string_ref("_push");
  StringRef mutex_unlock_string_ref("mutex_unlock");
  StringRef list_remove_head_string_ref("list_remove_head");
  StringRef sched_set_status_string_ref("sched_set_status");
  StringRef clist_insert_string_ref("clist_insert");
  StringRef irq_arch_restore_string_ref("irq_arch_restore");
  StringRef irq_arch_in_string_ref("irq_arch_in");
  StringRef memfault_string_ref("__stackoverflow");
  StringRef NVIC_SetPendingIRQ_string_ref("NVIC_SetPendingIRQ");
  StringRef gnrc_netapi_get_string_ref("gnrc_netapi_get");
  StringRef tsrb_add_one_string_ref("tsrb_add_one");
  StringRef uart_stdio_rx_cb_string_ref("uart_stdio_rx_cb");
  StringRef isr_usart1_strinf_ref("isr_usart1");
  StringRef dev_string_ref("dev");
  StringRef sched_switch_string_ref("sched_switch");
  StringRef thread_yield_string_ref("thread_yield");
  StringRef clist_advance_string_ref("clist_advance");
  StringRef add_timer_to_long_list_string_ref("_add_timer_to_long_list");

  if ( F.getName() == irq_handler_string_ref   
    || F.getName() == tsrb_full_string_ref
    || F.getName() == _push_string_ref
    || F.getName() == mutex_unlock_string_ref 
    || F.getName() == list_remove_head_string_ref
    || F.getName() == sched_set_status_string_ref
    || F.getName() == irq_arch_restore_string_ref 
    || F.getName() == irq_arch_in_string_ref
    || F.getName() == memfault_string_ref
    || F.getName() == NVIC_SetPendingIRQ_string_ref
    || F.getName() == gnrc_netapi_get_string_ref
    || F.getName() == tsrb_add_one_string_ref    
    || F.getName() == uart_stdio_rx_cb_string_ref
    || F.getName() == isr_usart1_strinf_ref
    || F.getName() == dev_string_ref 
    || F.getName() == sched_switch_string_ref
    || F.getName() == thread_yield_string_ref
    || F.getName() == clist_advance_string_ref
    || F.getName() == add_timer_to_long_list_string_ref
    || F.getName() == clist_insert_string_ref ) {
    return true;
  }

  return false;
}




char ProtectionPass::ID = 0;
static void registerProtectionPass(const PassManagerBuilder &,
                         legacy::PassManagerBase &PM) {
  PM.add(new ProtectionPass());
}
static RegisterStandardPasses
  RegisterMyPass(PassManagerBuilder::EP_OptimizerLast, registerProtectionPass);

static RegisterPass<ProtectionPass> X("protection", "Protection Pass for RIOT OS",
                             false, false);

// EP_EarlyAsPossible  EP_OptimizerLast



