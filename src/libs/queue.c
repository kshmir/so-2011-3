#include "queue.h"
#include "stdlib.h"

struct Queue {
	int 		size;
	int			write_ptr;
	int			read_ptr;
	int *	data;
};

Queue * queue_init(int size) {
	Queue * q    = (Queue *) malloc(sizeof(Queue));
	q->size      = size;
	q->write_ptr = 0;
	q->read_ptr  = 0; 
	q->data      = (int *) malloc(sizeof(int *) * size);
	return q;                                           	
}

int queue_enqueue(Queue * q, void * data) {
	if(!queue_isfull(q)) {
		q->data[q->write_ptr++] = (int) data;
		if(q->write_ptr == q->size){
			q->write_ptr = 0;
		}
		return 1;
	}
	return 0;
}

int queue_isempty(Queue * q) {
	return q->write_ptr == q->read_ptr;
}

int queue_isfull(Queue * q) { 
	return q->write_ptr == q->read_ptr - 1 || 
				 (q->read_ptr == q->size - 1 && 
					q->write_ptr == 0);
}

void * queue_dequeue(Queue * q) {
	if(queue_isempty(q)) {
		return NULL;
	}
	
	int ret = q->data[q->read_ptr++];
	if(q->read_ptr == q->size){
		q->read_ptr = 0;
	}
	return (void *) ret;
}

void * queue_peek(Queue * q) {
	return (void *) q->data[q->read_ptr];
}