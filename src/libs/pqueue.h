#ifndef _PQUEUE_H_
#define _PQUEUE_H_

typedef struct PQueue PQueue;

PQueue * pqueue_init(int size, int qsize);

int pqueue_enqueue(PQueue * q, void * data, int priority);

int pqueue_count(PQueue * q);

int pqueue_isempty(PQueue * q);

int pqueue_isfull(PQueue * q);

void * pqueue_dequeue(PQueue * q);

void * pqueue_peek(PQueue * q);

#endif