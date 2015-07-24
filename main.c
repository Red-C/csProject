// UCLA CS 111 Lab 1 main program
// Hongbo Li & Hongfei Li
#define _GNU_SOURCE
#include <errno.h>
#include <error.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include "command.h"
#include "queue.h"
#include "command-internals.h"
#include "locker.h"
#include "vector.h"
#include "set.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>

#define DEBUG_MODE



static char const *program_name;
static char const *script_name;
locker LOCKER;
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

void*
execute(void* cmd) {

	//command_t cmd_t = (command_t)cmd;

	// execute command
	execute_command((command_t)cmd, 1);

	pthread_exit(0);
	//return NULL;
}

//build_I_O_set takes a command and puts all input to the set "in"
//		and all outputs to the set "out".
//
//Important: in, out must be empty.
void 
build_I_O_set(command_t c, set *in, set *out)
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
	else
	{
		build_I_O_set(c->u.command[0], in, out);
		build_I_O_set(c->u.command[1], in, out);
	}
	return;
}

void
generate_file_indexes(command_t cmd,set* in, set *out, set *set) 
{
	int n = in->size;
	int m = out->size;
	int* inSet = (n == 0? NULL: (int*)malloc(sizeof(int) * n));
	int* outSet = (m == 0? NULL: (int*)malloc(sizeof(int) * m));
	
	int i = 0;
	for(i = 0; i < n; i++) {
		if(in->fileName[i])
			inSet[i] = indexOf(set, in->fileName[i]);
	}
	for(i = 0; i < m; i++) {
		if(out->fileName[i])
			outSet[i] = indexOf(set, out->fileName[i]);
	}
	cmd->in = inSet;
	cmd->out = outSet;
	cmd->n_in = n;
	cmd->n_out = m;

}

int
main (int argc, char **argv)
{
  int opt;
  int command_number = 1;
  int print_tree = 0;
  int time_travel = 0;
  program_name = argv[0];
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
 
  set S = createFileSet();
  queue* waitingList = queue_init();

  while ((command = read_command_stream (command_stream)))
    {
		  if (print_tree)
		{
		  printf ("# %d\n", command_number++);
		  print_command (command);
		}
		  else if(!time_travel)
		  	{
			  last_command = command;
			  execute_command(command, time_travel);
		  }
		  else
		 {
			// insert all command into waiting list
			queue_enqueue(waitingList, command);		
			// union a set that includes all files
			set in = createFileSet();
			set out = createFileSet();
			build_I_O_set(command, &in, &out);
			unionSet(&S, &in);
			unionSet(&S, &out);

#ifdef BUG_MODE
			int i = 0;
			for(i = 0; i < in.size; i++) {
				printf("in-%s ",  in.fileName[i]);
			}
			for(i = 0; i < out.size; i++) {
				printf("out-%s ", out.fileName[i]);
			}
			printf("\n");
		
#endif
			
			generate_file_indexes(command, &in, &out, &S);
		}
    }

	if(time_travel) {
		// create lockers base on number of file required by all commands
		LOCKER = create_locker(S.fileName, S.size);
		
	  // create threads that for join purpose
	  pthread_t threads[waitingList->size];

	  int i = 0;
		//int n_thread = 0;
	  // stop when all commands are executed
	  while(queue_isempty(waitingList) == false)
	  {
		queue* running_queue = queue_init();
		int n_thread = 0;
		  // doesn't allow other thread to exit
		  // condition 1: n process are finished and release locks, m process are finished and not release locks yet
		  // 		run process that have dependency with n process, this loop will finally release mutex locks
		  // 		and wait for other thread to run, and other thread will get the lock and release the locks in locker
		  // condition 2: all thread are finished but havn't released locks, this should not happen because wait 
		  // 		function should block current thread until one thread is exit
		  // condition 3: all thread are finished and release locks, this should run all command that have dependency
		  // 		on those commands, and get new locks
		  // condition 4: all commands are finished, queue should finished all commands and queue is empty, exit looping
		 
		  // check all command in queue, if it is ready to run, create new threads and execute it, otherwise
		  // put in queue again
		  int queue_size = queue_sizeof(waitingList);
		  for(i = 0; i < queue_size; i++){	
			// get next command
			command_t next;
			next = (command_t)queue_dequeue(waitingList);
			// this will not asking for locks again because  
			if(get_locks(next, &LOCKER, next->in, next->n_in, 
								next->out, next->n_out,
								(int (*)(void*, void*))command_t_cmp)) {
				queue_enqueue(running_queue, next);
			   pthread_create(&(threads[n_thread++]), NULL, execute, next); 
			}
			else {
				queue_enqueue(waitingList, next);
			}
		  }


		  for(i = 0; i < n_thread; i++) {
			pthread_join(threads[i], NULL);
		  }

		  while(queue_isempty(running_queue) == false) {
			command_t next;
			next = (command_t)queue_dequeue(running_queue);
			release_locks(next, &LOCKER, next->in, next->n_in, 
								next->out, next->n_out,
								(int (*)(void*, void*))command_t_cmp);
		  }
  }
}
  return print_tree || !last_command ? 0 : command_status (last_command);
}
