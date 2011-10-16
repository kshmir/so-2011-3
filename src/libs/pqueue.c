#include "queue.h"
#include "pqueue.h"

#include "../../include/defs.h"

struct PQueue {
	int 		size;
	int			qsize;
	Queue **	queues;
};

PQueue * pqueue_init(int size, int qsize)	{
	PQueue * pq = (PQueue *) malloc(sizeof(PQueue));
	pq->size    = size;
	pq->queues  = (Queue **) malloc(sizeof(Queue *) * (size + 1));
	pq->qsize   = qsize;
	
	int i = 0;
	for(; i < size + 1; ++i)	{
		pq->queues[i] = queue_init(qsize);
	}
	
	return pq;
}

int pqueue_enqueue(PQueue * q, void * data, int priority)	{
	int i = 0;
	if (priority > q->size + 1) { 
		priority = q->size + 1;
	}
	
	// priority 1 2 3 4 5
	// 
	// i        5 4 3 2 1
	
	i = q->size - priority;
	queue_enqueue(q->queues[i], data);
}

int pqueue_count(PQueue * q)	{
	int i = 0;
	int c = 0;
	for(; i < q->size + 1; ++i)	{
		c += queue_count(q->queues[i]);
	}
	return c;
}

int pqueue_isempty(PQueue * q)	{
	return pqueue_count(q) == 0;
}

int pqueue_isfull(PQueue * q)	{
	return pqueue_count(q) == (q->size + 1) * q->qsize;
}


static void pqueue_rotate(PQueue * q) {
	Queue * offset = q->queues[0];
	int i = 1;
	for(; i < q->size + 1; ++i)	{
		q->queues[i - 1] = q->queues[i];
	}
	q->queues[q->size] = offset;
}

void * pqueue_dequeue(PQueue * q)	{
	if(pqueue_isempty(q))	{
		return NULL;
	}
	while(queue_isempty(q->queues[0]))	{
		pqueue_rotate(q);
	}
	return queue_dequeue(q->queues[0]);
}

void * pqueue_peek(PQueue * q)	{
	if(pqueue_isempty(q))	{
		return NULL;
	}
	while(queue_isempty(q->queues[0]))	{
		pqueue_rotate(q);
	}
	return queue_peek(q->queues[0]);	
}
