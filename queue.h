#ifndef QUEUE_H
#define QUEUE_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct queue_node 
{
	void *item;
	struct queue_node *next;
} queue_node;

typedef struct queue 
{
	queue_node* head;
	queue_node* tail;
	int size;
} queue;

queue* queue_init();
void queue_free(queue* Q);
queue* queue_enqueue(queue* Q, void* item);
void* queue_dequeue(queue* Q);
int queue_sizeof(queue* Q);
bool queue_isempty(queue* Q);
#endif
