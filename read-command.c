// UCLA CS 111 Lab 1 command reading;

#include "command.h"
#include "command-internals.h"
#include "alloc.h"

#include <error.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

unsigned int _line = 0;

/*********************************************************************/



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
next_token_type(queue* s)
{
	if(isempty(*s) == false)
		return next(*s)->key;
	else
	{
		return END_OF_FILE;
	}
	
	/*
	while(next(*s) != NULL) {
		
		if(next(*s)->key == BACK_SLASH || next(*s)->key == NEW_LINE) {
			_line++;
			*s = destroy(*s);
		}
		else
			return ((pair*)next(*s))->key;
	}
	return END_OF_LINE;
	*/
}

bool 
isWORDS(queue* s) 
{
	while(next(*s) != NULL) {
		
		if(next(*s)->key == BACK_SLASH) {
			_line++;
			*s = destroy(*s);
		}
		else if(next(*s)->key == NEW_LINE) {
			next(*s)->key = SEQ;
			return false;
		}
		else
			return (((pair*)next(*s))->key == WORD);
	}
	return false;
}
void eat_newline(queue *s) {
	while(next(*s) != NULL && next(*s)->key == NEW_LINE) {
		_line++;
		*s = destroy(*s);
	}
}

bool
isSEQ(queue *s) 
{
	token_type type;
	if(next(*s) != NULL) {
		type = ((pair*)next(*s))->key;
		if(type == SEQ) {
			*s = destroy(*s);
			return true;
		}

	}
	return false;

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
		// empty command
		if(cur == NULL)
			lineError(_line);
		return read_seq(cur, s);
	}
	 
	command_t cmd;
	// end of pipeline
	if(isSEQ(s) == false){
		if(holder->type != SEQUENCE_COMMAND) {
			// print uses
			// it will return pipeline/andor/simple command if
			// this is removed, however, read command requires
			// sequence type as root of tree
			cmd = malloc_cmd();
			cmd->type = SEQUENCE_COMMAND;
			cmd->u.command[0] = holder;
			return cmd;
		}
		return holder;
	}
	// allocate memory for current pipeline command
	else  								
	{ 
		cmd = malloc_cmd();
	}

	// assign left and right child, left associative
	// read_andor will return NULL if it reaches the end of command
	// and the command is end with ';'
	// the result will same as the if statement above
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
	if(next_token_type(s) == LB)
	{	
		// build subshell
		cmd->type = SUBSHELL_COMMAND;
		*s = destroy(*s);
		cmd->u.subshell_command = read_seq(NULL, s);
		
		// match right shell
		if(next_token_type(s) == RB)	{
			*s = destroy(*s);	
			return cmd;
		}
		else
			lineError(_line);
	}
	else
		lineError(_line);

  return NULL;
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
	eat_newline(s);
	if (isempty(*s)) {
		return NULL;
	}
	
	command_t cmd = malloc_cmd();
	int n = 0;
	queue words = q_empty();

	while(isWORDS(s))
	{
		words = enqueue(b_pair(next(*s)->key, next(*s)->value), words);
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

		token_type type = next_token_type(s);
		if(type == IN)
		{
			*s = destroy(*s);
			if(next_token_type(s) != WORD)
				lineError(_line);
			char* in = next(*s)->value;
			cmd->input = in;
			*s = dequeue(*s);
			type = next_token_type(s);;
			if(type == OUT)
			{
				*s = destroy(*s);
				if(next_token_type(s) != WORD)
					lineError(_line);
				char* out = next(*s)->value;
				cmd->output = out;
				*s = dequeue(*s);
			}
		}
		else if(type == OUT)
		{
			*s = destroy(*s);
			if(next_token_type(s) != WORD)
				lineError(_line);
			char* out = next(*s)->value;
			cmd->output = out;
			*s = dequeue(*s);
			type = next_token_type(s);
			if(type == IN)
			{
				*s = destroy(*s);
				if(next_token_type(s) != WORD)
					lineError(_line);
				char* in = next(*s)->value;
				cmd->input = in;
				*s = dequeue(*s);
			}
		}

		type = next_token_type(s);
		if(type == NEW_LINE)
			next(*s)->key = SEQ;
			
		cmd->u.word[n] = NULL;
		/*
		cmd->u.word[n] = (char*) checked_malloc(1);
		strcpy(cmd->u.word[n], "\0");
		*/
		return cmd;
			
	}	
	else 
	{
		lineError(_line);
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
		if(cur == NULL)
			return NULL;
		return read_pipeline(cur, s);
	}
	 
	command_t cmd;
	// end of pipeline
	if(next_token_type(s) != PIPE)		
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
		token_type t = next_token_type(s);
		command_t cur;
		// read subshell command if ( is reached
		if(t == LB) 
			cur = read_subshell(s);
		
		// read pipeline command otherwise
		else
		{
			cur = read_pipeline(NULL, s);
			if(cur == NULL)
				return NULL;
		}
		// build tree
		return read_andor(cur, s);
	}
	else { 
		command_t cmd;
		// get type of next token, will not modified stream
		token_type ttype = next_token_type(s);
		
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
		if(next_token_type(s) == OR)			cmd->type = OR_COMMAND;
		// AND COMMAND
		else if( next_token_type(s) == AND)		cmd->type = AND_COMMAND;

		cmd->u.command[0] = holder;
		if(next_token_type(s) == LB)
			cmd->u.command[1] = read_subshell(s);
		cmd->u.command[1] = read_pipeline(NULL, s);
		// error if || or && is shown but nothing after that
		if(cmd->u.command[1] == NULL)
			lineError(_line);
		// left associtive
		return read_andor(cmd, s);

	}
}

command_t* prefix_traversal_helper(command_t* Q, command_t cmd, size_t *cap, size_t *i) {
	if(*i + 4 >= *cap)
		Q = (command_t*)checked_grow_alloc(Q, (size_t*)cap);

	if(cmd == NULL)
		return Q;
	if(cmd->type != SEQUENCE_COMMAND) {
		Q[(*i)++] = cmd;
		return Q;
	}
	else
	{
		Q = prefix_traversal_helper(Q, cmd->u.command[0], cap, i);
		Q = prefix_traversal_helper(Q, cmd->u.command[1], cap, i);
	}
	return Q;
}

command_t* prefix_traversal(command_t root, size_t *i)
{
	size_t cap = 1024;
	*i = 0;
	command_t* cmd_queue = (command_t*)checked_malloc(sizeof(command_t) * cap);
	return prefix_traversal_helper(cmd_queue, root, &cap, i);

}

command_stream_t build_token_tree(queue Q) {

	command_stream_t cmd_stream = (command_stream_t)checked_malloc(sizeof(struct command_stream));

	cmd_stream->root = read_seq(NULL, &Q);
	if(isempty(Q) == false)  
		lineError(_line);

	cmd_stream->iterator = 0;
	cmd_stream->command_queue = prefix_traversal(cmd_stream->root, &cmd_stream->size);
	return cmd_stream;
}

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
			 void *get_next_byte_argument)
{
	char c;
	char* buffer = (char* ) checked_malloc(sizeof(char) * STREAM_BUFFER_SIZE);
	size_t n = 0;
	size_t cap = STREAM_BUFFER_SIZE;

	while((c = get_next_byte(get_next_byte_argument)) != EOF)
	{
		buffer[n++] = c;
		if(n + 1 >= cap * 0.8) {
			buffer = (char*) checked_grow_alloc(buffer, &cap);
		}
	}
	buffer[n] = '\0';

	queue Q;
	Q = partition(buffer);
	free(buffer);
	command_stream_t T = build_token_tree(Q);
	return T;
}

command_t
read_command_stream (command_stream_t s)
{
	if((size_t)s->iterator < s->size)
		return s->command_queue[s->iterator++];
	return NULL;
}

/********************************************/



queue 
q_empty()
{
	 queue q;
	 q.head = q.tail = NULL;
	 q.size = 0;
	 return q;
}


bool isempty(queue q)
{
	 if(q.head == NULL)
	   return true;
	 else
	   return false;
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
 q.size++;
 return q;
};

queue destroy(queue q)
{
	return dequeue(q);
	/*
	struct node * temp;
	if(q.head)
	{
		temp = q.head;
		q.head = q.head->next;
		if(temp->value != NULL) {
			if(temp->value->value != NULL) {
				free(temp->value->value);
			}
			free(temp->value);
		}
		free(temp);
	}
	if(isempty(q))
		return q_empty();
	return q;
	*/
}

queue dequeue(queue q)
{
	struct node * temp;
	if(q.head)
	{
		temp = q.head;
		q.head = q.head->next;
		if(temp->value != NULL)
			free(temp->value);
		free(temp);
		q.size--;
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





void lineError(int numLine)
{
  fprintf(stderr, "%d:\n", numLine);
  exit(1);
}

int isRegular(char input)
//detect if the charactor is a regular charactor
{
  if (input >= 48 && input <= 57)
    return 1;
  if (input >= 65 && input <= 90)
    return 1;
  if (input >= 97 && input <= 122)
    return 1;
  switch(input)
    {
    case '!':
    case '%':
    case '+':
    case ',':
    case '-':
    case '.':
    case '/':
    case ':':
    case '@':
    case '^':
    case '_':
	  return 1;
    default:
      return 0;
    }
}



queue partition(char* input)
{
  char* current = input;
  int Status_comment = 0, Status_backslash = 0;
  int len = 0, numLine = 1;

  char* tmpStr = (char*)checked_malloc(3*strlen(input)*sizeof(char));
  while(*current != '\0')
    {
      if(Status_comment && *current != '\n')
	{
	  current++;
	  continue;
	}

      switch(*current)
        {
	case '#':
	  if(isRegular(*(current + 1)))
	    strcat(tmpStr, "#");
	  else
	    Status_comment = 1;
	  break;

	case '\\':
	  if(!Status_comment)
	    Status_backslash = 1;
	  if(*(current + 1) != '\n')
	    {
	      lineError(numLine);
	    }
	  break;

	case '\n':
	  numLine++;
	  if(Status_backslash)
	    {
	      Status_backslash = 0;
	      strcat(tmpStr, " \\ ");
	      break;
	    }
	  else if(Status_comment)
	    {
	      Status_comment = 0;
	      strcat(tmpStr, " \n ");
	      break;
	    }
	  else
	    {
	      strcat(tmpStr, " \n ");
	    }
	  break;

	case '\t':
	case ' ':
	  strcat(tmpStr, " ");
	  break;

	case ';':
	  strcat(tmpStr, " ; ");
	  break;

	case '>':
	  strcat(tmpStr, " > ");
	  len = len + 3;
	  break;

	case '<':
	  strcat(tmpStr, " < ");
	  len = len + 3;
	  break;

	case '(':
	  strcat(tmpStr, " ( ");
	  len = len + 3;
	  break;

	case ')':
	  strcat(tmpStr, " ) ");
	  break;

	case '|':
	  if(*(current + 1) == '|')
	    {
	      strcat(tmpStr, " || ");
	      current++;
	    }
	  else
	    {
	      strcat(tmpStr, " | ");
	    }
	  break;

	case '&':
	  if(*(current + 1) == '&')
	    {
	      strcat(tmpStr, " && ");
	      len = len + 4;
	      current++;
	    }
	  else
	    lineError(numLine);
	  break;

	default:
	  if(isRegular(*current))
	    strncat(tmpStr, current, 1);
	  else
	    lineError(numLine);
        }

      current++;
    }

  /* testing code */
  /*
  printf("%s\n", input);
  printf("%s\n", tmpStr);
  */

  //partition and put them in queue
  queue tokenQueue = q_empty();
  char* pch;
  token_type key;
  pch = strtok(tmpStr, " ");
  while(pch != NULL)
    {
      //determine token type
      if(*pch == '<')
		key = IN;
      else if(*pch == '>')
		key = OUT;
      else if(*pch == '&')
		key = AND;
      else if(*pch == '|') 
	  {
		if(strlen(pch) == 2)
		  key = OR;
		else
		  key = PIPE;
	  }
      else if(*pch == '\n')
		key = NEW_LINE;
	  else if(*pch == ';')
		key = SEQ;
      else if(*pch == '\\')
		key = BACK_SLASH;
      else if(*pch == '(')
		key = LB;
      else if(*pch == ')')
		key = RB;
      else
		key = WORD;

      //construct the pair
      pair* inputPair = b_pair(key, pch);

      //enqueue the pair
      tokenQueue = enqueue(inputPair, tokenQueue);

      //testing
      // printf("%s\n", pch);

      pch = strtok(NULL, " ");
    }

  //free(tmpStr);

  return tokenQueue;
}

