#include <thread.h>
#include <stdio.h>
#include <sched.h>
#include <irq.h>
#include "memmgmt.h"
#include <inttypes.h>
#include <protection.h>
#include <sched.h>


/*DEBUG*/
int forbidden[8] = {0,0,0,0,0,0,0,0};

void __stackoverflow(void);
void __loadfault(void);
void __storefault(void);
void __irqfault(void);

void __loadcheck(void* pointer, __int64_t access_size) {

    /* PRIVILIGED ACCESS - allways allowed */

    if (irq_is_in()) {
        return;
    }

    /* UNPRIVILEGED ACCESS */
    
    uintptr_t ptr = (uintptr_t) pointer;

    // Load access in R3 - allowed

    if (ptr < 0x20000000){
        return;
    }

    // Load access in R1 - allowed
    
    if (ptr < upper_stack_bound && ptr > (lower_stack_bound-32)){
        return;
    }

    // Load access in R2 - not allowed 

    if (ptr < upper_stacks_bound){
        forbidden[sched_active_pid] += 1;
        __loadfault();
        return;
    }

    // Load access in R5 - not allowed

    if (ptr < kernel_st_hp_end && ptr > kernel_st_hp_start){
        forbidden[sched_active_pid] += 1;
        //__loadfault();
        return;
    }

    // Everything else (R4 or Periphals) - allowed

    return;
}


void __storecheck(void* pointer, __int64_t access_size) {

    /* PRIVILIGED ACCESS - allways allowed */

    if (irq_is_in()) {
        return;
    }


    /* UNPRIVILEGED ACCESS */

    uintptr_t ptr = (uintptr_t) pointer;


    // Load access in R3 - allowed

    if (ptr < 0x20000000){
        __storefault();
    }


    // Store access in R1 - allowed
    
    if (ptr < upper_stack_bound && ptr > (lower_stack_bound-32)){
        return;
    }

    // Store access in either R3 or R4 - not allowed

    if (ptr < kernel_data_end){
        forbidden[sched_active_pid] += 1;
        __storefault();
        return;
    }

    // Store access in R5 - not allowed

    if (ptr < kernel_st_hp_end && ptr > kernel_st_hp_start){
        forbidden[sched_active_pid] += 1;
        //__storefault();
        return;
    }

    // Everything else (R4 or Periphals) - allowed

    return; 
}

void __stackoverflow(void){

    /* STACKOVERFLOW DETECTED - SVCall for own exit */

    __asm__ volatile (
    "svc    #0xB    \n"
    );
}

void __loadfault(void){

    /* LOADFAULT DETECTED - SVCall for own exit */

    __asm__ volatile (
    "svc    #0xC    \n"
    );
}

void __storefault(void){

    /* STOREFAULT DETECTED - SVCall for own exit */

    __asm__ volatile (
    "svc    #0xD    \n"
    );
}

void __irqfault(void){

    /* IRQ ACCESS DETECTED - SVCall for own exit */

    __asm__ volatile (
    "svc    #0xE    \n"
    );
}




