/**
 * @ingroup core
 * @{
 * @file        shared_memory.c
 * @author      Lars Parmakerli 
 */

#include "shared_memory.h"


char shared[SHARED_MEMORY_SIZE];


//	shared block struct - allocated at the beginning of each shared block 
//	holds context information about the shared block

typedef struct shared_block{
	bool free;
	unsigned int size;
	char *start;
	char *end;
	struct shared_block* next_shared_block;
} shared_block_t;


//	find the first free shared block with the required size

shared_block_t* find_shared_block(unsigned int size){
	shared_block_t* current = (shared_block_t*) &shared[0];

	while(current){
		if (current->free && current->size>=size){
			current->free=false;
			return current;
		}
		current=current->next_shared_block;
	}
	return NULL;
}


//	create new shared block with the remaining space 

void divide_shared_block(void* block,unsigned int  size){
	shared_block_t* old_block = (shared_block_t*)block;
	shared_block_t* new_shared_block = (shared_block_t*)((char*)old_block + sizeof(shared_block_t) + size);
	
	new_shared_block->free=true;
	new_shared_block->size=old_block->size - sizeof(shared_block_t) - size ;
	new_shared_block->start= ((char *) new_shared_block) + sizeof(shared_block_t);
	new_shared_block->end = new_shared_block->start + new_shared_block->size - 4;	
	new_shared_block->next_shared_block=old_block->next_shared_block;
	old_block->size=size;
	old_block->end = old_block->start + size;
	old_block->next_shared_block=new_shared_block;
}


//	initialize thread shared blocks by creating an initial shared block
//	with the complete shared space

void init_shared_blocks(void){

	shared_block_t *new_shared_block = (shared_block_t*) &shared[0];
	new_shared_block->size= SHARED_MEMORY_SIZE;
	new_shared_block->free=true;
	new_shared_block->next_shared_block=NULL;
	new_shared_block->start= (char*) &shared[0];
	new_shared_block->end  = (char*) &shared[SHARED_MEMORY_SIZE]; 
}


//	get shared block data from start pointer ptr

shared_block_t* get_shared_block(void* ptr){
	
	return (shared_block_t*)((char*)ptr - sizeof(shared_block_t));
}


//	allocate a new shared block

void* alloc_shared_block(unsigned int  size){

	void* shared_block;
	shared_block = find_shared_block(size);
	if (shared_block){
		if(((shared_block_t*)shared_block)->size > size + sizeof(shared_block_t)){
			divide_shared_block(shared_block, size);
		}
	}
	if (!shared_block){
		return NULL;
	}	
	return ((char*)shared_block)+sizeof(shared_block_t);
}


//	free shared block with startpointer ptr

void free_shared_block(void* ptr){

	if (ptr >= (void *) &shared[0] && ptr < (void *) &shared[SHARED_MEMORY_SIZE]){
		shared_block_t* ptr_metadata = get_shared_block(ptr);
		if (ptr_metadata->free==false){
			ptr_metadata->free=true;
		}
	}
}
