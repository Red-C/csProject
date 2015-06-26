#include <stdio.h>
#include <stdlib.h>
#include "alloc.h"
// UCLA CS 111 Lab 1 command interface

typedef struct command *command_t;
typedef struct command_stream *command_stream_t;

/* Create a command stream from LABEL, GETBYTE, and ARG.  A reader of
   the command stream will invoke GETBYTE (ARG) to get the next byte.
   GETBYTE will return the next input byte, or a negative number
   (setting errno) on failure.  */
command_stream_t make_command_stream (int (*getbyte) (void *), void *arg);

/* Read a command from STREAM; return it, or NULL on EOF.  If there is
   an error, report the error and exit instead of returning.  */
command_t read_command_stream (command_stream_t stream);

/* Print a command to stdout, for debugging.  */
void print_command (command_t);

/* Execute a command.  Use "time travel" if the integer flag is
   nonzero.  */
void execute_command (command_t, int);

/* Return the exit status of a command, which must have previously been executed.
   Wait for the command, if it is not already finished.  */
int command_status (command_t);


command_t 
read_seq(command_t holder, command_stream_t s);
command_t 
read_subshell( command_stream_t s);
command_t 
read_simple_command( command_stream_t s);
command_t 
read_single_command( command_stream_t s);
command_t 
read_pipeline(command_t holder, command_stream_t s);
command_t 
read_andor(command_t holder, command_stream_t s);


/*********************************************************************/
// Queue
struct node{
	 char* value;
	 struct node * next; 
};


typedef struct queue{
	 struct node * head;
	 struct node * tail;
} queue;

queue 
q_empty()
{
	 queue q;
	 q.head = q.tail = NULL;
	 return q;
}


int isempty(queue q)
{
	 if(q.head == NULL)
	   return 1;
	 else
	   return 0;
}

queue enqueue(char* in, queue q)
{
 struct node * item = (struct node *)checked_malloc(sizeof(struct node));
 item->value = in;
 item->next = NULL;

 if(isempty(q))
 	q.head = q.tail = item;
 else
 {
    q.tail->next = item;
  	q.tail = item;
 }
 return q;
};


queue dequeue(queue q)
{
	struct node * temp;
	if(q.head)
	{
		temp = q.head;
		q.head = q.head->next;
		free(temp);
	}
	if(isempty(q))
		return q_empty();
	return q;
}

char* next(queue q)
{
	 return q.head->value;
}





/*********************************************************************/




queue
build_token_queue(char* stream, int len);

typedef enum token_type 
  {
	WORD,  // token
	IN,    // <
	OUT,   // >
    AND,   // &&
	OR,	   // ||
    SEQ,   // \n ;
    PIPE,  // | 
    SIMPLE,// one or more adjacent characters that are ASCII letters (either upper or lower case), digits, or any of: ! % + , - . / : @ ^
    LB,  // ( 
	RB,   // )
	UNKNOWN
  } token_type;
typedef char* token;
#define STREAM_BUFFER_SIZE 100
