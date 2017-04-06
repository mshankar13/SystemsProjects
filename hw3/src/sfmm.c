#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "sfmm.h"

/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */

void remove_block (void* ptr);
void add_freeblock(void* ptr);
void *coalesce(void* ptr);
void *address_check(void* ptr);
void split_block (void* ptr, size_t size, size_t old_size, size_t padding);

char* heap_start = NULL;
char* end_heap = NULL;
sf_free_header* freelist_head = NULL;
int heap_start_marker = 0;

static size_t internal = 0;
static size_t external = 0;
static size_t allocations = 0;
static size_t frees = 0;
static size_t coalesces = 0;

size_t calculate_block_size (size_t size) {
	size_t new_block_size = (2 * SF_HEADER_SIZE) + size;
	size_t remainder = new_block_size % 16;
	size_t padding = 0;
	if (remainder != 0) {
		padding = 16 - remainder;
	}	
	new_block_size += padding;

	return new_block_size;
}

void split_block (void* ptr, size_t size, size_t old_size, size_t padding) {
	/*Get pointer the head. Split using new size which is smaller than the blocksize given*/
	char* address = (char*) ptr;
	sf_free_header* old_header = (sf_free_header*) address;
	sf_footer* old_footer = (sf_footer*) (address + old_size - SF_HEADER_SIZE);
	sf_free_header* new_header = NULL;
	sf_footer* new_footer = NULL;
	size_t diff = (old_header->header.block_size << 4) - size;

	if (diff < 32) {
		/*Can't split block. Return pointer*/
	} else {
		/*Can split block. Update new header and footer*/
		new_header = (sf_free_header*) (address + size);
		new_header->header.block_size = diff >> 4;
		new_header->header.padding_size = 0;
		new_header->header.alloc = 0;
		new_footer = (sf_footer*) (address + size + diff - SF_HEADER_SIZE);
		new_footer->block_size = diff >> 4;
		new_footer->alloc = 0;

		old_header->header.block_size = size >> 4;
		old_header->header.padding_size = padding;
		old_header->header.alloc = 1;
		old_footer = (sf_footer*) (address + size - SF_HEADER_SIZE);
		old_footer->block_size = size >> 4;
		old_footer->alloc = 1;

		sf_free((char*)new_header + SF_HEADER_SIZE); /*Free the block*/
	}

}

void remove_block (void* ptr) {
	sf_free_header* current_block = (sf_free_header*) ptr;
	if (current_block->prev == NULL && current_block->next == NULL) {
		/*Only one element in the list*/
		freelist_head = NULL;
	} else if (current_block == freelist_head) {
		/*Head of the list*/
		current_block->next->prev = NULL;
		freelist_head = current_block->next;
	} else if (current_block->next == NULL) {
		/*Tail of the list*/
		current_block->prev->next = NULL;
	} else {
		current_block->prev->next = current_block->next;
		current_block->next->prev = current_block->prev;
	}
	current_block->prev = NULL;
	current_block->next = NULL;
	//current_block->header.alloc = 1;
}

void add_freeblock(void* ptr) {
	sf_free_header* free_block = (sf_free_header*) ptr;
	if (freelist_head == NULL) {
		freelist_head = free_block;
		free_block->next = NULL;
		free_block->prev = NULL;
	} else {
		freelist_head->prev = free_block;
		free_block->next = freelist_head;
		free_block->prev = NULL;
		freelist_head = free_block;
	}
}

void *coalesce(void* ptr) {

	char* address = (char*) ptr;
	int before = 0;
	int after = 0;
	size_t block_size = 0;
	size_t next_size = 0;
	size_t prev_size = 0; 
	size_t pad = 0;
	size_t pad_prev = 0;
	size_t pad_next = 0; 

	sf_free_header* free_header = (sf_free_header*) (address);
	block_size = free_header->header.block_size << 4;
	pad = free_header->header.padding_size;
	free_header->header.padding_size = 0;
	free_header->header.alloc = 0;
	sf_footer* free_footer = (sf_footer*) ((char*)free_header + block_size - SF_HEADER_SIZE);
	free_footer->block_size = block_size >> 4;
	free_footer->alloc = 0;

	sf_footer* prev_footer = (sf_footer*) ((char*)free_header - SF_HEADER_SIZE);
	sf_free_header* prev_header = NULL;

	if(((char*)prev_footer) <= heap_start) {
		/*Before the heap! Don't coalesce back*/
		before = 2;
	} else {
		before = prev_footer->alloc;
		prev_size = prev_footer->block_size << 4;
		prev_header = (sf_free_header*) ((char*)free_header - prev_size);
		pad_prev = prev_header->header.padding_size;
	}

	sf_free_header* next_header = (sf_free_header*) ((char*)free_header + block_size);
	sf_footer* next_footer = NULL;
	if (((char*)next_header) >= end_heap) {
		after = 2;
	} else {
		after = next_header->header.alloc;
		next_size = next_header->header.block_size << 4;
		pad_next = next_header->header.padding_size;
		next_footer = (sf_footer*) ((char*) next_header + next_size - SF_HEADER_SIZE);
	}

	if (before != 0 && after != 0) {
		internal = internal - 16 - pad;
		coalesces++;
	} else if (before == 0 && after != 0) {
		/*Free block before*/
		remove_block(prev_header);

		internal = internal - 16 - pad - pad_prev;
		coalesces++;
		block_size += prev_size;
		free_header = prev_header;

	} else if (before != 0 && after == 0) {
		/*Free block after. Update header and footer*/
		remove_block(next_header);

		internal = internal - 16 - pad - pad_next;
		coalesces++;
		block_size += next_size;
		free_footer = next_footer;
	} else if (before == 0 && after == 0){
		/*Free block before and after*/
		block_size += next_size;
		block_size += prev_size;

		remove_block(prev_header);
		remove_block(next_header);

		internal = internal - 32 - pad - pad_prev - pad_next;
		coalesces++;
		free_header = prev_header;
		free_footer = next_footer;
	}

	free_header->header.block_size = block_size >> 4;
	free_header->header.alloc = 0;

	free_footer->block_size = block_size >> 4;
	free_footer->alloc = 0;

	add_freeblock(free_header);

	return free_header;
}

void *address_check(void* ptr) {
	char* address = NULL;

	address = (char*)ptr - SF_HEADER_SIZE;
	if (ptr == NULL){
		errno = EFAULT;
		return NULL;
	}

	if (address < heap_start || address >= end_heap) {
		errno = EFAULT;
		return NULL;
	}

	sf_free_header* block_header = (sf_free_header*) address;
	sf_footer* block_footer = (sf_footer*) ((char*)address + (block_header->header.block_size << 4) - SF_HEADER_SIZE);

	if ((char*)block_footer >= end_heap) {
		errno = EFAULT;
		return NULL;
	}

	if (block_header->header.alloc == 0) {
		errno = EFAULT;
		return NULL;
	}

	if (block_header->header.block_size != block_footer->block_size || block_header->header.alloc != block_footer->alloc) {
		errno = EFAULT;
		return NULL;
	}

	return address;
}


void *sf_malloc(size_t size){
	/*if size == 0 return null; no memory to allocate*/
	if (size <= 0 || size > 16368) {
		errno = EINVAL;
		return NULL;
	}

	char* ptr = NULL; 
	sf_footer* freelist_footer = NULL;
	if (freelist_head == NULL) {
		/*Check to see if the first page has been called by looking for freelist*/
		/*Create the first page of memory. Points to the start of the new page*/
		freelist_head = sf_sbrk(1);
		if ((char*)freelist_head == (char*) -1) {
			freelist_head = NULL;
			errno = ENOMEM;
			return NULL;
		}
		end_heap = (char*) freelist_head;
		ptr = (char*) freelist_head;
		ptr -= 4096;

		if (heap_start_marker == 0) { /*Check to see is heap start is already set*/
			heap_start = ptr;
			heap_start_marker = 1;
		}

		freelist_head = (sf_free_header*) ptr;
		freelist_head->header.alloc = 0;
		freelist_head->header.block_size = 4096 >> 4;
		freelist_footer = (sf_footer*) (ptr + 4096 - SF_HEADER_SIZE);
		freelist_footer->block_size = 4096 >> 4;
		freelist_footer->alloc = 0;
		freelist_head->prev = NULL;
		freelist_head->next = NULL;
	}

	/*8 bytes for header + 8 bytes for footer + size = payload + padding*/
	/*Should be a multiple of 16*/
	/*Calculate the padding needed*/
	size_t block_size = (2 * SF_HEADER_SIZE) + size;
	size_t remainder = block_size % 16;
	size_t padding = 0;
	if (remainder != 0) {
		padding = 16 - remainder;
	}	
	block_size += padding;

	char* address = NULL; 

	sf_free_header* current_block = freelist_head;
	sf_free_header* free_block = NULL;
	sf_footer* block_footer = NULL;
	sf_footer* free_footer = NULL;
	size_t diff = 0;
	size_t real_value = 0;

	while (current_block != NULL) {
		real_value = current_block->header.block_size << 4;
		if(real_value >= block_size){
			diff = real_value - block_size;
			address = (char*) (current_block);
			remove_block (current_block); 

			if (diff < 32) {	
				current_block->header.padding_size = padding + diff;
				current_block->header.alloc = 1;
				block_size = real_value;

			} else {
				current_block->header.block_size = block_size >> 4;
				current_block->header.padding_size = padding;
				current_block->header.alloc = 1;

				/*Handle freeblock*/
				free_block = (sf_free_header*)(address + block_size);
				free_block->header.alloc = 0;
				free_block->header.block_size = diff >> 4;
				free_footer = (sf_footer*)((char*)free_block + diff - SF_HEADER_SIZE);
				free_footer->alloc = 0;
				free_footer->block_size = diff >> 4;
				add_freeblock(free_block);	
			}

			/*Update allocated block footer*/
			block_footer = (sf_footer*)(address + block_size - SF_HEADER_SIZE);
			block_footer->alloc = 1;
			block_footer->block_size = block_size >> 4;

			allocations++;
			internal += (2*SF_HEADER_SIZE) + padding;
			return address + SF_HEADER_SIZE;

		} else {
			current_block = current_block->next;
		}
	}

	/*Does not find anything in the freelist*/
	current_block = sf_sbrk(1);
	/*Make space on the heap of there is nothing in freelist or sbrk doesn't give enough room*/
	while (current_block != (void*)-1) {
		end_heap = (char*) current_block; /*Adjust end of the heap*/
		ptr = (char*) current_block;
		ptr -= 4096; /*ptr = beginning of new heap*/
		current_block->header.block_size = 4096 >> 4;
		/*Footer??? */
		freelist_footer = (sf_footer*)((char*) current_block - SF_HEADER_SIZE);
		freelist_footer->alloc = 0;
		freelist_footer->block_size = 4096 >> 4;

		sf_free_header* ptr_header = (sf_free_header*) ptr;
		ptr_header->header.alloc = 0;
		ptr_header->header.block_size = 4096 >> 4;
		ptr_header = (sf_free_header*)(coalesce(ptr_header)); /*Coalesce new heap with old heap if possible*/

		if((ptr_header->header.block_size << 4) >= block_size) {
			remove_block(ptr_header);
			/*Enough space in the heap */
			diff = (ptr_header->header.block_size << 4) - block_size;
			if (diff < 32) {
				ptr_header->header.alloc = 1;
				ptr_header->header.padding_size = padding + diff;

				block_footer = (sf_footer*)((char*)ptr_header + (ptr_header->header.block_size << 4) - SF_HEADER_SIZE);
				block_footer->alloc = 1;
				block_footer->block_size = ptr_header->header.block_size;
			} else {
				ptr_header->header.block_size = block_size >> 4;
				ptr_header->header.padding_size = padding;
				ptr_header->header.alloc = 1;

				/*Update allocated block footer*/
				block_footer = (sf_footer*)((char*)ptr_header + block_size - SF_HEADER_SIZE);
				block_footer->alloc = 1;
				block_footer->block_size = block_size >> 4;

				/*Handle freeblock*/
				free_block = (sf_free_header*)((char*)ptr_header + block_size);
				free_block->header.alloc = 0;
				free_block->header.block_size = diff >> 4;

				free_footer = (sf_footer*)((char*)free_block + diff - SF_HEADER_SIZE);
				free_footer->alloc = 0;
				free_footer->block_size = diff >> 4;
				add_freeblock(free_block);
			}

			internal += (2*SF_HEADER_SIZE) + padding;
			allocations++;
			return (char*)ptr_header + SF_HEADER_SIZE;
		} else {
			/*get another page*/
			current_block = sf_sbrk(1);
		}
	}
	/*Nothing!*/
	errno = ENOMEM;
	return NULL;
}

void sf_free(void *ptr){
	char* address = (char*)address_check(ptr);

	if (address == NULL) {
		
	} else {
		frees++;
		sf_free_header* free_header = (sf_free_header*) address;
		coalesce(free_header);
	}
}

void *sf_realloc(void *ptr, size_t size){
	size_t old_block_size = 0;
	size_t new_block_size = 0;
	size_t right_block_size = 0;

	char* address = (char*)address_check(ptr); /*Address for the header*/
	if(address == NULL || size <= 0 || size > 16368) {
		errno = EINVAL;
		return NULL;
	}

	/*Valid address header.*/
	sf_free_header* old_free_header = (sf_free_header*) address; /*ptr's header*/

	old_block_size = old_free_header->header.block_size << 4;


	new_block_size = (2 * SF_HEADER_SIZE) + size;
	size_t remainder = new_block_size % 16;
	size_t padding = 0;
	if (remainder != 0) {
		padding = 16 - remainder;
	}	
	new_block_size += padding;


	//new_block_size = calculate_block_size(size);
	//new_block_size += (2*SF_HEADER_SIZE);
	sf_free_header* right_header = NULL;
	sf_footer* right_footer = NULL;
	sf_footer* new_footer = NULL;

	sf_free_header* malloc_ptr = NULL;

	if(new_block_size == old_block_size) {
		return ptr;
	} else if (new_block_size > old_block_size) {
		right_header = (sf_free_header*) (address + old_block_size);

		if ((char*)right_header < end_heap) { /*There is a right side*/
			right_block_size = right_header->header.block_size << 4;
			right_footer = (sf_footer*) ((char*)right_header + right_block_size - SF_HEADER_SIZE);
			if (right_header->header.alloc == 0) { /*Right block can be coalesced*/
				/*Combine the 2 blocks*/
				remove_block(right_header);
				old_free_header->header.block_size = (right_block_size + old_block_size) >> 4;
				new_footer = right_footer;
				new_footer->block_size = (right_block_size + old_block_size) >> 4;
				new_footer->alloc = 1;
				coalesces++;

				if ((old_block_size + right_block_size) >= new_block_size) {
					split_block(old_free_header, new_block_size, old_block_size, padding);
					void* payload = (void*)old_free_header + 8;
					memcpy(payload, ptr, (old_block_size - (2 * SF_HEADER_SIZE)));
					return payload;

				} else {
					malloc_ptr = sf_malloc(size);
					if (malloc_ptr == NULL) {
						errno = ENOMEM;
						return NULL;
					} else {
						/*Copy old payload with old payload size*/
						memcpy(malloc_ptr, ptr, (old_block_size - (2 * SF_HEADER_SIZE)));
						sf_free(ptr);
						return malloc_ptr;
					}
				}
			} else {
				malloc_ptr = sf_malloc(size);
				if (malloc_ptr == NULL) {
					errno = ENOMEM;
					return NULL;
				} else {
					/*Copy old payload with old payload size*/
					memcpy(malloc_ptr, ptr, (old_block_size - (2 * SF_HEADER_SIZE)));
					sf_free(ptr);
					return malloc_ptr;
				}
			}
		} else {
		malloc_ptr = sf_malloc(size);
			if (malloc_ptr == NULL) {
				errno = ENOMEM;
				return NULL;
			} else {
				/*Copy old payload with old payload size*/
				memcpy(malloc_ptr, ptr, (old_block_size - (2 * SF_HEADER_SIZE)));
				sf_free(ptr);
				return malloc_ptr;
			}
		}
	} else {
		split_block(address, new_block_size, old_block_size, padding);
		return ptr;
	}
	errno = ENOMEM;
	return NULL;
}

int sf_info(info* meminfo){
	external = 0;
	/*Go through freelist and get the external stuff*/
	sf_free_header* current_block = freelist_head;
	while (current_block != NULL) {
		/*Add up freeblocks payload*/
		external += ((current_block->header.block_size << 4) - (2 * SF_HEADER_SIZE));
		current_block = current_block->next;
	}

	if (meminfo != NULL) {
		meminfo->internal = internal;
		meminfo->external = external;
		meminfo->allocations = allocations;
		meminfo->frees = frees;
		meminfo->coalesce = coalesces;
		return 0;
	} else{
		return -1;
	}
	

 	
}
