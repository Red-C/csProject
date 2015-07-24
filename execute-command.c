#include "command.h"
#include "command-internals.h"
#include "alloc.h"
#include <unistd.h>
#include <error.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

/*******************************************************/
void execute_simple_command ( command_t c);
void execute_pipe_command ( command_t c);
void execute_andor_command( command_t c);
void execute_simple_command ( command_t c);
void execute_subshell(command_t c);
void execute_sequence(command_t c);
int brancher(command_t c);
/*******************************************************/


void execute_command(command_t c, int time_travel) {
	if(time_travel == 0)
		brancher(c);
	else
		brancher(c);
}
int
command_status (command_t c)
{
  return c->status;
}


/**
 * brancher
 * in: 
 * 		c: current command that is going to execute
 *
 * descr:
 * 		brancher will jump to the function base on type of c
 */
int
brancher(command_t c)
{
	switch(c->type) {
	case SIMPLE_COMMAND:
		execute_simple_command(c);
		break;
	case SUBSHELL_COMMAND:
		execute_subshell(c);
		break;
	case PIPE_COMMAND:
		execute_pipe_command(c);
		break;
	case AND_COMMAND:
	case OR_COMMAND:
		execute_andor_command(c);
		break;
	case SEQUENCE_COMMAND:
		execute_sequence(c);
		break;
	default:
		error(0,1,"unknow type");
	}
	return c->status;
}

/**
 * execute_simple_command
 * in:
 * 		c (command_t) : the simple command that is going to execute
 * out: 
 * 		exit status of c
 *
 * descr: call by brancher, doesnt have to test if command is simple
 * 		it is already checked in brancher
 *
 * Error:
 *		fork unsuccessful
 *		output/input file open unsuccessful
 *		command execute unsuccessful by execvp
 */
void
execute_simple_command(command_t c)
{
  pid_t p = fork();
  if(p==0)
  {
		// assign input stream if input file exist 
		if(c->input != NULL)
		{
			int fd = open(c->input, O_RDONLY);
			if(fd < 0)
				error(0,1, "invalid input file");
			dup2(fd, 0);
			close(fd);
				
		}
		// assign output stream if output file exist
		if(c->output != NULL)
		{
			// assign mode '666' to the output file, create if not exist
			int fd = open(c->output, O_WRONLY|O_CREAT|O_TRUNC
					,S_IRUSR|S_IWUSR|S_IWGRP|S_IRGRP|S_IROTH|S_IWOTH);
			if(fd < 0) {
				error(0,1, "invalid output file");
				exit(-1);
			}
			dup2(fd, 1);
			close(fd);
		}
      execvp(c->u.word[0],c->u.word);
    }
  else if(p >0)
      waitpid(p,&c->status,0);
  else
    {
		error(0,1, "fork error");
      	exit(-1);
    }  
}

/**
 * execute_andor_command
 * in:
 * 		c(command_t):	andor command to be execute
 * descr: 
 * 		recursive call by calling brancher
 */
void
execute_andor_command(command_t c)
{
  // execute_left
  c->status = brancher(c->u.command[0]);
  
  // execute_right
  if(c->type == OR_COMMAND && c->status) // a || b
    {
      c->status = brancher(c->u.command[1]);
    }
  else if(c->type == AND_COMMAND && !c->status) // a && b
    {
      c->status = brancher(c->u.command[1]);
    }
  
}

void
execute_sequence(command_t c)
{
  c->status = brancher(c->u.command[0]);
  c->status = brancher(c->u.command[1]);
}

void
execute_subshell(command_t c)
{
	int input_fd;
	int output_fd;
	// assign input stream if input file exist 
	if(c->input != NULL)
	{
		// save input file descriptor, for close purpose
		input_fd = dup(0);
		int fd = open(c->input, O_RDONLY);
		if(fd < 0)
			error(0,1, "invalid input file");
		dup2(fd, 0);
		close(fd);
			
	}
	// assign output stream if output file exist
	if(c->output != NULL)
	{
		// save output file descriptor, for close purpose
		output_fd = dup(1);
		
		// assign mode '666' to the output file, create if not exist
		int fd = open(c->output, O_WRONLY|O_CREAT|O_TRUNC
				,S_IRUSR|S_IWUSR|S_IWGRP|S_IRGRP|S_IROTH|S_IWOTH);
		if(fd < 0) {
			error(0,1, "invalid output file");
			exit(-1);
		}
		dup2(fd, 1);
		close(fd);
	}
	c->status = brancher(c->u.subshell_command);
  if(c->input != NULL)
    {
	  // assign stdin back to main process 
      dup2(input_fd,STDIN_FILENO);
      close(input_fd);
    }
  if(c->output != NULL)
    {
	  // assign stdout back to main process 
      dup2(output_fd,STDOUT_FILENO);
      close(output_fd);
    }
}


/**
 * execute_pipe_helper
 *
 * in: 
 * 		c(command_t): pipe command that is going to execute
 *
 *	
 * descr:
 * 		this will be called by brancher
 *
 */
void
execute_pipe_command(command_t c)
{
	
	// create pipe between parent and child
	int fds[2];
	pipe(fds);
	if(pipe(fds) == -1)
		error(0,1,"pipe error");

	pid_t pid;
	if((pid = fork()) == -1)
		  error(0,1,"fork error");

	// child process
	else if(pid == 0) {
		close(fds[0]);
		// redirect stdout
		dup2(fds[1], 1);
		close(fds[1]);
		brancher(c->u.command[0]);
		// c->status=c->u.command[0]->status;
		// this can be deleted
		exit(0);

	}
	else {
		// waiting for child process exit
		waitpid(pid, &c->status, 0);
		dup2(fds[0], 0);
		// close pipe
		close(fds[1]);
		close(fds[0]);
		

		// execute right 
      c->status = brancher(c->u.command[1]);
	}
}

