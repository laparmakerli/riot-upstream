#include "memmgmt.h"


unsigned int METADATA_SIZE;


/**
 * Determines the start of heap.
 */ 
void* heap_start;



/**
 * Adjusts the requested size so that the allocated space is always a multiple of alighment factor
 */ 
unsigned int align_size(unsigned int size){
	return (size % ALIGN_OF(void *)) ? size+ALIGN_OF(void *)-(size%ALIGN_OF(void *)) : size;
}



/**
 * Goes through the whole heap to find an empty slot.
 */ 
slot_data* find_slot(unsigned int size){
	slot_data* iter = (slot_data*)heap_start;
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
 * If a free slot can accommodate atleast 1 more (METADATA_SIZE+ALIGNMENT FACTOR)
 * apart from the requested size, then the slot is divided to save space.
 */ 
void divide_slot(void* slot,unsigned int  size){
	slot_data* slot_to_divide = (slot_data*)slot;
	slot_data* new_slot= (slot_data*)((char*)slot_to_divide+METADATA_SIZE+size);
	
	new_slot->size=slot_to_divide->size - size - METADATA_SIZE;
	new_slot->available=true;
	new_slot->next_block=slot_to_divide->next_block;
	new_slot->start= ((char *) new_slot) + METADATA_SIZE;
	new_slot->end = new_slot->start + new_slot->size - 4;
	new_slot->magic_number=MAGIC_NUMBER;
	
	slot_to_divide->size=size;
	slot_to_divide->end = slot_to_divide->start + size;
	slot_to_divide->next_block=new_slot;
}

void init_blocks(void){

	METADATA_SIZE = sizeof(slot_data);
	uintptr_t misalignment = (uintptr_t) METADATA_SIZE % ALIGN_OF(void *);
    if (misalignment) {
    	misalignment = ALIGN_OF(void *) - misalignment;
        METADATA_SIZE += misalignment;
    }

	heap_start = (void *)lower_stacks_bound;
	int total_size = 16384;

	misalignment = (uintptr_t) heap_start % ALIGN_OF(void *);
    if (misalignment) {
        misalignment = ALIGN_OF(void *) - misalignment;
        heap_start += misalignment;
        total_size -= misalignment;
    }

	slot_data * new_block = (slot_data*) heap_start;

	new_block->size=total_size;
	new_block->available=true;
	new_block->next_block=NULL;
	new_block->start=heap_start + METADATA_SIZE;
	new_block->end  =heap_start + total_size - 4; 
	new_block->magic_number = MAGIC_NUMBER;

}

/**
 * Returns the metadata from heap corresponding to a data pointer.
 */ 
slot_data* get_metadata(void* ptr){
	
	return (slot_data*)((char*)ptr - METADATA_SIZE);
}

/**
* Search for big enough free space on heap.
* Split the free space slot if it is too big, else space will be wasted.
* Return the pointer to this slot.
* If no adequately large free slot is available, extend the heap and return the pointer.
*/
void* alloc_block(unsigned int  size){
	size = align_size(size);

	void* slot;

	slot = find_slot(size);

	if (slot){
		if(((slot_data*)slot)->size > size + METADATA_SIZE){
			divide_slot(slot,size);
		}
	}

	if (!slot){
		return slot;
	}
	
	return ((char*)slot)+METADATA_SIZE;
}

/**
 * Frees the allocated memory. If first checks if the pointer falls
 * between the allocated heap range. It also checks if the pointer
 * to be deleted is actually allocated. this is done by using the
 * magic number. Due to lack of time i haven't worked on fragmentation.
 */ 
void free_block(void* ptr){

	if (!heap_start) return;
	if (ptr >= heap_start && ptr < heap_start+16384){
		slot_data* ptr_metadata = get_metadata(ptr);
		if (ptr_metadata->magic_number==MAGIC_NUMBER){
			ptr_metadata->available=true;
		}
	}
}


