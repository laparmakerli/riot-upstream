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

#ifndef _SHARED_MEMORY_H
#define _SHARED_MEMORY_H





/**
 * This structure contains the metadata.
 * Size determines the size of data excuding the size of metadata
 * next block is the pointer to next slot of memory in heap.
 */
typedef struct shared_meta_data{
	unsigned int size;
	unsigned int available;
	char * start;
	char * end;
	struct shared_meta_data* next_block;
	unsigned int magic_number;
} shared_slot_data;



#define SHARED_MEMSIZE 1024
#define SHARED_ALIGNMENT_FACTOR 4
#define SHARED_MAGIC_NUMBER 0123


void init_shared_blocks(void);


void* alloc_shared(unsigned int  size);


void free_shared_block(void* ptr);







#endif // _MEMORY_MGMT_H
