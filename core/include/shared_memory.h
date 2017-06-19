/**
 * @ingroup kernel
 * @{
 * @file        shared_memory.h
 * @author      Lars Parmakerli <dev@famulla.eu>
 */


#include "clist.h"
#include "cib.h"
#include "msg.h"
#include "arch/thread_arch.h"
#include "cpu_conf.h"
#include "sched.h"
#include "clist.h"
#include "thread_flags.h"

#ifndef _SHARED_MEMORY_H
#define _SHARED_MEMORY_H


#define SHARED_MEMORY_SIZE   (16000)




/**
 * @brief initialize thread blocks by creating an initial block
 *
 * This will create an initial shared block with the full size
 * of the shared memory area.
 * The block will be available for allocation.
 *
 */
void init_shared_blocks(void);


/**
 * @brief allocate a shared block
 *
 * This will try to allocate a shared 
 * block of a certain size and return
 *
 * @param[in] desired size of the allocated block
 *
 * @return stack pointer
 */
void* alloc_shared_block(unsigned int  size);


/**
 * @brief free allocated shared block
 *
 * This will free the shared block with 
 * start address ptr
 *
 * @param[in] startpointer of the shared block
 *
 */
void free_shared_block(void* ptr);







#endif // _MEMORY_MGMT_H
