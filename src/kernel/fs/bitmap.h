/*
 *  bitmap.h
 *  SO-FS
 *
 *  Created by Cristian Pereyra on 19/10/11.
 *  Copyright 2011 My Own. All rights reserved.
 *
 */

typedef struct bitmap bitmap;

bitmap * bitmap_init(int real_block_size, int block_size, void * data);

unsigned int bitmap_read(bitmap * bm, unsigned int index);

void bitmap_write(bitmap * bm, unsigned int index, unsigned int value);

unsigned int bitmap_max_blocks(bitmap * bm, unsigned int size, unsigned int val);

unsigned int bitmap_block_count(bitmap * bm, unsigned int size, unsigned int val);

unsigned int bitmap_first_valued(bitmap * bm, unsigned int size, unsigned int val);