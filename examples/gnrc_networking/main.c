
/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Example application for demonstrating the RIOT network stack
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>

#include "shell.h"
#include "msg.h"
#include <malloc.h>
#include "thread.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <sys/types.h>
#include <xtimer.h>

#include <unistd.h>




#define MAIN_QUEUE_SIZE     (8)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

extern int udp_cmd(int argc, char **argv);
int udp_cmd_benchmarked(int argc, char **argv);

char stack_arr[256];


// need to set arb pointer manually. 
// passing as function argument is broken
volatile uintptr_t arb_pointer = 0x20000001; 


// this function provokes a stack overflow by
// calling itself recursively

int __attribute__ ((noinline))  stack_overflow(int j){
    if (j>100){
        return j;
    }
    return stack_overflow(j);
}

// this function loads from an arbitrary address
// casted from an an integer 

int __attribute__ ((noinline)) load_from(){
    int *ptr = (int*) arb_pointer; 
    int res = *ptr;
    thread_sleep();
    return res;
}


// this function stores 10 to an arbitrary address
// casted from an an integer 

int __attribute__ ((noinline))  store_into(){
    int *ptr = (int*) arb_pointer; 
    *ptr = 10;
    thread_sleep();
    return 0;
}


// this function handles the new
// created memfault thread

void *thread_handler(void *arg){
    if (strcmp("stackoverflow", arg)==0){
        stack_overflow(2);
    } else
    if (strcmp("load", arg)==0){
        load_from();
    } else
    if (strcmp("store", arg)==0){
        store_into();
    }

    return NULL;
}


int crash_cmd(int argc, char **argv)
{
    if (strcmp("load", argv[0])==0 || strcmp("store", argv[0])==0){
        //arb_pointer = atoi(argv[1]);
        asm("nop");
    }

    thread_create(stack_arr, 256,
                    THREAD_PRIORITY_MAIN - 1,
                    THREAD_CREATE_STACKTEST,
                    thread_handler,
                    argv[0], "crash_thread");
    return 0;
}




static const shell_command_t shell_commands[] = {
    { "udp", "send data over UDP and listen on UDP ports", udp_cmd },
    { "stackoverflow", "provoke stack overflwo", crash_cmd},
    { "load", "load from arbitrary address", crash_cmd},
    { "store", "store to arbitrary address", crash_cmd},
    { NULL, NULL, NULL }
};

int main(void)
{
    /* we need a message queue for the thread running the shell in order to
     * receive potentially fast incoming networking packets */
    svc_msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);
    puts("RIOT network stack example application");

    /* start shell */
    puts("All up, running the shell now");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    /* should be never reached */
    return 0;
}


int udp_cmd_benchmarked(int argc, char **argv){
    unsigned long start = xtimer_now();

    int res = udp_cmd(argc, argv);

    unsigned long end = xtimer_now();

    printf("udp time : %lu\n", end-start);

    return res;
}

