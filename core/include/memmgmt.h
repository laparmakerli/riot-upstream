/**
 * @ingroup core
 * @{
 * @file        memmgmt.h
 * @author      Lars Parmakerli 
 */


#include "clist.h"
#include "cib.h"
#include "msg.h"
#include "arch/thread_arch.h"
#include "cpu_conf.h"
#include "sched.h"
#include "clist.h"
#include "thread_flags.h"

#ifndef _MEMORY_MGMT_H
#define _MEMORY_MGMT_H


//	bounds of memory sections based on the linker script

extern const uint32_t protected_threads_end;
extern const uint32_t protected_threads_start;
extern const uint32_t _eheap;
extern const uint32_t _sstack;

static uint32_t lower_threads_bound = (uint32_t) &protected_threads_start;
static uint32_t upper_threads_bound = (uint32_t) &protected_threads_end;

extern const uint32_t kernel_global_start;
extern const uint32_t kernel_global_end;

static uint32_t kernel_data_start = (uint32_t) &kernel_global_start;
static uint32_t kernel_data_end = (uint32_t) &kernel_global_end;

static const uint32_t kernel_st_hp_start = (uint32_t) &_sstack;
static const uint32_t kernel_st_hp_end = (uint32_t) &_eheap;




/**
 * @brief initialize thread blocks by creating an initial block
 *
 * This will create an initial block with the full size
 * of the .thread_blocks -section.
 * The block will be available for allocation.
 *
 */
void init_thread_blocks(void);

/**
 * @brief allocate a thread block
 *
 * This will try to allocate a block of 
 * a certain size and return
 *
 * @param[in] desired size of the allocated block
 *
 * @return stack pointer
 */
char* alloc_thread_block(unsigned int  size);


/**
 * @brief free allocated block
 *
 * This will free the block with 
 * start address ptr
 *
 * @param[in] startpointer of the block
 *
 */
void free_thread_block(void* ptr);




#endif 
