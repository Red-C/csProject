// UCLA CS 111 Lab 1 command reading;

#include "command.h"
#include "command-internals.h"
#include "alloc.h"

#include <error.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*********************************************************************/
#define STREAM_BUFFER_SIZE 100

unsigned int _line = 0;

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
/*********************************************************************/
typedef struct pair {
	token_type key;
	char* value;
} pair;

/*********************************************************************/
struct node{
	 pair* value;
	 struct node * next; 
};
typedef struct queue{
	 struct node * head;
	 struct node * tail;
	 unsigned int size;
} queue;

queue q_empty();
bool isempty(const queue q);
queue enqueue(pair* in, queue q);
queue dequeue(queue q);
queue destroy(queue q);
pair* next(const queue q);
pair* b_pair(token_type key, char* value);

/*********************************************************************/
command_t read_seq(command_t holder, queue *s);
command_t read_subshell( queue *s);
command_t read_simple_command( queue *s);
command_t read_single_command( queue s);
command_t read_pipeline(command_t holder, queue *s);
command_t read_andor(command_t holder, queue *s);
/*********************************************************************/
struct command_stream {
	command_t root;				// root of tree
	int iterator;				// iterator
	command_t* command_queue;	// complete commands array
	size_t size;				// number of element in command_queue
	char* buffer;				// start location of buffer, deallocate purpose
};
/*********************************************************************/
command_stream_t build_token_tree(queue);
queue partition(char* input);
void lineError(const char*);
void load_in_out(queue*, command_t);
/*********************************************************************/


/**
 * next_token_type TODO
 * param: 
 * 		s-queue*: command array that holds all the tokens
 * return:
 * 		token_type: type of token as enum above 
 * descr:
 * 		analyze next token and return the type, this function will
 * 		not modify the command_stream
 */
token_type 
next_token_type(queue* s)
{
	while(isempty(*s) == false && next(*s)->key == BACK_SLASH) {
		_line++;
		*s = destroy(*s);
	}
	if(isempty(*s) == false) {
		return next(*s)->key;
	}
	else
	{
		return END_OF_FILE;
	}
	
}

/**
 * isWORDS
 * 
 * desc: 
 * 		return true if next non-new_line token is a word
 * 		false otherwise
 */
bool 
isWORDS(queue* s) 
{
	while(next(*s) != NULL) {
		
		// ignore all back_slash   it is "\\n"
		if(next(*s)->key == BACK_SLASH) {
			_line++;
			*s = destroy(*s);
		}
		// because the new_lines is already eated at first line
		// of read_simple_command and input and output must be in
		// same line with simple command because new_line cannot
		// appear before or after < >, therefore, the only situation
		// this reaches NEW_LINE is it is a sequence separator
		else if(next(*s)->key == NEW_LINE) {
			_line++;
			next(*s)->key = SEQ;
			return false;
		}
		else
			return (((pair*)next(*s))->key == WORD);
	}
	return false;
}

/**
 * eat_newline
 * desc: eats all the continous new_line token at the top of queue
 */
void eat_newline(queue *s) {
	while(next(*s) != NULL && next(*s)->key == NEW_LINE) {
		// count current line number, for error detector
		_line++;
		*s = destroy(*s);
	}
}

/**
 * isSEQ
 * in:
 * 		s(queue*): command queue buffer
 * return:
 * 		true if next token is a sequence token
 * desc:
 * 		function will detect next token, return true and dequeue
 * 		the seq token if it is a seq token_type
 * 		TODO this should only return bool without remove element
 * 		the dequeue should handle by user
 */
bool
isSEQ(queue *s) 
{
	token_type type;
	// have not reached the end of file
	if(next(*s) != NULL) {
		type = next(*s)->key;
		// if next type is sequence remove token and return true
		// **we do not have to worry about new line because
		// read_simple_command will change the new_line character
		// to SEQ type if it is appended at the end of simple
		// command
		if(type == SEQ) {
			*s = destroy(*s);
			return true;
		}

	}
	return false;

}

/**
 * malloc_cmd
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
{
	//TODO MEMORY ALLOCATED
	//free location: NULL this will be the element of tree and can be transform
	//				into different struct, should be free will command_stream
	//MEMORY LEAKING
	command_t cmd = (command_t)checked_malloc(sizeof(struct command));
	cmd->output = NULL;
	cmd->input = NULL;
	cmd->status = 0;
	return cmd;
}


/**
 * read_seq 
 * param: 
 * 		holder(command_t) recursive holder, helper to 
 * 		s(queue*) command queue buffer
 *
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
	eat_newline(s);
	
	// beginning of current pipeline 
	if( holder == NULL) 
	{
		// read first command, this will be the left child of node if 
		// number of commands in pipeline is more than one
		command_t cur = read_andor(NULL, s);
		// empty command
		if(cur == NULL)
			lineError("empty command");
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
			return holder;
		}
		return holder;
	}
	// allocate memory for current pipeline command
	else  								
	{ 
		eat_newline(s);
		// assign left and right child, left associative
		// right child will be NULL if it reaches the end of command
		// and the command is end with ';'
		// the result will same as the if statement above
		command_t cmd_t =read_andor(NULL, s);
		if(cmd_t == NULL)
			return holder;
		cmd = malloc_cmd();
		cmd->type = SEQUENCE_COMMAND;
		cmd->u.command[0] = holder;
		cmd->u.command[1] = cmd_t; 
		return read_seq(cmd, s);
	}

}


/**
 * read_subshell
 * param: 
 * 		s(queue*) : command queueu buffer pointer
 * return:
 * 		root of current command tree
 *
 * descr:
 * 		same level as pipeline command, this will be called by
 * 		read_andor_command, which is called by read_seq.
 * 		this will call read_seq and build a tree base on the commands
 * 		inside of bracket. if right bracket is not exist, throw exception
 */
command_t 
read_subshell(queue *s) 
{
	// new_line can appear before subshell, eat them
	eat_newline(s);
	// build shell command struct
	command_t cmd = malloc_cmd();
	// this will never happen because this function will be called
	// only if LB is detected, but write this if any stupid mistake
	if(next_token_type(s) == LB)
	{	
		// pop (
		cmd->type = SUBSHELL_COMMAND;
		*s = destroy(*s);
		cmd->u.subshell_command = read_seq(NULL, s);
		
		eat_newline(s);
		// match right shell
		if(next_token_type(s) == RB)	{
			*s = destroy(*s);	
			load_in_out(s, cmd);
			if(next_token_type(s) == NEW_LINE) {
				_line++;
				next(*s)->key = SEQ;
			}
			return cmd;
		}
		else
			// left bracked and right bracket doesn't match
			lineError("bracket not matched");
	}
	else
		// some stupid mistake
		lineError("can not find left bracket");

  // this will never be reached
  return NULL;
}



/**
 * read_pipeline
 * param: 
 * 		s(queue*) : command queue buffer
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
	eat_newline(s);
	// beginning of current pipeline 
	if( holder == NULL) 
	{
		command_t cur = read_simple_command(s);
		// empty command, this happen if multiple new_line character
		// is appended at the end of file
		// exception is already caught in read_simple_command
		
		// TODO we can remove this, because if the code reaches end 
		// of file, there will be nothing after that, read_andor will
		// return holder anyway.
		if(cur == NULL)
			return NULL;
		// left associative
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
	// TODO lets see if this can be happened or not
	if(cmd->u.command[1] == NULL)
		lineError("invalid format, no command on the right of '|'");

	return read_pipeline(cmd, s);

}

/**
 * read_andor
 * param: 
 * 		s(queue*) : command queue buffer
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
	eat_newline(s);
	// beginning of current complete command
	if( holder == NULL) 
	{
		command_t cur;	
		cur = read_pipeline(NULL, s);
		// empty command, this happen if multiple new_line character
		// is appended at the end of file
		// TODO we can remove this, because if the code reaches end 
		// of file, there will be nothing after that, read_andor will
		// return holder anyway.
		if(cur == NULL)
			return NULL;
		// left associative
		return read_andor(cur, s);
	}
	else { 
		
		command_t cmd;

		token_type ttype = next_token_type(s);
		
		// end of current complete command, if the input has wrong format
		// ie: cat a b < a > c c ||
		// this will return cat a b < a > c
		// but Q still has c ||, and exception will be catched in
		// build_token_tree function
		if(ttype != OR && ttype != AND)		
			return holder;

		// allocate memory for cmd and remove first token {|| &&}
		*s = dequeue(*s);
		cmd = malloc_cmd();
		// assign type of command
		if(ttype == OR)
			cmd->type = OR_COMMAND;
		else
			cmd->type = AND_COMMAND;
		
		// have to eat new line here
		// ie: ((a)||
		//      (b))
		// next_token_type will be \n
		// which will not get into the read_subshell command
		eat_newline(s);	

		cmd->u.command[0] = holder;
		// read subshell if ( occur, it will return NULL if expcetion 
		if(next_token_type(s) == LB)
			cmd->u.command[1] = read_subshell(s);
		else
			cmd->u.command[1] = read_pipeline(NULL, s);
		// error if || or && is shown but nothing after that
		// ie: cat a ||;
		if(cmd->u.command[1] == NULL)
			lineError("invalid format, no command at the right of && or ||");
		// left associtive
		return read_andor(cmd, s);

	}
}
/**
 * prefix_traversal_helper
 * in:
 * 		Q(command_t*): array holder that will hold the complete commands
 *		cmd(command_t): current location in tree
 *		cap(size_t): capacity of of Q
 *		i: size holder that holds number of commands readed
 * out:
 *		command_t*: complete commands array
 *	desc:
 *		recursively call each node with prefix order(left center right)
 *		
 */
command_t* prefix_traversal_helper(command_t* Q, command_t cmd, size_t *cap, size_t *i) {
	// TODO BUG: checked_grow_alloc modified the value inside of array for 
	// 			unknown reason
	if(*i >= (*cap)/2)
		Q = (command_t*)checked_grow_alloc(Q, (size_t*)cap);

	// this will happen at top level of tree if there is a sequence token
	// at the end of file
	// cat a b;echo a b;  <- two sequence, second sequence command has NULL
	// 						right child
	if(cmd == NULL)
		return Q;
	// insert and return if not seq command
	if(cmd->type != SEQUENCE_COMMAND) {
		Q[(*i)++] = cmd;
		return Q;
	}
	else
	{
		// prefix traversal recursive step
		Q = prefix_traversal_helper(Q, cmd->u.command[0], cap, i);
		Q = prefix_traversal_helper(Q, cmd->u.command[1], cap, i);
	}
	return Q;
}

/*
 * prefix_traversal
 * in:
 * 		root(command_t): root of entire command tree
 * 		i(size_t*): size holder
 * return:
 * 		an array of root of complete commands that is ready to print
 * desc:
 * 		BUG: the checked_malloc modifies my pointers during checked_grow_alloc
 * 		PARTIAL SOLUTION: I set the size of capacity to 1024, which migh enough to
 * 						holds commands, because the size of a pointer is very small
 * 						it might not waste too much of memory
 * 			 
 */
command_t* prefix_traversal(command_t root, size_t *i)
{
	size_t cap = 1024;
	*i = 0;
	// TODO MEMORY ALLOCATED
	// free location: NULL  this will put into the command_stream
	// 						free the memory with command_stream together
	// MEMORY LEAKING
	command_t* cmd_queue = (command_t*)checked_malloc(sizeof(command_t) * cap);
	return prefix_traversal_helper(cmd_queue, root, &cap, i);

}

/*
 * build_token_tree
 * in:
 * 		Q(queue): the queue of commands that ready to build
 * out:
 * 		command_stream_t: commands array that is in order, and ready to print out
 * 						with iterator
 *
 */
command_stream_t build_token_tree(queue Q) {

	//TODO MEMORY ALLOCATED
	//free location: NULL 
	//				 should deallocate before program stop
	//TODO MEMORY LEAKING
	command_stream_t cmd_stream = (command_stream_t)checked_malloc(sizeof(struct command_stream));

	// build a token tree
	cmd_stream->root = read_seq(NULL, &Q);
	// if the queue has remained tokens, throw error
	// ie:   a|b||a a c d
	// 				[a c d]
	if(isempty(Q) == false)  
		lineError("invalid format, more element in queue");

	// initialize iterator 
	cmd_stream->iterator = 0;
	// prefix traversal to insert the roots of each complete command into cmd_stream
	// cmd_stream->size is pass by ref, it will hold number of command in array
	cmd_stream->command_queue = prefix_traversal(cmd_stream->root, &cmd_stream->size);
	return cmd_stream;
}

/**
 * make_command_stream
 * in:
 * 		get_next_byte: read function pointer, word with get_next_byte_argument
 * 		get_next_byte_argument: file stream
 * out:
 * 		command_stream_t:	build trees and return an array of roots that are
 * 		in prefix order
 *
 */
command_stream_t
make_command_stream (int (*get_next_byte) (void *),
			 void *get_next_byte_argument)
{
	char c;
	// TODO memory allocated, this is used to hold the data read from stream
	// free location make_command_stream 401: free(buffer)
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
	// TODO memory allocated, MEMORY LEAKING
	// free location: NO WHERE, the start pointer is inside of partition function
	// maybe we can saved it in queue, or pass the address of that location by
	// reference through buffer 
	Q = partition(buffer);
	// TODO if pass location by reference, this have to removed
	free(buffer);
	// TODO maybe we can save memory location inside of command_stream_t
	command_stream_t T = build_token_tree(Q);
	return T;
}

/**
 * read_command_stream
 * in:
 * 		s(command_stream_t): a pointer points to command_stream
 * 				command_stream includes iterator and an array
 * out:
 * 		command_t:
 * 				next command_t
 * 				NULL if no more command to pop 
 * descr:
 * 		return next available comment
 */
command_t
read_command_stream (command_stream_t s)
{
	if((size_t)s->iterator < s->size)
		return s->command_queue[s->iterator++];
	return NULL;
}

/********************************************/


/** 
 * q_empty
 * desc: return an empty initialized queue
 */
queue 
q_empty()
{
	 queue q;
	 q.head = q.tail = NULL;
	 q.size = 0;
	 return q;
}

/**
 * is empty
 * desc: check if empty
 */
bool isempty(const queue q)
{
	 if(q.head == NULL)
	   return true;
	 else
	   return false;
}
/**
 * enqueue
 * in:
 * 		in(pair*): value to enqueue
 * 		q(queue): queue that used to enqueue
 * out:
 * 		queue: queue after add a element
 *
 * 	desc:	
 * 		can be done with pass by reference
 * 		the value that is passed in should already allocated and initialized	
 * 		
 */
queue enqueue(pair* in, queue q)
{
 // allocate memory for node
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

/**
 * destroy
 * in:
 * 		q(queue): the queue that want to pop a element
 * out:
 * 		queue: copy of modified queue
 *
 * descr:
 * 		can be done by pass by ref. this was written because I
 * 		planning to free the memory of char pointer
 * 		however it doesn't work very well
 * 		TODO this can work with dequeue after modified , pass 
 * 		by ref the Q into dequeue, and dequeue will free the node
 * 		and return value of node without free it, and this function
 * 		handles the free of value
 */
queue destroy(queue q)
{
	return dequeue(q);
}

/**
 * dequeue
 * in:
 * 		q(queue): the queue that want to pop a eleement
 * out:
 * 		queue: modified queue
 * descr:
 * 		this function will free the memory of first node
 * 		and memory of first node->value
 * 		TODO this can be implements with pass by reference
 * 		and return first nodes value without free it
 * 		user will handle the deallocation of memory of value
 */
queue dequeue(queue q)
{
	struct node * temp;
	// detect empty situation
	if(q.head)
	{
		temp = q.head;
		q.head = q.head->next;
		// free memory of node->value
		if(temp->value != NULL)
			free(temp->value);
		// free node
		free(temp);
		q.size--;
	}
	if(isempty(q))
		return q_empty();
	return q;
}

/**
 * next 
 * in:
 * 		q: queue that we looking next for
 * out:
 * 		pair: next pair
 *
 * desc:
 * 		return next
 *
 */
pair* next(const queue q)
{
	if(isempty(q) == false)
	 	return q.head->value;
	return NULL;
}


/**
 * b_pair
 * in:
 * 		key(token_type): key of pair
 * 		value(char*):	value of pair
 *
 * out: 
 * 		pair with key and value 
 * 	descr:
 * 	WARNING: the value is a pointer, and it is pass by reference
 * 			which means this will not allocate memory to copy the
 * 			string that passed in. 
 * 			
 * 			****this is function is used for queue, deqeue function will
 * 			free the pair struct memory, but value will not free in dequeue step
 * 			PLEASE free string manually.
 *
 * 			In this project, all the string is in a same block of memory,
 * 			just free the block at the end
 *
 */
pair*
b_pair(token_type key, char* value)
{
	pair* p = (pair*) checked_malloc(sizeof(pair));
	p->key = key;
	p->value = value;
	return p;
}




/**
 *	lineError
 *	in:
 *		numLine(int): line number that has error
 *  descr:
 *  	TODO add a parameter, const char* as error message
 *  		this can be done wil error() function
 */
void lineError(const char* msg)
{
  fprintf(stderr, "%d:%s\n",_line, msg);
  exit(1);
}

/**
 * isRegular
 * in:
 * 		input(char): just a character to check
 * out:
 * 		int: true if character is a valid word token
 */
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


/**
 * partition
 * in:
 * 		input(char*): buffer that holds all character reads from stream
 * out:
 * 		queue: token queue that split by special character
 * desc: 
 * 		split the input by special character and put into the queue
 * 		TODO thinking to passing in a char** because this function
 * 			allocates new memory for queue, and previous memory is wasted
 */
queue partition(char* input)
{
  char* current = input;
  int Status_comment = 0, Status_backslash = 0;
  int len = 0;
  int newLine = 1;
  _line = 1;

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
	  if(!newLine)
	    strcat(tmpStr, "#");
	  else
	    Status_comment = 1;
	  break;

	case '\\':
	  if(!Status_comment)
	    Status_backslash = 1;
	  if(*(current + 1) != '\n')
	    {
	      lineError("\\ isn't followed by \\n");
	    }
	  break;

	case '\n':
	  _line++;
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
	    lineError("invalid token (&)");
	  break;

	default:
	  if(isRegular(*current))
	    strncat(tmpStr, current, 1);
	  else
	    lineError("invalid token");
        }
	  newLine = (*current == '\n')? 1 :
		  			(*current == ' ' || *current == '\t')? newLine : 0;
      current++;
    }

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

      pch = strtok(NULL, " ");
    }

  // reset error line 
  _line = 1;
  return tokenQueue;
}


void 
load_in_out(queue *s, command_t cmd) {

	// if < or > token has reached, assign input or output value to struct
	token_type type = next_token_type(s);
	if(type == IN || type == OUT)
	{
		// pop < or >
		*s = destroy(*s);
		// invalid format, ie: a <;
		if(next_token_type(s) != WORD)
			lineError("need file name after IN or OUT token");
		// get input or output file name
		char* data = next(*s)->value;
		// indicator
		char** ptr = (type == IN)? &cmd->input: &cmd->output; 
		*ptr = data;
		// pop message
		*s = dequeue(*s);
		token_type next_type = next_token_type(s);;
		// only read another if type of prev redir is different as 
		// current redir token
		if((type == IN && next_type == OUT) 
				|| (type == OUT && next_type == IN))
		{
			// pop < or >
			*s = destroy(*s);
			// invalid format: a < b >
			if(next_token_type(s) != WORD)
				lineError("need file name after IN or OUT token");
			// get file name
			data = next(*s)->value;
			// indicator
			ptr = (next_type == IN)? &cmd->input: &cmd->output;
			*ptr = data;
			// pop message
			*s = dequeue(*s);
		}
	}
}


/**
 * read_simple_command
 * param: 
 * 		s(queue*) : command buffer queue
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
	// because new line can only appear at beginning of simple command or
	// sub shell command, therefore, eats all the newline at this point is 
	// enough
	eat_newline(s);
	if (isempty(*s) || next(*s)->key == RB) {
		return NULL;
	}
	// because the new line character can not appear before || &&
	// we do not have to eat them
	token_type t = next_token_type(s);
	// read subshell command if ( is reached
	if(t == LB) 
		return read_subshell(s);
	
	command_t cmd = malloc_cmd();
	int n = 0;
	queue words = q_empty();

	// eats seequence of words before special character
	while(isWORDS(s))
	{
		words = enqueue(b_pair(next(*s)->key, next(*s)->value), words);
		*s = dequeue(*s);
		n++;
	}

	// pop words and insert into struct
	// n == 0 means empty command, two cases, 
	// complete empty command: ;;
	// wrong format:  cat a || || ls b
	// both of them are invalid, throw exception
	if(n != 0) 
	{
		// TODO allocated Memory
		// free location: 
		// char pointer array, this will reconstruct the struct,
		// which will not reallocate the memory again
		cmd->type = SIMPLE_COMMAND;
		cmd->u.word = (char**)checked_malloc(sizeof(char*) * n + 1);
		n = 0;
		// copy all words
		for(n = 0; isempty(words) == false; n++) 
		{
			char* temp = next(words)->value;
			cmd->u.word[n] = temp;
			words = dequeue(words);
		}

		load_in_out(s, cmd);

		// because new_line can only appear before of first words of message
		// and can only appear after || && |, therefore if the \n is appended
		// at the end of simple command, it is the end of seq 
		token_type type = next_token_type(s);
		if(type == NEW_LINE) {
			_line++;
			next(*s)->key = SEQ;
		}
			
		// print function is detect NULL pointer as end of **word
		cmd->u.word[n] = NULL;
		return cmd;
			
	}	
	else 
	{
		// n == 0 empty simple command
		lineError("empty simple command");
		return NULL;
	}
}
