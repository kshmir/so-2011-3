/*
 *  list.c
 *  so-2011
 *
 *  Created by Cristian Pereyra on 03/08/11.
 *  Copyright 2011 My Own. All rights reserved.
 *
 */

#include "list.h"


typedef struct node * node;

// List node
struct node {
	void * 			data;
	node	next;
};

// Base list structure
struct list {
	node    header;
	int		size;
};


// Inits the list
list list_init() {
	list ret = (list) malloc(sizeof(struct list));
	ret->size = 0;
	ret->header = NULL;
	return ret;
}

// Adds the pointer to the list.
int list_add(list p, void * obj) {
	if (p == NULL) {
		return -1;
	}
	
	node last = NULL;
	node current = p->header;
	node n = NULL;
	
	n = (node) malloc(sizeof(struct node));


	if (n != NULL) {
		n->data = obj;
		n->next = NULL;
	}
	else {
		return -1;
	}

	if (current == NULL) {
		p->header = n;
	}
	else {
		while (current) {
			last = current;
			current = current->next;
		}
		if (last) {
			last->next = n;
		}
		else {
			p->header->next = n;
		}
	}
	p->size++;
	return 1;
}

// Gets the pointer from a given index
void *  list_get(list p, int index) {
	if (p == NULL) {
		return NULL;
	}
	node current = p->header;
	int i = 0;

	while(current) {
		if (i == index) {
			return current->data;
		}
		current = current->next;
		i++;
	}
	return NULL;
}

// Free's up the list
void list_free_with_data(list p) {
	foreach(void *, n, p) {
		free(n);
	}
	list_free(p);
}

void list_free(list p) {
	if (p == NULL) {
		return;
	}
	node actual = p->header;
	free(p);
	while (actual != NULL) {
		node aux = actual->next;
		free(actual);
		actual = aux;
	}
}

int list_size(list l) {
	int size = l->size;
	return size;
}



list list_from_ptrarray_w_count(int size, size_t block_size, void * ptr) {
	list l = list_init();
	int i = 0;
	for(; i < size; i++) {
		list_add(l, ((char*)ptr + block_size * i));
	}
	return l;
}

list list_from_ptrarray_null_ended(char ** values)  {
	list l = list_init();
	int i = 0;
	for(; values[i] != NULL; i++) {
		list_add(l, values[i]);
	}
	return l;
}


// Inserts data to the list.
// Returns -1 if it's a in invalid operation, or the index if it's valid.
int list_insert(list p, int index, void *  ptr) {
	if (p == NULL || ptr == NULL || index < 0 || index > p->size) {
		return -1;
	}
	if (index == 0) {
		node old_ref = p->header;
		node ptr_node = (node) malloc(sizeof(struct node));
		ptr_node->data = ptr;
		ptr_node->next = old_ref;
		p->header = ptr_node;
		p->size++;

		return index;
	}
	else if (index == p->size) {
		list_add(p, ptr);
		return index;
	}
	else {
		node aux = NULL;
		node ptr_node = (node) malloc(sizeof(struct node));
		node cursor = p->header;
	
		int i = 0;
		while(cursor->next && i != index) {
			cursor = cursor->next;
			i++;
		}
		aux = cursor->next;
		ptr_node->next = aux;
		cursor->next = ptr_node;
		
		ptr_node->data = ptr;
		p->size++;
		return index;
	}
	return -1;
}

int list_remove(list l, int index) {
	if (index < 0 || l == NULL || index >= l->size) {
		return -1;
	}
	if (index == 0)
	{
		node n = l->header;
		l->header = n->next;
		free(n);
		l->size--;
	}
	else {
		int i = 0;
		node last = NULL;
		node n = l->header;
		while(i != index) {
			last = n;
			n = n->next;
			i++;
		}
		last->next = n->next;
		free(n);
		l->size--;
	}
	return index;
}

int list_indexOf(list p, void * ptr, comparer comp) {
	if (ptr == NULL)
		return -1;
	node n = p->header;
	
	int i = 0;
	while(n != NULL)
	{
		if (comp(n->data,ptr) == 0) {
			return i;
		}
		
		i++;
		n = n->next;
	}
	return -1;
}

void * list_header(list p) {
	return p->header;
}

void * list_node_next(void * n) {
	if (n == NULL) {
		return NULL;
	}
	return ((node)n)->next;
}

void * list_node_value(void * n) { 
	if (n == NULL) {
		return NULL;
	}
	
	return ((node)n)->data;
}
