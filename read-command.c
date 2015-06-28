// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include "alloc.h"

#include <error.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

unsigned int _line = 0;

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


token_type 
token_type_of(char* p) {
	char* it = p;
	unsigned int n = 0;
	if((n =eat_word(it)) != 0)
	{	
		if(n == strlen(p))
			return WORD;
	}	
	else if((n = eat_comment(p)) != 0)
		return COMMENT;
	else if((n = eat_special(p)) != 0)
	{
		if(n == strlen(p)) {
			if(n == 2) {
				switch(*p) {
				case '|':
					return OR;
				case '&':
					return AND;
				}
			}
			else if(n == 1) {
				switch(*p) {
					case '|':	return PIPE;
					case '<':	return IN;
					case '>':	return OUT;
					case '(':	return LB;
					case ')':	return RB;
					case ';':	return SEQ; 
					case '\n':  return NEW_LINE;
					default:
						return UNKNOWN;
				}
			}
		}
	}

	return UNKNOWN;

}

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
int eat_word(char* s) 
{
	char* it = s;
	if(is_word(*it))
	{
		int n = 1;
		it++;
		while(is_word(*it)) {
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
		case '<':
		case ';':
		case '(':
		case ')':
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
next_token_type(queue s)
{
	while(next(s) != NULL) {
		if(next(s)->key == BACK_SLASH) {
			_line++;
			s = destroy(s);
		}
		else if(next(s)->key == NEW_LINE) {
			_line++;
			return SEQ;
		}
		else
			return next(s)->key;
	}
	return UNKNOWN;
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
	command_t cmd = (command_t)checked_malloc(sizeof(struct command));
	cmd->output = NULL;
	cmd->input = NULL;
	cmd->status = 0;
	return cmd;
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
read_seq(command_t holder, queue *s)
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
	if(next_token_type(*s) != SEQ)		
		return holder;
	// allocate memory for current pipeline command
	else  								
	{ 
		*s = destroy(*s);
		cmd = malloc_cmd();
	}

	// assign left and right child, left associative
	cmd->type = SEQUENCE_COMMAND;
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
read_subshell(queue *s) 
{
	command_t cmd = malloc_cmd();
	if(next_token_type(*s) == LB)
	{	
		// build subshell
		cmd->type = SUBSHELL_COMMAND;
		*s = destroy(*s);
		cmd->u.subshell_command = read_seq(NULL, s);
		
		// match right shell
		if(next_token_type(*s) == RB)	{
			*s = destroy(*s);	
			return cmd;
		}
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
read_simple_command( queue *s) 
{
	
	command_t cmd = malloc_cmd();
	int n = 0;
	queue words = q_empty();

	while(next_token_type(*s) == WORD)
	{
		words = enqueue(next(*s), words);
		*s = dequeue(*s);
		n++;
	}

	if(n != 0) 
	{
		cmd->type = SIMPLE_COMMAND;
		cmd->u.word = (char**)checked_malloc(sizeof(char*) * n + 1);
		n = 0;
		for(n = 0; isempty(words) == false; n++) 
		{
			char* temp = next(words)->value;
			cmd->u.word[n] = (char*)malloc(sizeof(char*) * strlen(temp) + 1);
			strcpy(cmd->u.word[n], temp);
			words = dequeue(words);
		}

		token_type type = next_token_type(*s);
		if(type == IN)
		{
			char* in = next(*s)->value;
			*s = dequeue(*s);
			cmd->input = in;
			type = next_token_type(*s);
			if(type == OUT)
			{
				char* out = next(*s)->value;
				*s = dequeue(*s);
				cmd->output = out;
			}
		}
		else if(type == OUT)
		{
			char* out = next(*s)->value;
			*s = dequeue(*s);
			cmd->output = out;
			type = next_token_type(*s);
			if(type == IN)
			{
				char* in = next(*s)->value;
				*s = dequeue(*s);
				cmd->input = in;
			}
		}
		cmd->u.word[n] = (char*) checked_malloc(1);
		strcpy(cmd->u.word[n], "\0");
		return cmd;
			
	}	
	else 
	{
		//TODO throw exception
		return NULL;
	}
		
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
read_pipeline(command_t holder, queue* s) 
{
	// beginning of current pipeline 
	if( holder == NULL) 
	{
		// read first command, this will be the left child of node if 
		// number of commands in pipeline is more than one
		command_t cur = read_simple_command(s);
		return read_pipeline(cur, s);
	}
	 
	command_t cmd;
	// end of pipeline
	if(next_token_type(*s) != PIPE)		
		return holder;
	
	// allocate memory for current pipeline command
	*s = dequeue(*s);
	cmd = malloc_cmd();

	cmd->type = PIPE_COMMAND;
	// assign left and right child, left associative
	cmd->u.command[0] = holder;
	cmd->u.command[1] = read_simple_command(s);
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
read_andor(command_t holder, queue *s) 
{
	// beginning of current complete command
	if( holder == NULL) 
	{
		token_type t = next_token_type(*s);
		command_t cur;
		// read subshell command if ( is reached
		if(t == LB)
			cur = read_subshell(s);
		// read pipeline command otherwise
		else
			cur = read_pipeline(NULL, s);
		// build tree
		return read_andor(cur, s);
	}
	else { 
		command_t cmd;
		// get type of next token, will not modified stream
		token_type ttype = next_token_type(*s);
		
		// end of current complete command 
		if(ttype != OR && ttype != AND)		
			return holder;
		// allocate memory for cmd and remove first token {|| &&}
			
		*s = dequeue(*s);
		cmd = malloc_cmd();
		if(ttype == OR)
			cmd->type = OR_COMMAND;
		else
			cmd->type = AND_COMMAND;
		
		// OR COMMAND
		if(next_token_type(*s) == OR)			cmd->type = OR_COMMAND;
		// AND COMMAND
		else if( next_token_type(*s) == AND)		cmd->type = AND_COMMAND;

		cmd->u.command[0] = holder;
		cmd->u.command[1] = read_pipeline(NULL, s);
		// left associtive
		return read_andor(cmd, s);

	}
}



queue
build_token_queue(char* stream)
{
	queue Q = q_empty();
	char* tok = strtok(stream, " ");
	while(tok != NULL) {
		token_type type = token_type_of(tok);

		if(type == UNKNOWN){
			printf("unknow type: [%s]\n", tok);
			error(0,1,"unknown type");
		}
		Q = enqueue(b_pair(token_type_of(tok), tok), Q);
		tok = strtok(NULL, " ");
	}

	return Q;
}

command_stream_t build_token_tree(queue Q) {
	//TODO
	return NULL;
}

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
			 void *get_next_byte_argument)
{
	char c;
	char* buffer = (char* ) checked_malloc(sizeof(char) * STREAM_BUFFER_SIZE);
	size_t n = 0;
	size_t cap = STREAM_BUFFER_SIZE;

	char d;
	while((c = get_next_byte(get_next_byte_argument)) != EOF) {
		
		switch(c) {
			case '|':
			case '&':
				buffer[n++] = ' ';
				buffer[n++] = c;
				if((d = get_next_byte(get_next_byte_argument)) != EOF) {
					if(d == c) {
						buffer[n++] = d;
						buffer[n++] = ' ';
					}
					else
					{
						buffer[n++] = ' ';
						buffer[n++] = d;
					}
				}
				break;
			case '<':
			case '>':
				buffer[n++] = ' ';
				buffer[n++] = c;
				buffer[n++] = ' ';
				break;
			case '\\':
				if((d = get_next_byte(get_next_byte_argument)) != EOF) {
					if(d == '\n') 
						buffer[n++] = '\n';
					else
					{
						buffer[n++] = ' ';
						buffer[n++] = '\\';
						buffer[n++] = ' ';
						buffer[n++] = d;
					}
				}
				break;
			case '\n':
				buffer[n++] = ' ';
				buffer[n++] = ';';
				buffer[n++] = ' ';
				buffer[n++] = '\n';
				buffer[n++] = ' ';
				break;
			case ';':
				buffer[n++] = ' ';
				buffer[n++] = c;
				buffer[n++] = ' ';
				break;
			default:
				buffer[n++] = c;
				break;

			
		}

		if(n + 1 >= cap * 0.8) {
			buffer = (char*) checked_grow_alloc(buffer, &cap);
		}
	}
	buffer[n] = '\0';

	queue Q = build_token_queue(buffer);
	command_stream_t T = build_token_tree(Q);
	return T;
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

queue destroy(queue q)
{
	struct node * temp;
	if(q.head)
	{
		temp = q.head;
		q.head = q.head->next;
		if(temp->value != NULL)
			free(temp->value);
		free(temp);
	}
	if(isempty(q))
		return q_empty();
	return q;
}

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
	if(isempty(q) == false)
	 	return q.head->value;
	return NULL;
}

pair*
b_pair(token_type key, char* value)
{
	pair* p = (pair*) checked_malloc(sizeof(pair));
	p->key = key;
	p->value = value;
	return p;
}

int 
free_pair(pair*);

/********************************************/
