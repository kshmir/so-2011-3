/*
 *  malloc.c
 *  SO-FS
 *
 *  Created by Cristian Pereyra on 23/11/11.
 *  Copyright 2011 My Own. All rights reserved.
 *
 */

#include "malloc.h"

#define MEM_BLOCK_SIZE  0x1000
#define MEM_HEADER_SIZE 0x80
#define MEM_BLOCK_COUNT 1024


typedef struct mem_metadata {
	char info[MEM_HEADER_SIZE];
} mem_metadata;


mem_metadata metadata[MEM_BLOCK_COUNT];
char         data[MEM_BLOCK_COUNT * MEM_BLOCK_SIZE];

int first_free_index = 0;

void malloc_init()	{
	
}


static void declare_allocd(int log_index, int size) {
	int meta_index = log_index / 128; // Find the block metadata sector.
	int meta_end = log_index % 128;
	int j = 0;
	
	// If using a waste space
	// Before: (Inserting 320 bytes)
	///////////////////////////////////////////////////////////////////
	// Freed block 20   // -1/[20]/-4/000000000000000000000000000000 //
	///////////////////////////////////////////////////////////////////
	// After:
	///////////////////////////////////////////////////////////////////
	// Freed block 10 and -10 Used   // -1/[-10/10]/-4/0000000000000 //
	///////////////////////////////////////////////////////////////////
	if (metadata[meta_index].info[meta_end] > 0) {
		int stored = (size) / 32;
		int remaining = metadata[meta_index].info[meta_end] - stored;
		metadata[meta_index].info[meta_end] = stored * -1;
		if (remaining > 0) {
			for (j = 126; j > meta_end + 1; j--) {
				metadata[meta_index].info[j] = metadata[meta_index].info[j - 1];
			}
			metadata[meta_index].info[j] = remaining;
		}
		

		// printf("\nAdding %d to %d[%d] \n", stored * -1, meta_index, meta_end);
	
	} else {
		// If using a tail
		// Recalculate the space used to maximize the usage of the block
		int first = 1;
		while (size > 0) {
			int block_free_space = 4096;
			for (j = 0; j < 128; j++) {
				if (metadata[meta_index].info[j] > 0) {
					block_free_space -= metadata[meta_index].info[j] * 32;
				} else if (metadata[meta_index].info[j] < 0 && (first || j == 127)){
					block_free_space -= metadata[meta_index].info[j] * -1 * 32;
				}
			}
			
			if (block_free_space > size) {
				
				metadata[meta_index].info[meta_end] = size / 32 * -1;
				// printf("\nAdding %d to %d[%d] \n", size / 32 * -1, meta_index, meta_end);
				size = 0;
			} else {
				metadata[meta_index].info[meta_end] = block_free_space / 32 * -1;
				// printf("\nAdding %d to %d[%d] %d x\n", block_free_space / 32 * -1, meta_index, meta_end, size);
				size -= block_free_space;
			}
			meta_index++;
			meta_end = 127;
			first = 0;
		}
	}
}


int last_i = 0;
void * malloc(size_t size) {
	int i = 0, j = 0;
	int found_size = 0;
	int data_index = -1;
	int found_index = -1;
	
	
	if(!size)
	{
		return NULL;
	}
	
	if(size < 32)
	{
		size = 32;
	} else if (size % 32 != 0){
		size = ((size + 32) / 32) * 32;
	}
	
	// Iterate over free memory.
	for (i = last_i; i < MEM_BLOCK_COUNT; i++) {
		// With no start index.
		if (found_index == -1) {
			int endj = -1;
			int size_loss = 0; // How much is already reserved
			for (j = 0; j < 128 && size_loss < 4096; j++) {
				
				// If reserved space...
				if (metadata[i].info[j] < 0) {
					///////////////////////////////////////////////////////////////////
					// Used block       // §[-1]/20/[-4]/0000000000000000000000000000 //
					///////////////////////////////////////////////////////////////////
					size_loss += 32 * metadata[i].info[j] * -1;
				} else {
					// "Waste space"
					///////////////////////////////////////////////////////////////////
					// Freed block      // -1/[20]/-4/000000000000000000000000000000 //
					///////////////////////////////////////////////////////////////////
					
					// If freed' block
					
					found_size = 32 * metadata[i].info[j];
					
					if (found_size >= size) {
						found_index = i * 0x80 + j; // It MUST be bigger or won't work. 
						data_index = i * 0x80 + size_loss / 32;
						size_loss = 4096;
						////printf("Got freed\n");
						// You cannot put something bigger than 20 * 32 (in example)
					} else {
						found_size = 0;
						size_loss += 32 * metadata[i].info[j];
					}
				}
				
				if (!metadata[i].info[j] && j < 127) {	
					endj = j;
					j = 126;
				}
			}
			
			///////////////////////////////////////////////////////////////////
			// First 0 of alloc chain // -1/20/-4/[0]000000000000000000000   //
			///////////////////////////////////////////////////////////////////
			// If 0 ended space.
			if (endj != -1 && !metadata[i].info[endj] && size_loss < 4096) {
				found_index = i * 0x80 + endj;
				data_index = i * 0x80 + size_loss / 32;
				found_size = 4096 - size_loss;
				////printf("Got tail 2 %d %d\n", size_loss, found_size);
			}
			// WITH a start index.
		} else {
			if (metadata[i].info[0] == 0 && metadata[i].info[127] == 0) {
				///////////////////////////////////////////////////////////////////
				// Empty Block // [0]00000000000000000000000000000000000000000000//
				///////////////////////////////////////////////////////////////////
				found_size += 4096 + metadata[i].info[127] * 32;	
			} else 	{
				int endj = -1;
				for (j = 0; j < 128; i++) {
					///////////////////////////////////////////////////////////////////
					// Freed block      // [20]/-4/-10/00000000000000000000000000000 //
					///////////////////////////////////////////////////////////////////
					// If reserved space... then we haven't found nuff space.
					if (metadata[i].info[j] < 0) {
						found_index = -1;
						i--; // Go back so we can try again.
						break;
					} else if (metadata[i].info[0] > 0) {
						///////////////////////////////////////////////////////////////////
						// Freed block      // [20]...[2]/-4/-10/00000000000000000000000 //
						///////////////////////////////////////////////////////////////////
						found_size += 32 * metadata[i].info[0];
						if (found_size < size && i < 127 && metadata[i + 1].info[j] < 0) {
							found_index = -1;
							i--; // Go back so we can try again.
							break;
						}
					} 
					
					if (!metadata[i].info[j] && j < 127) {	
						endj = j;
						j = 127;
					}
				}
				
				// If 0 ended space. 
				if (endj != -1 && !metadata[i].info[endj]) {
					found_index = -1;
					i--; // Go back so we can try again.
				}		
			}
		}
		
		if (found_size >= size && found_index != -1) {
			declare_allocd(found_index, size);
			////printf("Alloc place: %d\n", data_index);
			break;
		}
		
		if(i == MEM_BLOCK_COUNT - 1)
		{
			i = 0;
		}
	}
	last_i = i;
	
	char * ptr = (void *)(data + data_index * 32);
	i = 0;
	for(i = 0; i < size; ++i)
	{
		ptr[i] = 0;
	}
	
	return ptr;
}
	
// Roughly callocs some memory
void * calloc(size_t size, size_t cols)	{
	char* ret = (char*)malloc(size * cols);
	int i = 0;
	for(; i < size; i++)
        ret[i] = 0;
	return ret;
}

// Roughly reallocs some memory
void * realloc(void * ptr, size_t size,size_t old_size)
{
    char * ret = (char*)malloc(size);
    int i = 0;
	for(; i < old_size; ++i) {
		ret[i] = ((char*)ptr)[i];
	}
	free(ptr);
	
    return ret;
}



// Shrinks a given section of a block
static void shrink_block(int meta_index, int sum, int amount, int start_i) {
	int j;
	amount--;
	for (j = start_i + amount; j < 126 ; j++) {
		metadata[meta_index].info[j - amount] = metadata[meta_index].info[j];
	}
	metadata[meta_index].info[start_i] = sum;
}

// Shrinks all repeated freed blocks
static void rebuild_block(int meta_index) {
	int all_positive = 1;
	int j = 0;
	for (j = 0; j < 127 && all_positive; j++) {
		all_positive = metadata[meta_index].info[j] >= 0;
	}
	if (all_positive) {
		for (j = 0; j < 127; j++) {
			metadata[meta_index].info[j] = 0;
		}
	} else {
		int positive_sum_i = -1;
		int positive_sum = 0;
		int positive_indexes = 0;
		for (j = 0; j < 127; j++) {
			if (metadata[meta_index].info[j] > 0) {
				positive_sum += metadata[meta_index].info[j];
				positive_indexes++;
				if (positive_sum_i == -1) {
					positive_sum_i = j;
				}
			} else if (metadata[meta_index].info[j] <= 0) {
				if (positive_indexes >= 2) {
					shrink_block(meta_index, positive_sum, positive_indexes, positive_sum_i);
					j -= positive_indexes - 1;
					positive_indexes = 0;
				} else {
					positive_sum = 0;
					positive_indexes = 0;
					positive_sum_i = -1;
				}
			}
		}
	}

	
}

int free(void * ptr) {
	int index       = (int)((int)ptr - (int)data) / 32; // Decode it's data index
	int meta_index  = index / 128;
	int ph_meta_end = index % 128;
	// We don't know meta_end now, we need to calculate it based on the block
	// Then we need to iterate and free everything up.
	int meta_end = 0; 
	

	

		ph_meta_end -= metadata[meta_index].info[127] * -1;
		int j = 0;		
		if (ph_meta_end > 0) {
			for (; j < 127; j++) {			
				if (metadata[meta_index].info[j] > 0) {
					ph_meta_end -= metadata[meta_index].info[j];
				} else {
					ph_meta_end += metadata[meta_index].info[j];
				}

				
				if (ph_meta_end == 0) {
					meta_end++;	
					break;
				} else if (ph_meta_end < 0){
					////printf("ERROR 2\n");
				} else {
					meta_end++;	
				}
			}
		}

		
		if (metadata[meta_index].info[meta_end] >= 0) {
			////printf("Double free!\n");
		} else {
			if (metadata[meta_index].info[meta_end] != -128) {
				metadata[meta_index].info[meta_end] *= -1;
			} else {
				metadata[meta_index].info[meta_end] = 0;
			}
		}

		rebuild_block(meta_index);
		
		meta_index++;
		do  {	
			int aux = - metadata[meta_index].info[127];
			for (j = 127; j > 0; j--) {
				metadata[meta_index].info[j] = metadata[meta_index].info[j - 1];
			}
			metadata[meta_index].info[0] = aux;
			rebuild_block(meta_index);
			meta_index++;
		} while (metadata[meta_index].info[127] != 0 && metadata[meta_index].info[0] == 0);
	
}