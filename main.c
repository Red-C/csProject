// UCLA CS 111 Lab 1 main program
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
#include "vector.h"
#include "command-internals.h"
//#include "locker.h"
#include "set.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>

//pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct box 
{
	volatile int n_reading;
	bool is_wlocked;
	char* filename;
	int n;

} box;

typedef struct locker 
{
	box** storage;
	int n_locks;
} locker;

typedef box* box_t;


locker create_locker(char** all_files, int n);
bool release_locks(locker* L, int* r_locker_id, int n, int* w_locker_id, int m);
bool get_locks(locker* L, int* r_locker_id, int n, int* w_locker_id, int m);
void print_locker(locker* L);



locker
create_locker(char** all_files, int n)
{
	locker L;
	assert(all_files != NULL);	

	L.storage = (box_t*)malloc(sizeof(box_t) * n + 1);
	L.storage[n] = NULL;
	L.n_locks = n;
	int i = 0;
	for(i = 0; i < n; i++)
	{
		assert(all_files[i] != NULL);
		L.storage[i] = (box_t)malloc(sizeof(box));
		L.storage[i]->filename = (char*)malloc(strlen(all_files[i]));
		strcpy(L.storage[i]->filename, all_files[i]);
		L.storage[i]->n = i;
		L.storage[i]->n_reading = 0;
		L.storage[i]->is_wlocked = false;
	}
	return L;
}

#define set_read_lock(L, i,state) L->storage[i]->is_rlocked = state 
#define set_write_lock(L, i, state) L->storage[i]->is_wlocked = state 
#define read_lock(L, i) set_read_lock(L, i, true)
#define write_lock(L, i) set_write_lock(L, i, true)
#define read_unlock(L, i) set_read_lock(L,i, false) 
#define write_unlock(L, i) set_write_lock(L,i, false)
#define RLOCK(L, i) (L->storage[i]->n_reading != 0)
#define WLOCK(L, i) (L->storage[i]->is_wlocked)

bool 
get_locks(locker* L, int* r_locker_id, int n, int* w_locker_id, int m) {
	
	int i = 0,j = 0;

	// check reading locks
	for(i = 0; i < n; i++) {
		j = r_locker_id[i];
		// if someone is writing
		if(RLOCK(L,j) == false && WLOCK(L, j)) 
		{
			// unlock 
			return false;
		}
	}

	// check writing locks
	for(i = 0; i < m; i++) {
		j = w_locker_id[i];
		// if someone is reading or writing return false
		if(WLOCK(L, j) || RLOCK(L, j)) 
		{
			// unlock
			return false;
		}
	}

	// lock reading lock
	// doesn't allow other people to write
	// increment number of user that is reading
	for(i = 0; i < n; i++) {
		j = r_locker_id[i];
		L->storage[j]->n_reading++;
	}

	// lock writing lock
	// doesn't allow other to write
	// people who try to read have to check write lock
	for(i = 0; i < m ;i++) {
		j = w_locker_id[i];
		write_lock(L, j);
	}

	// unlock
	return true;
}

bool 
release_locks(locker* L, int* r_locker_id, int n, int* w_locker_id, int m) {
	// LOCK
	// pthread_mutex_lock(&mutex_lock);
	int i = 0, j = 0;
	// release read locks
	for(i = 0; i < n; i++) {
	    j = r_locker_id[i];

		// decrement number of people that is reading
		L->storage[j]->n_reading--;
		
		assert(L->storage[j]->n_reading >= 0);
		// if no one is reading, allow other people to write
		if(RLOCK(L, j) == false)
			write_unlock(L, j);
		else 
			printf("reading queue: %d\n", L->storage[j] ->n_reading);

	}
	for(i = 0; i < m; i++) {
		// release write lock
	    j = w_locker_id[i];
		write_unlock(L, j);

	}
	// UNLOCK
	for( i = 0; i < L->n_locks; i++) {
		printf("%s: %d %d\n", L->storage[i]->filename, RLOCK(L, i), WLOCK(L, i));
	}
	//pthread_mutex_unlock(&mutex_lock);
	return true;
}


void print_locker(locker* L) {
	int i = 0;
	for( i = 0; i < L->n_locks; i++) {
		printf("%s: %d %d\n", L->storage[i]->filename, RLOCK(L, i), WLOCK(L, i));
	}

}






/////////////////////////////////////////////////////////////

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
	printf("finished\n");
	// get mutex lock and release locks in locker
	// wait if other thread is doing something with locker
	//release_locks(&LOCKER, cmd_t->in, cmd_t->n_in, cmd_t->out, cmd_t->n_out);
	// release locks
	//TODO check if locks are released	

	// create new process and wait to exit
	/*
	pid_t pid;	
	if((pid = fork()) == -1) 
		error(0,1,"fork error");
	else if(pid == 0) {
		execute_command(cmd_t, 0);
		exit(0);
	}
	else {
		// if pid exited
		while(wait(&cmd_t->status) != pid)
			;
		printf("process exited\n");
		release_locks(&LOCKER, cmd_t->in, cmd_t->n_in, cmd_t->out, cmd_t->n_out);
		// waiting to release locks
		//release_locks(&LOCKER, cmd_t->in, cmd_t->n_in, cmd_t->out, cmd_t->n_out);
		
	}
	
	printf("exiting\n");
	*/
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
		else
			printf("i: %d\n", i);
	}
	for(i = 0; i < m; i++) {
		if(out->fileName[i])
			outSet[i] = indexOf(set, out->fileName[i]);
		else
			printf("i: %d\n", i);
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

  printf("=========================\n");
  printf("dependency:\n");
  while ((command = read_command_stream (command_stream)))
    {
		  if (print_tree)
		{
		  printf ("# %d\n", command_number++);
		  print_command (command);
		}
		  if(time_travel == false)
		  {
			  last_command = command;
			execute_command(command, 0);
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

			int i = 0;
			for(i = 0; i < in.size; i++) {
				printf("in-%s ",  in.fileName[i]);
			}
			for(i = 0; i < out.size; i++) {
				printf("out-%s ", out.fileName[i]);
			}
			printf("\n");
			
			generate_file_indexes(command, &in, &out, &S);
		}
    }
    printf("=========================\n");

	if(time_travel) {
	// create lockers base on number of file required by all commands
	LOCKER = create_locker(S.fileName, S.size);
	
  // create threads that for join purpose
  pthread_t threads[waitingList->size];
  printf("w=%d\n", waitingList->size);
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
		if(get_locks(&LOCKER, next->in, next->n_in, next->out, next->n_out)) {
			printf("get lock\n");
			queue_enqueue(running_queue, next);
		   pthread_create(&(threads[n_thread++]), NULL, execute, next); 
		}
		else {
			queue_enqueue(waitingList, next);
		}
	  }


	  for(i = 0; i < n_thread; i++) {
		int ret = pthread_join(threads[i], NULL);
		printf("ret = %d", ret);
	  }

	  while(queue_isempty(running_queue) == false) {
		command_t next;
		next = (command_t)queue_dequeue(running_queue);
		release_locks(&LOCKER, next->in, next->n_in, next->out, next->n_out);
	  }
  }
}
  return print_tree || !last_command ? 0 : command_status (last_command);
}
