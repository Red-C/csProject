// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include "alloc.h"

#include <error.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*********************************************************************/

/* FIXME: Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  */
/*  
 * tree data structure, 
 * ';' has highest priority
 * && and || has left associative property
 * | Pipe command
 * simple command 
 *  */


struct command_stream {
	
	command_t root;
	command_t iterator;

	char* buffer;
	size_t size;
	size_t cap;
};




/**
 * eat_whitespace TODO
 * param: 
 * 		s(char*): intput stream
 *
 * return:
 * 		char*: pointer to the character after whitespace
 * descr:
 * 		eat all prefix whitespace
 */
char* eat_whitespace(char* s) {
	char* it = s;
	while(*it == ' ' || *it == '\t' || (*it == '\\' && *(it+1) == '\n')){
		if(*it == '\\' && *(it+1) == '\n')
			it++;
		it++;

	}
	return it;
}
bool 
is_whitespace(const char* s) {
	return false;
}

/**
 * eat_word TODO
 * param: 
 * 		it(char*): iterator
 *
 * return:
 * 		length of current word from iterator
 * descr:
 * 		count number of word from iterator	
 */
int
eat_word(char* s) 
{
	char* it = s;
	if(is_word(*it))
	{
		int n = 1;
		it++;
		while(is_word(*it) || *it == ' ') {
			n++;
			it++;
		}
		return n;
	}
	return 0;
}


/**
 * eat_special TODO
 * param: 
 * 		it(char*): iterator
 *
 * return:
 * 		length of current special character from iterator
 * descr:
 * 		return length of special character such as || && | > < 
 */
int
eat_special(char* s) {
	char* it = s;
	switch(*it)
	{
		case '|':
			if(*(it+1) != '|') 	return 1;
			else 				return 2;
			break;
		case '&':
			if(*(it+1) != '&') 	return 0; //TODO ERROR
			else 				return 2;
			break;
		case '>':
			return 1;break;
		case '<':
			return 1;break;
		default:
			return 0;break;
	}
}

/**
 * eat_comment TODO
 * param: 
 * 		it(char*): iterator
 *
 * return:
 * 		length of current special character from iterator
 * descr:
 * 		return length of special character such as || && | > < 
 */
int
eat_comment(char* s) {
	char* it = s;
	int n = 0;
	if(*it == '#')  {
		do
		{
			it++; n++;
		}
		while(*it != '\0' && *it != '\n');
	}

	return n;

}

/**
 * next_token_type TODO
 * param: 
 * 		s-command_stream_t: command array that holds all the tokens
 * return:
 * 		token_type: type of token as enum above 
 * descr:
 * 		analyze next token and return the type, this function will
 * 		not modify the command_stream
 */
token_type 
next_token_type(command_stream_t s)
{
	//TODO analyze and return type of next token
	return UNKNOWN;
}

/**
 * read_token TODO
 * param: 
 * 		s-command_stream_t: command array that holds all the tokens
 * return:
 * 		token: next available token, null is reached end of file
 * descr:
 * 		remove next token and return
 */
token
read_token(command_stream_t s)
{//TODO remove and return next token, null if EOF
	return 0;
}


/**
 * pop_token TODO
 * param: 
 * 		s-command_stream_t: command array that holds all the tokens
 * return:
 * 		int: 1 pop successful
 * 			 0 end of file
 *
 * 		descr:
 * 			remove next token, can be implemented by read_token
 */
int 
pop_token(command_stream_t s)
{//TODO remove first token
	return 0;
}


/**
 * malloc_cmd TODO
 * param: 
 * 		null
 * return:
 * 		allocated command_t struct
 *
 * descr:
 *		allocate memory in heap for command_t struct and return 
 *		the pointer
 */
command_t 
malloc_cmd()
{//TODO initialize and allocate memory for command struct
	return 0;
}


/**
 * read_seq 
 * param: 
 * 		holder(command_t holder) recursive holder, helper to 
 * 		implements left associative tree
 * return:
 * 		root of current command tree
 *
 * descr:
 * 		top level of command string, separated by ';' or '\n'
 * 		this will recursively call read_andor function and itself
 */
command_t 
read_seq(command_t holder, command_stream_t s)
{
	//
	// beginning of current pipeline 
	if( holder == NULL) 
	{
		// read first command, this will be the left child of node if 
		// number of commands in pipeline is more than one
		command_t cur = read_andor(NULL, s);
		return read_seq(cur, s);
	}
	 
	command_t cmd;
	// end of pipeline
	if(next_token_type(s) != SEQ)		
		return holder;
	// allocate memory for current pipeline command
	else  								
	{ 
		pop_token(s); 
		cmd = malloc_cmd();
	}

	// assign left and right child, left associative
	cmd->u.command[0] = holder;
	cmd->u.command[1] = read_andor(NULL, s);
	return read_seq(cmd, s);

}


/**
 * read_subshell
 * param: 
 * 		s(command_stream_t) : command buffer
 * return:
 * 		root of current command tree
 *
 * descr:
 * 		lowest level of command sequence, this will be called by
 * 		read_single_command, which is called by read_pipeline.
 * 		this will call read_seq and build a tree base on the commands
 * 		inside of bracket. if right bracket is not exist, throw exception
 */
command_t 
read_subshell( command_stream_t s) 
{
	command_t cmd = malloc_cmd();
	if(next_token_type(s) == LB)
	{	
		// build subshell
		cmd->type = SUBSHELL_COMMAND;
		pop_token(s);
		cmd->u.subshell_command = read_seq(NULL, s);
		
		// match right shell
		if(next_token_type(s) == RB)	return cmd;
		else 							error(1,0, "invalid subshell format: right bracket is not found");
	}
	else
		error(1,0, "invalid subshell format: left bracket is not found");

  return 0;
}

/**
 * read_simple_command
 * param: 
 * 		s(command_stream_t) : command buffer
 * return:
 * 		root of current command tree
 *
 * descr:
 * 		read next simple command, cut token with white space 
 * 		and copy to struct's u.word array
 * 		check input and output symbol and copy corresponding
 * 		token into struct
 */
command_t 
read_simple_command( command_stream_t s) 
{
	
	command_t cmd = malloc_cmd();
	token t;
	while(next_token_type(s) == WORD)
	{ // TODO allocate memory for double array u.word
	  // TODO not decided to use dynamic array or just limits
	  // TODO array with a constant size
				
	}

	
		

		
  /* FIXME: Replace this with your implementation too.  */
  error (1, 0, "simple command reading not yet implemented");
  return 0;
}

/**
 * read_single_command
 * param: 
 * 		s(command_stream_t) : command buffer
 * return:
 * 		root of current command tree
 *
 * descr:
 * 		jumper for simple command and subshell
 */
command_t 
read_single_command( command_stream_t s) 
{
	// if next token is a left bracket, it get into a subshell command
  if(next_token_type(s) == LB)
	return read_subshell(s);
  else 
	 // simple command otherwise
	return read_simple_command(s);
}

/**
 * read_pipeline
 * param: 
 * 		s(command_stream_t) : command buffer
 * 		holder(command_t): helper for recursive,(left associative)
 * return:
 * 		root of current command tree
 *
 * descr:
 * 		this function will called by read_andor, its priority is lower
 * 		than read_andor and read_seq,(3)
 * 		left associative
 */
command_t 
read_pipeline(command_t holder, command_stream_t s) 
{
	// beginning of current pipeline 
	if( holder == NULL) 
	{
		// read first command, this will be the left child of node if 
		// number of commands in pipeline is more than one
		command_t cur = read_single_command(s);
		return read_pipeline(cur, s);
	}
	 
	command_t cmd;
	// end of pipeline
	if(next_token_type(s) != PIPE)		
		return holder;
	// allocate memory for current pipeline command
	else  								
	{ 
		pop_token(s); 
		cmd = malloc_cmd();
	}

	// assign left and right child, left associative
	cmd->u.command[0] = holder;
	cmd->u.command[1] = read_single_command(s);
	return read_pipeline(cmd, s);

}

/**
 * read_andor
 * param: 
 * 		s(command_stream_t) : command buffer
 * 		holder(command_t): helper for recursive,(left associative)
 * return:
 * 		root of current command tree
 *
 * descr:
 * 		this function will called by read_seq, its priority is lower
 * 		than read_seq,(2)
 * 		left associative
 */
command_t 
read_andor(command_t holder, command_stream_t s) 
{
	// beginning of current complete command
	if( holder == NULL) 
	{
		command_t cur = read_pipeline(NULL, s);
		return read_andor(cur, s);
	}
	 
	command_t cmd;
	// get type of next token, will not modified stream
	token_type ttype = next_token_type(s);
	
	// end of current complete command 
	if(ttype != OR && ttype != AND)		
		return holder;
	// allocate memory for cmd and remove first token {|| &&}
	else 
	{ 
		pop_token(s); 
		cmd = malloc_cmd();
	}

	// OR COMMAND
	if(next_token_type(s) == OR)			cmd->type = OR_COMMAND;
	// AND COMMAND
	else if( next_token_type(s) == AND)		cmd->type = AND_COMMAND;

	cmd->u.command[0] = holder;
	cmd->u.command[1] = read_pipeline(NULL, s);
	// left associtive
	return read_andor(cmd, s);

}



queue
build_token_queue(char* stream, int len)
{
	queue Q;
	char* it = stream;
	char* end = stream + len;
	for( it = stream; it <= end; )
	{
		if(is_whitespace(it))
			it = eat_whitespace(it);
		else if(is_word(*it))
		{
			int n = eat_word(it);
			char* temp = (char *)checked_malloc(sizeof(char) * n + 1);
			strncpy(temp, it, n);
			it += n;
			Q = enqueue(temp, Q);
		}
		else if(eat_special(it) > 0)
		{
			int n = eat_special(it);
			char* temp = (char *)checked_malloc(sizeof(char) * n + 1);
			strncpy(temp, it, n);
			it += n;
			Q = enqueue(temp,Q);
		}
		else
		{
			// TODO count line number
			error(0,1,"unexpected character at line x");
		}
		

	}
	return Q;
}



command_stream_t
make_command_stream (int (*get_next_byte) (void *),
			 void *get_next_byte_argument)
{
	char c;
	while((c = get_next_byte(get_next_byte_argument)) != EOF) {
		putc(c, stdout);
	}
	return 0;
}

command_t
read_command_stream (command_stream_t s)
{
  /* FIXME: Replace this with your implementation too.  */
  error (1, 0, "command reading not yet implemented");
  return 0;
}

/********************************************/



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

queue enqueue(pair* in, queue q)
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

pair* next(queue q)
{
	 return q.head->value;
}

pair*
b_pair(token_type key, token value)
{
	pair* p = (pair*) checked_malloc(sizeof(pair));
	p->key = key;
	p->value = value;
	return p;
}

int 
free_pair(pair*);







/********************************************/
