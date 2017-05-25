#include "shared_memory.h"


unsigned int SHARED_METADATA_SIZE;


/**
 * Determines the start of heap.
 */ 
void* shared_heap_start;

char shared[1024];



/**
 * Adjusts the requested size so that the allocated space is always a multiple of alighment factor
 */ 
unsigned int shared_align_size(unsigned int size){
	return (size % ALIGN_OF(void *)) ? size+ALIGN_OF(void *)-(size%ALIGN_OF(void *)) : size;
}



/**
 * Goes through the whole heap to find an empty slot.
 */ 
shared_slot_data* shared_find_slot(unsigned int size){
	shared_slot_data* iter = (shared_slot_data*)shared_heap_start;
	while(iter){
		if (iter->available && iter->size>=size){
			iter->available=false;
			return iter;
		}
		iter=iter->next_block;
	}
	return NULL;
}

/**
 * If a free slot can accommodate atleast 1 more (SHARED_METADATA_SIZE+ALIGNMENT FACTOR)
 * apart from the requested size, then the slot is divided to save space.
 */ 
void shared_divide_slot(void* slot,unsigned int  size){
	shared_slot_data* slot_to_divide = (shared_slot_data*)slot;
	shared_slot_data* new_slot= (shared_slot_data*)((char*)slot_to_divide+SHARED_METADATA_SIZE+size);
	
	new_slot->size=slot_to_divide->size - size - SHARED_METADATA_SIZE;
	new_slot->available=true;
	new_slot->next_block=slot_to_divide->next_block;
	new_slot->start= ((char *) new_slot) + SHARED_METADATA_SIZE;
	new_slot->end = new_slot->start + new_slot->size - 4;
	new_slot->magic_number=SHARED_MAGIC_NUMBER;
	
	slot_to_divide->size=size;
	slot_to_divide->end = slot_to_divide->start + size;
	slot_to_divide->next_block=new_slot;
}

void init_shared_blocks(void){

	SHARED_METADATA_SIZE = sizeof(shared_slot_data);
	uintptr_t misalignment = (uintptr_t) SHARED_METADATA_SIZE % ALIGN_OF(void *);
    if (misalignment) {
    	misalignment = ALIGN_OF(void *) - misalignment;
        SHARED_METADATA_SIZE += misalignment;
    }

	shared_heap_start = (void *)shared;
	int total_size = 1024;

	misalignment = (uintptr_t) shared_heap_start % ALIGN_OF(void *);
    if (misalignment) {
        misalignment = ALIGN_OF(void *) - misalignment;
        shared_heap_start += misalignment;
        total_size -= misalignment;
    }

	shared_slot_data * new_block = (shared_slot_data*) shared_heap_start;

	new_block->size=total_size;
	new_block->available=true;
	new_block->next_block=NULL;
	new_block->start=shared_heap_start + SHARED_METADATA_SIZE;
	new_block->end  =shared_heap_start + total_size - 4; 
	new_block->magic_number = SHARED_MAGIC_NUMBER;

}

/**
 * Returns the metadata from heap corresponding to a data pointer.
 */ 
shared_slot_data* shared_get_metadata(void* ptr){
	
	return (shared_slot_data*)((char*)ptr - SHARED_METADATA_SIZE);
}

/**
* Search for big enough free space on heap.
* Split the free space slot if it is too big, else space will be wasted.
* Return the pointer to this slot.
* If no adequately large free slot is available, extend the heap and return the pointer.
*/
void* alloc_shared(unsigned int  size){
	size = shared_align_size(size);

	void* slot;

	slot = shared_find_slot(size);

	if (slot){
		if(((shared_slot_data*)slot)->size > size + SHARED_METADATA_SIZE){
			shared_divide_slot(slot,size);
		}
	}

	if (!slot){
		return slot;
	}
	
	return ((char*)slot)+SHARED_METADATA_SIZE;
}

/**
 * Frees the allocated memory. If first checks if the pointer falls
 * between the allocated heap range. It also checks if the pointer
 * to be deleted is actually allocated. this is done by using the
 * magic number. Due to lack of time i haven't worked on fragmentation.
 */ 
void free_shared_block(void* ptr){

	if (!shared_heap_start) return;
	if (ptr >= shared_heap_start && ptr < shared_heap_start+1024){
		shared_slot_data* ptr_metadata = shared_get_metadata(ptr);
		if (ptr_metadata->magic_number==SHARED_MAGIC_NUMBER){
			ptr_metadata->available=true;
		}
	}
}


