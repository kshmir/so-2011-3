#ifndef _QUEUE_H_
#define _QUEUE_H_

typedef struct Queue Queue;

Queue * queue_init(int size);

int queue_enqueue(Queue * q, void * data);

int queue_count(Queue * q);

int queue_isempty(Queue * q);

int queue_isfull(Queue * q);

void * queue_dequeue(Queue * q);

void * queue_peek(Queue * q);

#endif