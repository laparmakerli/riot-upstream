/**
 * @ingroup kernel
 * @{
 * @file        memmgmt.h
 * @author      Tobias Famulla <dev@famulla.eu>
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





/**
 * This structure contains the metadata.
 * Size determines the size of data excuding the size of metadata
 * next block is the pointer to next slot of memory in heap.
 */
typedef struct meta_data{
	unsigned int size;
	unsigned int available;
	char * start;
	char * end;
	struct meta_data* next_block;
	unsigned int magic_number;
} slot_data;



#define MEMSIZE 32768
#define ALIGNMENT_FACTOR 4
#define MAGIC_NUMBER 0123



extern const uint32_t user_stack_end;
extern const uint32_t user_stack_start;

static uint32_t lower_stacks_bound = (uint32_t) &user_stack_start;
static uint32_t upper_stacks_bound = (uint32_t) &user_stack_end;

extern const uint32_t kernel_global_start;
extern const uint32_t kernel_global_end;

static uint32_t kernel_data_start = (uint32_t) &kernel_global_start;
static uint32_t kernel_data_end = (uint32_t) &kernel_global_end;




void init_thread_blocks(void);


void* alloc_thread_block(unsigned int  size);


void free_thread_block(void* ptr);







#endif // _MEMORY_MGMT_H
