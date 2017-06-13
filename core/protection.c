#include <thread.h>
#include <stdio.h>
#include <sched.h>
#include <irq.h>
#include "memmgmt.h"
#include <inttypes.h>
#include <protection.h>
#include <sched.h>


/*DEBUG*/
int in_irq = 0;
int in_stack = 0;
int in_code = 0;
int forbidden[8] = {0,0,0,0,0,0,0,0};
int rubbish[56];


int outer_stacks = 0;

uintptr_t in_stacks_arr[8][32];


void __loadcheck(void* pointer, __int64_t access_size) {
    asm("nop");
    if (irq_is_in()) {
        in_irq +=1;
        return;
    }
    
    uintptr_t ptr = (uintptr_t) pointer;

    if (ptr < 0x20000000){
        in_code += 1;
        return;
    }
    
    if (ptr < upper_stack_bound && ptr > lower_stack_bound){
        in_stack += 1;
        return;
    }

    if (ptr < upper_stacks_bound){
        forbidden[sched_active_pid] += 1;
        int i = forbidden[sched_active_pid] % 32;
        in_stacks_arr[sched_active_pid][i] = (uintptr_t) pointer;
        return;
    }

    outer_stacks += 1;  
    return;
}


void __storecheck(void* pointer, __int64_t access_size) {
    asm("nop");
    if (irq_is_in()) {
        in_irq +=1;
        return;
    }
    
    uintptr_t ptr = (uintptr_t) pointer;
    
    if (ptr < upper_stack_bound && ptr > lower_stack_bound){
        in_stack += 1;
        return;
    }

    if (ptr < kernel_data_end){
        forbidden[sched_active_pid] += 1;
        int i = forbidden[sched_active_pid] % 32;
        in_stacks_arr[sched_active_pid][i] = (uintptr_t) pointer;
        return;
    }

    outer_stacks += 1;  
    return; 
}


void __memfault(void){

    __asm__ volatile (
    //   "add    sp, #100         \n"       
    "svc        #0xB              \n"
    );

}

void __attribute__ ((noinline)) forbidden_at_pid(kernel_pid_t pid){
    printf("forbidden[%i] = %i\n", pid, forbidden[pid]);
}