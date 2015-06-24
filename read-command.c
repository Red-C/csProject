// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include "alloc.h"

#include <error.h>
#include <stdio.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */


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
	char* buffer;
	size_t size;
	size_t cap;
};

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
			 void *get_next_byte_argument)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
	// allocate struct command_stream
	command_stream_t p = (command_stream_t)checked_malloc(sizeof(command_stream));
	// initialize buffer size, will realloc if needed
	p->cap = STREAM_BUFFER_SIZE;
	p->buffer = (char*) checked_malloc(sizeof(char)*p->cap);

	while(char c = get_next_byte(get_next_byte_argument))
	{
			// if buffer is full, increase with uprate
			if(p->size >= p->cap)
			{ 
					// p->cap *= UPRATE;    // there is a grow_alloc function, use that one instead
					p->buffer = (char*)checked_grow_alloc(p->buffer, &p->cap);
			}
			
			// read next byte and increase size
			p->buffer[p->size++] = c;
	}
	printf("%s", p->buffer);
  //error (1, 0, "command reading not yet implemented");
  //return 0;
	return p;
}




command_t
read_command_stream (command_stream_t s)
{
	
  /* FIXME: Replace this with your implementation too.  */
  //error (1, 0, "command reading not yet implemented");
  //return 0;
}
