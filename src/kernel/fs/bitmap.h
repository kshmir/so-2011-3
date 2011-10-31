/*
 *  bitmap.h
 *  SO-FS
 *
 *  Created by Cristian Pereyra on 19/10/11.
 *  Copyright 2011 My Own. All rights reserved.
 *
 */

typedef struct bitmap bitmap;

// Stores a gapped bitmap and allows us to access it as an array, jumping it's gaps
// real_block_size is <= block_size. This implementation 'jumps' all the gaps between block_size and real_block_size.
// For example:
// With real_block_size = 6 and block_size = 8
// The bitmap      : 0011001100001100
// indexes:          0123456789abcdef			

// would be seen as: 001100XX110000XX
// indexes:          012345  6789ab
// It would 'jump' the X's.

// This is useful for the bitmaps in the EXT2 filesystem

// Starts a bitmap
// real_block_size: real_block_size is always < to block_size, and makes the gaps between blocks after n > real_block_size
// block_size:      the actual size of the block
// data:            the pointer to the data there the bitmap is held.
bitmap * bitmap_init(int real_block_size, int block_size, void * data);

// Reads from a bitmap in the given index
unsigned int bitmap_read(bitmap * bm, unsigned int index);

// Writes inside the bitmap, with the given index and value
void bitmap_write(bitmap * bm, unsigned int index, unsigned int value);

// Tells the maximum amount of blocks with the given value inside the bitmap
unsigned int bitmap_max_blocks(bitmap * bm, unsigned int size, unsigned int val);

// Tells the amount of blocks with the given value inside the bitmap
unsigned int bitmap_block_count(bitmap * bm, unsigned int size, unsigned int val);

// Gives the index of the first value which matches val
unsigned int bitmap_first_valued(bitmap * bm, unsigned int size, unsigned int val);