/*
 *  list.h
 *  so-2011
 *
 *  Created by Cristian Pereyra on 03/08/11.
 *  Copyright 2011 My Own. All rights reserved.
 *
 */

#include "../../include/kasm.h"
#include "../../include/defs.h"

#ifndef _LIST_H_
#define _LIST_H_

typedef int (*comparer)(void *, void *);

// Este foreach es ninja
#define foreach(type, item, list) \
	void * _item = NULL; \
	type   item  = NULL; \
	for(_item = (void *)list_header(list), item = list_node_value(_item); \
		_item != NULL && item != NULL; \
		_item = (void *)list_node_next(_item), item = list_node_value((_item != NULL) ? _item : NULL))

#define foreach_(type, item, list) \
	void * _item = NULL; \
	type   item  = NULL; \
	for(_item = (void *)list_header(list), item = list_node_value(_item); \
		_item != NULL && item != NULL; \
		_item = (void *)list_node_next(_item), item = list_node_value((_item != NULL) ? _item : NULL))

#define foreach_next(item) \
		_item = (void *)list_node_next(_item); item = list_node_value((_item != NULL) ? _item : NULL);

// This makes all calls to list equal to "struct list *"
// So BEWARE that any call to list is a pointer by itself.
typedef struct list * list;

// Inits the list
list list_init();

// Free's up the list
void list_free(list l);

// Adds an element to the list.
int list_add(list p, void * obj);

// Size of the given list
int list_size(list p);

// Retreives an element in the given index of the list
void * list_get(list p, int index);

// Inserts an element in the given index of the list
int list_indexOf(list p, void * ptr, comparer comp);

// Removes an element in the given index of the list
int list_remove(list p, int index);

list list_from_ptrarray_w_count(int size, size_t block_size, void * ptr);

list list_from_ptrarray_null_ended(char ** values);

void list_free_with_data(list p);

void * list_header(list p);

void * list_node_next(void * n);

void * list_node_value(void * n);

#endif
