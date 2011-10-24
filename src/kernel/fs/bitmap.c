/*
 *  bitmap.c
 *  SO-FS
 *
 *  Created by Cristian Pereyra on 19/10/11.
 *  Copyright 2011 My Own. All rights reserved.
 *
 */

#include "bitmap.h"
#include "../../libs/stdlib.h"

#define BYTE 8

// Stores a gapped bitmap and allows us to access it as an array, jumping it's gaps
// real_block_size is <= block_size. This implementation 'jumps' all the gaps between block_size and real_block_size.
// For example:
// With real_block_size = 6 and block_size = 8
// The bitmap      : 0011001100001100
// indexes:          0123456789abcdef			

// would be seen as: 001100XX110000XX
// indexes:          012345  6789ab
// It would 'jump' the X's.

struct bitmap {
	int		real_block_size;
	int		block_size;
	void *	data;
};

bitmap * bitmap_init(int real_block_size, int block_size, void * data) {
	bitmap * bm = (bitmap *) malloc(sizeof(bitmap));
	bm->real_block_size = real_block_size;
	bm->block_size = block_size;
	bm->data = data;
	return bm;
}

unsigned int bitmap_read(bitmap * bm, unsigned int index) {
	
	// This code makes the weird indexing.
	unsigned int base = index / bm->real_block_size;
	unsigned int off = index % bm->real_block_size;
	index = base * bm->block_size + off;
	
	
	unsigned int offset = index % BYTE + 1;
	unsigned int bytes = index / BYTE;

	

	
	return (*((char*)bm->data + bytes) >> (BYTE - offset)) & 0x1;
}

void bitmap_write(bitmap * bm, unsigned int index, unsigned int value) {
	
	// This code makes the weird indexing.
	unsigned int base = index / bm->real_block_size;
	unsigned int off = index % bm->real_block_size;
	index = base * bm->block_size + off;
	
	unsigned int offset = index % BYTE;
	unsigned int bytes = index / BYTE;

	
	
	if (value) {
		*((char*)bm->data + bytes) |= 128 >> offset;
	} else {
		*((char*)bm->data + bytes) &= ~(128 >> offset);
	}
}

unsigned int bitmap_max_blocks(bitmap * bm, unsigned int size, unsigned int val) {
	unsigned int i = 0;
	unsigned int max = 0, c = 0;
	for (; i < size; i++) {
		c = (bitmap_read(bm, i) == val) ? c + 1 : 0;
		if (c > max) {
			max = c;
		}
	}
	return max;
}

unsigned int bitmap_block_count(bitmap * bm, unsigned int size, unsigned int val) {
	unsigned int i = 0;
	unsigned int c = 0;
	for (; i < size; i++) {
		c += (bitmap_read(bm,i) == val) ? 1 : 0;
	}
	return c;
}

unsigned int bitmap_first_valued(bitmap * bm, unsigned int size, unsigned int val) {
	unsigned int i = 0;
	for (; i < size; i++) {
		if (bitmap_read(bm, i) ==  val)
			return i;
	}
	return (unsigned int) -1;
}