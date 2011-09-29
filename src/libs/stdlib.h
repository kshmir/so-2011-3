/*
 * stdlib.h
 *
 *  Created on: May 11, 2011
 *      Author: cristian
 */

#ifndef _STDLIB_H_

#include "../monix/monix.h"

#define _STDLIB_H_

void * malloc(size_t size);
void * calloc(size_t size, size_t cols);
void * realloc(void* ptr, size_t size, size_t old_size);
void free(void* ptr);
int time(void *ptr);
int srand(int i);
int rand();
void memcpy(void *s, char* c, int n);

#endif /* STDLIB_H_ */
