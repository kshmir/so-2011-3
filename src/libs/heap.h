/*
 *  The code of this heap is based on this implementation
 *  http://cprogramminglanguage.net/binary-heap-c-code.aspx
 *
 *  Created by Cristian Pereyra on 07/08/11.
 *  Copyright 2011 My Own. All rights reserved.
 */

#ifndef _HEAP_H_
#define _HEAP_H_

struct HeapStruct;
typedef struct HeapStruct * heap;

typedef int (*comparer)(void *, void *);

heap heap_init(int max_elements, comparer comp);

void heap_clear(heap H);

void heap_insert(void * X, heap H);

void * heap_remove_min(heap H);

void * heap_get_min(heap H);

int heap_empty(heap H);

int heap_full(heap H);

heap heap_expand(heap h);

void heap_free(heap H);

#endif