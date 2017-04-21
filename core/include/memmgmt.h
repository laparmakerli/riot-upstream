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


extern const uintptr_t lower_stacks_bound;
extern const uintptr_t upper_stacks_bound;

void init_blocks(void);


void* alloc_block(unsigned int  size);


void free_block(void* ptr);







#endif // _MEMORY_MGMT_H
