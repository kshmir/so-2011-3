/*
 *  The code of this heap is based on this implementation
 *  http://cprogramminglanguage.net/binary-heap-c-code.aspx
 *
 *  Created by Cristian Pereyra on 07/08/11.
 *  Copyright 2011 My Own. All rights reserved.
 */

#include "heap.h"

#include <stdlib.h>

#define min_size (1024)
#define MinData (-32767)

struct HeapStruct {
    int Capacity;
    int Size;
    void * * Elements;
	comparer Comparer;
};

heap heap_init(int max_elements, comparer comp) {
    heap H;
	
	
	H = (heap) malloc(sizeof( struct HeapStruct));
	
    H->Elements = (void * *) malloc((max_elements + 1) * sizeof (void *));
	
    H->Capacity = max_elements;
    H->Size = 0;
    *((int*)H->Elements) = (int)NULL;
	H->Comparer = comp;
	
    return H;
}


/* END */

void heap_clear(heap H) {
    H->Size = 0;
}





// Expands the heap if needed
// It makes it the double of size.
heap heap_expand(heap h) {
	int i = 0;
	
	heap h2 = heap_init(h->Capacity * 2 + 1, h->Comparer);
	
	
	for (; i < h->Capacity + 1; i++) {
		((int*)h2->Elements)[i] = ((int*)h->Elements)[i];
	}
	h2->Size = h->Size;
	
	heap_free(h);
	return h2;
}


/* H->Element[ 0 ] is a sentinel */

void heap_insert(void * X, heap H) {
    int i;
	int * elements = (int *) H->Elements;
	
    if (H->Size == H->Capacity) {
		return;
    }
	
    for (i = ++H->Size; i/2 > 0 && H->Comparer((void *)X, (void *)elements[ i / 2 ]) < 0; i /= 2)
        elements[ i ] = elements[ i / 2 ];
    elements[ i ] = (int) X;
}

/* END */


void * heap_remove_min(heap H) {
    int i, Child;
    void * MinElement, * LastElement;
	
	int * elements = (int *) H->Elements;
	
    if (heap_empty(H)) {
        return (void *) elements[ 0 ];
    }
	MinElement = (void *) elements[ 1 ];
    LastElement = (void *) elements[ H->Size-- ];
	
    for (i = 1; i * 2 <= H->Size; i = Child) {
        Child = i * 2;
        if (Child != H->Size && H->Comparer((void *)elements[ Child + 1 ], (void *)elements[ Child ]) <  0)
			Child++;
		
		if (H->Comparer(LastElement,(void *)elements[ Child ]) > 0)
			elements[ i ] = elements[ Child ];
        else
			break;
    }
	elements[ i ] = (int) LastElement;
	return MinElement;
}

void * heap_get_min(heap H) {
	int * elements = (int *) H->Elements;
    if (!heap_empty(H))
        return (void *)elements[ 1 ];
    return  NULL;
}

int heap_empty(heap H) {
    return H->Size == 0;
}

int heap_full(heap H) {
    return H->Size == H->Capacity;
}

void heap_free(heap H) {
    free(H->Elements);
    free(H);
}
