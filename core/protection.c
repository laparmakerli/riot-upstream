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
int outer_stacks = 0;
int forbidden[8] = {0,0,0,0,0,0,0,0};

uintptr_t in_stacks_arr[8][32];
void __stackoverflow(void);

void __loadcheck(void* pointer, __int64_t access_size) {

    /* PRIVILIGED ACCESS - allways allowed */

    if (irq_is_in()) {
        in_irq +=1;
        return;
    }

    /* UNPRIVILEGED ACCESS */
    
    uintptr_t ptr = (uintptr_t) pointer;

    // Load access in R3 - allowed

    if (ptr < 0x20000000){
        in_code += 1;
        return;
    }

    // Load access in R1 - allowed
    
    if (ptr < upper_stack_bound && ptr > (lower_stack_bound-32)){
        in_stack += 1;
        return;
    }

    // Load access in R2 - not allowed 

    if (ptr < upper_stacks_bound){
        forbidden[sched_active_pid] += 1;
        int i = forbidden[sched_active_pid] % 32;
        in_stacks_arr[sched_active_pid][i] = (uintptr_t) pointer;
        return;
    }

    // Load access in R5 - not allowed

    if (ptr < kernel_st_hp_end && ptr > kernel_st_hp_start){
        forbidden[sched_active_pid] += 1;
        int i = forbidden[sched_active_pid] % 32;
        in_stacks_arr[sched_active_pid][i] = (uintptr_t) pointer;
        return;
    }

    // Everything else (R4 or Periphals) - allowed

    outer_stacks += 1; 

    return;
}


void __storecheck(void* pointer, __int64_t access_size) {

    /* PRIVILIGED ACCESS - allways allowed */

    if (irq_is_in()) {
        in_irq +=1;
        return;
    }


    /* UNPRIVILEGED ACCESS */

    uintptr_t ptr = (uintptr_t) pointer;


    // Load access in R3 - allowed

    if (ptr < 0x20000000){
        __stackoverflow();
    }


    // Store access in R1 - allowed
    
    if (ptr < upper_stack_bound && ptr > (lower_stack_bound-32)){
        in_stack += 1;
        return;
    }

    // Store access in either R3 or R4 - not allowed

    if (ptr < kernel_data_end){
        forbidden[sched_active_pid] += 1;
        int i = forbidden[sched_active_pid] % 32;
        in_stacks_arr[sched_active_pid][i] = (uintptr_t) pointer;
        return;
    }

    // Store access in R5 - not allowed

    if (ptr < kernel_st_hp_end && ptr > kernel_st_hp_start){
        forbidden[sched_active_pid] += 1;
        int i = forbidden[sched_active_pid] % 32;
        in_stacks_arr[sched_active_pid][i] = (uintptr_t) pointer;
        return;
    }

    // Everything else (R4 or Periphals) - allowed

    outer_stacks += 1;  
    return; 
}


void __stackoverflow(void){

    /* STACKOVERFLOW DETECTED - SVCall for own exit */

    __asm__ volatile (
    "svc    #0xB    \n"
    );

}



