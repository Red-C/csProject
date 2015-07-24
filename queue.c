#include "queue.h"


//return a pointer to an empty queue.
queue* queue_init()
{
	queue* newQueue = (queue*)malloc(sizeof(queue));
	newQueue->head = NULL;
	newQueue->tail = NULL;
	newQueue->size = 0;
	return newQueue;
}

void queue_free(queue* Q)
{
	//first free all nodes.
	while (Q->head)
	{
		queue_node* tmp = Q->head->next;
		free(Q->head);
		Q->head = tmp;
	}

	//then free the queue.
	free(Q);
}

queue* queue_enqueue(queue* Q, void* item)
{
	//first create the new node.
	queue_node* newNode = (queue_node*)malloc(sizeof(queue_node));
	newNode->item = item;
	newNode->next = NULL;

	if(Q->size == 0) {
		Q->head = newNode;
		Q->tail = newNode;
	}
	else {
		Q->tail->next = newNode;
		Q->tail = newNode;
	}

	//return
	Q->size++;
	return Q;
}

void* queue_dequeue(queue* Q)
{
	//if not empty, return item stored in the head node, free it, and set the head pointer to the next node.
	if (Q->head != NULL)
	{
		void* retval = Q->head->item; //the return value
		queue_node* tmp = Q->head;	  //pointer to the node to be free
		Q->head = Q->head->next;	  //set the head pointer to the next node
		Q->size--;
		free(tmp);
		if(Q->size == 0)
		{
			Q->head = NULL;
			Q->tail = NULL;
		}
		return retval;
	}
	//if empty, return null pointer.
	else
		return NULL;
}

bool queue_isempty(queue* Q)
{
	return Q->size == 0;
}

int queue_sizeof(queue* Q)
{
	return Q->size;
}

