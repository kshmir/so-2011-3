#include "../../include/defs.h"

/*
 *  malloc.h
 *  SO-FS
 *
 *  Created by Cristian Pereyra on 23/11/11.
 *  Copyright 2011 My Own. All rights reserved.
 *
 */

void * malloc(size_t size);

void * realloc(void * ptr, size_t size,size_t old_size);

int free(void * ptr);