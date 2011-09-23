#include "queue.h"
#include "stdlib.h"

struct Queue {
	int 		size;
	int			count;
	int			write_ptr;
	int			read_ptr;
	int *		data;
};

Queue * queue_init(int size) {
	Queue * q    = (Queue *) malloc(sizeof(Queue));
	q->size      = size;
	q->write_ptr = 0;
	q->read_ptr  = 0; 
	q->count     = 0;
	q->data      = (int *) malloc(sizeof(int *) * size);
	return q;                                           	
}

int queue_count(Queue * q) {
	return q->count;
}

int queue_contains(Queue * q, void * ptr) { 
	int reads = q->count;
	int i = q->read_ptr;
	for(; reads > 0; reads--, i++)
	{
		if(i == q->size){
			i = 0;
		}
		
		if(q->data[i] == (int) ptr) {
			return 1;
		}
	}
	
	return 0;
}

int queue_enqueue(Queue * q, void * data) {
	if(!queue_isfull(q) && !queue_contains(q, data)) {
		q->data[q->write_ptr++] = (int) data;
		if(q->write_ptr == q->size){
			q->write_ptr = 0;
		}
		q->count++;
		return 1;
	}
	return 0;
}

int queue_isempty(Queue * q) {
	return q->count == 0;
}

int queue_isfull(Queue * q) { 
	return q->size == q->count;
}

void * queue_dequeue(Queue * q) {
	if(queue_isempty(q)) {
		return NULL;
	}
	
	int ret = q->data[q->read_ptr++];
	q->count--;
	if(q->read_ptr == q->size){
		q->read_ptr = 0;
	}
	return (void *) ret;
}

void * queue_peek(Queue * q) {
	return (void *) q->data[q->read_ptr];
}