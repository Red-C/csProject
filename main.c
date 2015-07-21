// UCLA CS 111 Lab 1 main program

#include <errno.h>
#include <error.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include "command.h"
#include "queue.h"
#include "vector.h"
#include "command-internals.h"
#include <sys/types.h>


static char const *program_name;
static char const *script_name;

static void
usage (void)
{
  error (1, 0, "usage: %s [-pt] SCRIPT-FILE", program_name);
}

static int
get_next_byte (void *stream)
{
  return getc (stream);
}
pthread_mutex_t running_mutex = PTHREAD_MUTEX_INITIALIZER;
volatile bool is_any_thread_finished = false;
vector* running_list;

void*
execute(void* cmd) {
	execute_command((command_t)cmd, 0);

	pid_t tid = gettid();
	printf("pricess %d exit and tring to get lock\n", (int)tid);
	pthread_mutex_lock(&running_mutex);
	printf("pricess got lock\n");
	find_and_delete(running_list, cmd);
	is_any_thread_finished = true;
	printf("process exited\n");
	pthread_mutex_unlock(&running_mutex);
	pthread_exit(0);
	return NULL;
}

/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/

//build_I_O_set takes a command and puts all input to the set "in"
//		and all outputs to the set "out".
//
//Important: in, out must be empty.
void 
build_I_O_set(command_t c, struct fileSet *in, struct fileSet *out)
{
	//if the input command is a simple command,
	//add the input and output if exist, and return.
	if(c == NULL)
		return;

	if(c->input != NULL)
		addFile(in, c->input);
	if(c->output != NULL)
		addFile(out, c->output);


	if(c->type == SIMPLE_COMMAND) 
	{
		char** word = c->u.word;					
		int i = 1;
		while(word[i] != NULL) 
		{
			if(word[i][0] != '-') {
				addFile(in, word[i]);
			}
			i++;
		}
	}
	//if the input commands is a subshell,
	//recursively call this function to every sub commands.
	else if(c->type == SUBSHELL_COMMAND)
	{
		build_I_O_set(c->u.subshell_command, in, out);
	}

	//else, the input is either and-or, pipe, or sequence,
	//so recursively call this function to the two sub commands.
	else if(c->type != SIMPLE_COMMAND)
	{
		build_I_O_set(c->u.command[0], in, out);
		build_I_O_set(c->u.command[1], in, out);
	}
	return;
}

bool is_dependency(command_t cmd, struct fileSet* in, struct fileSet* out) {
	if(is_intersect(&cmd->in, out) == true 
			|| is_intersect(&cmd->out, out) == true 
			|| is_intersect(&cmd->out, in) == true)
		return true;
	return false;

}

int
main (int argc, char **argv)
{
  int opt;
  int command_number = 1;
  int print_tree = 0;
  int time_travel = 0;
  program_name = argv[0];
  running_list = vector_init();
  for (;;)
    switch (getopt (argc, argv, "pt"))
      {
      case 'p': print_tree = 1; break;
      case 't': time_travel = 1; break;
      default: usage (); break;
      case -1: goto options_exhausted;
      }
 options_exhausted:;

  // There must be exactly one file argument.
  if (optind != argc - 1)
    usage ();

  script_name = argv[optind];
  FILE *script_stream = fopen (script_name, "r");
  if (! script_stream)
    error (1, errno, "%s: cannot open", script_name);
  command_stream_t command_stream =
    make_command_stream (get_next_byte, script_stream);

  command_t last_command = NULL;
  command_t command;
  
  queue *waiting_queue = queue_init();
  while ((command = read_command_stream (command_stream)))
    {
		  if (print_tree)
		{
		  printf ("# %d\n", command_number++);
		  print_command (command);
		}
		  else
		{
			build_I_O_set(command, &command->in, &command->out);
			queue_enqueue(waiting_queue, command);		
		}
    }

  struct fileSet inputSet = createFileSet();
  struct fileSet outputSet = createFileSet();
  int size = queue_sizeof(waiting_queue);
  pthread_t threads[size];
  int i = 0;
  int n_thread = 0;
  while(queue_isempty(waiting_queue) == false) {

	  // start with empty set
	  cleanSet(&inputSet);
	  cleanSet(&outputSet);
	  inputSet = createFileSet();
	  outputSet = createFileSet();
	  

	  // get all files that is using by running command
	  // we do not want pthread modified running_list during the for loop
	  // therefore, add lock
	  pthread_mutex_lock(&running_mutex);
	  for( i = 0; i < vector_sizeof(running_list); i++) {
		      
			command_t next_running = (command_t)vector_get(running_list, i);
		    if(next_running->status != -1) {
				vector_delete(running_list, i);
				i--;
			}
			else {
			  unionSet(&inputSet, &next_running->in);
			  unionSet(&outputSet, &next_running->out);
			}
	  }
	  pthread_mutex_unlock(&running_mutex);

	  // check dependency of all command in waiting list
	  // this will run everytime there is command has finished.
  	  size = queue_sizeof(waiting_queue);
	  for(i = 0; i < size; i++) {
		  command_t next = (command_t)queue_dequeue(waiting_queue);

		  // if current command has dependency with previous commands
		  // update inputSet and outputSet
		  // put into the queue for next round
		  // because the condition of for loop is not checking whether
		  // queue is empty, we can just insert into the queue
		  if(is_dependency(next, &inputSet, &outputSet) == true)
		  {
			  // waiting for next round
			  queue_enqueue(waiting_queue, next);
		  }
		  else 
		  {
			// because we do not want other thread remove itself
			// when I try to insert new item, add lock
			pthread_mutex_lock(&running_mutex);
			vector_insert(running_list, next);
			pthread_mutex_unlock(&running_mutex);
			pthread_create(&threads[n_thread++], NULL, &execute, next);
		  }
		  unionSet(&inputSet, &next->in);
		  unionSet(&outputSet, &next->out);
	   }

	   // thread will modify this flag after it is done and remove its
	   // command from running array
//	   while(is_any_thread_finished == false && queue_isempty(waiting_queue) == false)
//	   {
//		   int ret = pthread_yield();
//		   assert(ret == 0);
//	   }
	   //
  		printf("some thread finished\n");
		pthread_mutex_lock(&running_mutex);
//	   is_any_thread_finished = false;
		pthread_mutex_unlock(&running_mutex);
   }

  // waiting for all threads are finished
  for(i = 0; i < n_thread; i++ )
	pthread_join(threads[i], NULL);


  return print_tree || !last_command ? 0 : command_status (last_command);
}
