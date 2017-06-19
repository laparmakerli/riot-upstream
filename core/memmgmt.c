/**
 * @ingroup core
 * @{
 * @file        memmgmt.c
 * @author      Lars Parmakerli 
 */


#include "memmgmt.h"



//	block struct - allocated at the beginning of each block 
//	holds context information about the block

typedef struct block{
	bool free;
	unsigned int size;
	char *start;
	char *end;
	struct block* next_block;
} block_t;


//	find the first free block with the required size

block_t* find_block(unsigned int size){
	block_t* current = (block_t*)lower_threads_bound;

	while(current){
		if (current->free && current->size>=size){
			current->free=false;
			return current;
		}
		current=current->next_block;
	}
	return NULL;
}


//	create new block with the remaining space 

void divide_block(void* block,unsigned int  size){
	block_t* old_block = (block_t*)block;
	block_t* new_block = (block_t*)((char*)old_block + sizeof(block_t) + size);
	
	new_block->free=true;
	new_block->size=old_block->size - sizeof(block_t) - size ;
	new_block->start= ((char *) new_block) + sizeof(block_t);
	new_block->end = new_block->start + new_block->size - 4;	
	new_block->next_block=old_block->next_block;
	old_block->size=size;
	old_block->end = old_block->start + size;
	old_block->next_block=new_block;
}


//	initialize thread blocks by creating an initial block
//	with the complete .thread_block -section size

void init_thread_blocks(void){

	block_t * new_block = (block_t*) lower_threads_bound;
	new_block->size=(char*) upper_threads_bound - (char*) lower_threads_bound;
	new_block->free=true;
	new_block->next_block=NULL;
	new_block->start=(char*) lower_threads_bound + sizeof(block_t);
	new_block->end  =(char*) upper_threads_bound; 
}


//	get block data from start pointer ptr

block_t* get_block(void* ptr){
	
	return (block_t*)((char*)ptr - sizeof(block_t));
}


//	allocate a new block

char* alloc_thread_block(unsigned int  size){

	void* block;
	block = find_block(size);
	if (block){
		if(((block_t*)block)->size > size + sizeof(block_t)){
			divide_block(block,size);
		}
	}
	if (!block){
		return NULL;
	}	
	return ((char*)block)+sizeof(block_t);
}


//	free block with startpointer ptr

void free_thread_block(void* ptr){

	if (ptr >= (void *) lower_threads_bound && ptr < (void *) upper_threads_bound){
		block_t* ptr_metadata = get_block(ptr);
		if (ptr_metadata->free==false){
			ptr_metadata->free=true;
		}
	}
}


