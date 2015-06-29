#ifndef COMMAND_H
#define COMMAND_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "alloc.h"
#include "command-internals.h"
// UCLA CS 111 Lab 1 command interface

typedef int bool;
#define false 0
#define true 1
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

/*********************************************************************/
typedef enum token_type 
  {
	WORD,  // token
	IN,    // <
	OUT,   // >
    AND,   // &&
	OR,	   // ||
    SEQ,   // \n ;
    PIPE,  // | 
    LB,  // ( 
	RB,   // )
	COMMENT, // #
	NEW_LINE,
	BACK_SLASH,
	END_OF_FILE,
	SEQ_ENTER
  } token_type;
typedef char* token;


/*********************************************************************/
/*********************************************************************/
typedef struct pair {
	token_type key;
	char* value;
} pair;

// Queue
struct node{
	 pair* value;
	 struct node * next; 
};


typedef struct queue{
	 struct node * head;
	 struct node * tail;
	 unsigned int size;
} queue;

queue 
q_empty();

bool isempty(queue q);

queue enqueue(pair* in, queue q);

queue dequeue(queue q);

queue destroy(queue q);

pair* next(queue q);

pair*
b_pair(token_type key, char* value);

int 
free_pair(pair*);
/*********************************************************************/

command_t 
read_seq(command_t holder, queue *s);
command_t 
read_subshell( queue *s);
command_t 
read_simple_command( queue *s);
command_t 
read_single_command( queue s);
command_t 
read_pipeline(command_t holder, queue *s);
command_t 
read_andor(command_t holder, queue *s);

/* Return the pointer that points to the next nonwhitespace character */
char*
eat_whitespace(char*);
/* return number of continous char in current position, 0 if current character is not a word */
int 
eat_word(char *);
/* return number of continous char that is special character at current position. 0 if it is not a special character */
int 
eat_special(char *);
/* return number of character after current position if current position is a # */
int 
eat_comment(char *);


#define STREAM_BUFFER_SIZE 100
#define UP_RATE 1.5


/** isWord**/
#define isNum(c) ((c) >= '0' && (c) <= '9')
#define	isLet(c) (((c) >= 'A' && (c) <= 'Z') || ((c) >= 'a' && (c) <= 'z'))
#define isvWord(c) ((c)=='!'||(c)=='%'||(c)=='+'||(c)==','|| \
	(c)=='-'||(c)=='.'||(c)=='/'||(c)==':'||(c)=='@'||(c)=='^'||(c)=='_')

#define is_word(c) (isNum((c)) || isLet((c)) || isvWord((c)))



queue partition(char* input);
void lineError(int numLine);


struct command_stream {
	
	command_t root;
	int iterator;

	command_t* command_queue;
	size_t size;

};
//TODO delete
command_stream_t
build_token_tree(queue);

#endif
