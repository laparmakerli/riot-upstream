#include <thread.h>
#include <stdio.h>
#include <sched.h>
#include <irq.h>
#include "memmgmt.h"
#include <inttypes.h>
#include <protection.h>
#include <sched.h>


int in_irq = 0;
int in_stack = 0;
int in_stacks[8] = {0,0,0,0,0,0,0,0};


int outer_stacks = 0;

uintptr_t in_stacks_arr[8][32];


void __loadcheck(void* pointer, __int64_t access_size) {
    if (irq_is_in()) {
        in_irq +=1;
        return;
    }
    
    uintptr_t ptr = (uintptr_t) pointer;
    
    if (ptr < upper_stack_bound && ptr > lower_stack_bound){
        in_stack += 1;
        return;
    }
    if (ptr < upper_stacks_bound && ptr > lower_stacks_bound){
        in_stacks[sched_active_pid] += 1;
        int i = in_stacks[sched_active_pid] % 32;
        in_stacks_arr[sched_active_pid][i] = (uintptr_t) pointer;
        asm("nop");
        return;
    }
    outer_stacks += 1;
    return;
}


void __storecheck(void* pointer, __int64_t access_size) {
    if (irq_is_in()) {
        in_irq +=1;
        return;
    }
    
    uintptr_t ptr = (uintptr_t) pointer;
    
    if (ptr < upper_stack_bound && ptr > lower_stack_bound){
        in_stack += 1;
        return;
    }
    if (ptr < upper_stacks_bound && ptr > lower_stacks_bound){
        in_stacks[sched_active_pid] += 1;
        int i = in_stacks[sched_active_pid] % 32;
        in_stacks_arr[sched_active_pid][i] = (uintptr_t) pointer;
        asm("nop");
        return;
    }
    outer_stacks += 1;
    return;
}


int getInIRQ(void){
    int tmp = in_irq;
    in_irq = 0;
    return tmp;
}

int getInStack(void){
    int tmp = in_stack;
    in_stack = 0;
    return tmp;
}

/*
int getInStacks(){
    int tmp = in_stacks;
    in_stacks = 0;
    return tmp;
}
*/

int getOuterStacks(void){
    int tmp = outer_stacks;
    outer_stacks = 0;
    return tmp;
}

void __memfault(void){
    asm("nop");
}








