// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"

#include <error.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/wait.h>

/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

int execute_simple_command ( command_t c);
int execute_pipe_command ( command_t c);
int execute_andor_command( command_t c);
int execute_simple_command ( command_t c);
int execute_subshell(command_t c);

/**
 * command_status
 */
int
command_status (command_t c)
{
  return c->status;
}

/**
 * execute_command
 * in:
 * 		c (command_t) : a complete command that is going to execute
 * 		time_travel (int):
 *
 * descr: execute a complete command
 */
void
execute_command (command_t c, int time_travel)
{
	int exit_status = time_travel;
	exit_status = execute_andor_command(c);
}

/**
 * execute_simple_command
 * in:
 * 		c (command_t) : the simple command that is going to execute
 * out: 
 * 		int: output file stream descriptor
 *
 * descr: this function is called by execute_pipe_command, 
 * 		it execute a simple command.
 *
 * Error:
 * 		the command passed in is not a simple command
 *		fork unsuccessful
 *		output/input file open unsuccessful
 *		command execute unsuccessful by execvp
 */
int
execute_simple_command ( command_t c)
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
		if(fd < 0)
			error(0,1, "invalid output file");
		dup2(fd, 1);
		close(fd);
	}

	if(c->type == SUBSHELL_COMMAND)
	{
		// subshell has same priority as simple command
		execute_subshell(c);
		// don't know why, when execute_subshell exit with 256
		// the execute_pipe_command function catches 0
		exit(c->status != 0);
	}
	else
	{
		// execute command, this will replace current block of code, 
		// and return status if it is exit
		execvp(c->u.word[0], c->u.word);
		// this should not be reached if command execute successful
		// because the process should be replace by the command
		// if this is reached, it means the command can not be found
		// in bash folder, ie. history
		exit(1);
	}

}


/**
 * execute_pipe_helper
 *
 * in: 
 * 		c(command_t): pipe command that is going to execute
 *
 * out: 
 * 		int: output stream file descriptor
 *	
 * descr:
 * 		this will be called by execute_pipe_command
 * 		it returns the output stream file descriptor, however,
 * 		it will return -1 if there is any error with execution.
 * 		please check c->status or -1 return value to catch the error 
 * 		state.
 *
 */
int execute_pipe_helper(command_t c, int i)
{
	
	// create pipe between parent and child
	int fds[2];
	pipe(fds);
	if(pipe(fds) == -1)
		error(0,1,"pipe error");

	if(i == 0)
		dup2(1,fds[1]);
	// end of recursion, the most left child of tree
	if(c->type != PIPE_COMMAND) {
		// end of recursion
		if(c->type != SIMPLE_COMMAND && c->type != SUBSHELL_COMMAND)
			  error(0,1, "non simple command  or subshell command has called execute simple");

		pid_t pid;
		if((pid = fork()) == -1)
			  error(0,1,"fork error");

		// child process
		else if(pid == 0) {
			close(fds[0]);
			// redirect stdout
			dup2(fds[1], 1);
			close(fds[1]);
			execute_simple_command(c);
			// this can be deleted
			exit(c->status != 0);

		}
		else {
			// waiting for child process exit
			while(wait(&c->status) != pid)
				;
			close(fds[1]);
			if(c->status != 0) {
				   close(fds[0]);
				   return -1;
			}
			// return output stream 
			return fds[0];
  		}
	}
	else {
		command_t left = c->u.command[0];
		command_t right = c->u.command[1];

		// recursive call left child command, redirect input stream to the
		// output of previous command
		int fd_in = execute_pipe_helper(left,1);
		// check return status, stop recursion if error occur
		pid_t pid;	
		if((pid = fork()) == 0)
		{	// child process	
			// redirect input fd to fd_i
			dup2(fd_in, 0);
			close(fd_in);
			close(fds[0]);
			// output stream	
			dup2(fds[1], 1);
			close(fds[1]);
			// redirect output stream
			execute_simple_command(right);
			// exit with exit status of right command
			// return non zero value if execute unsuccessful
			// zero if successful
			exit(right->status != 0);
			
		}
		else
		{	// parent process
			// wait for child process return
			// receive exit status and save it into c->status
			// the upper level of recursive will check the status if zero or not
			while(wait(&c->status) != pid)
				;
			// close fd_in
			close(fd_in);
			close(fds[1]);
			if(c->status != 0)
			{
				close(fds[0]);
				return -1;
			}
			// return output stream fd
			return fds[0];
		}
	}
	// return fds2
	return -1;
}

/**
 * execute_pipe_command
 * in:
 * 		c (command_t): pipe command to execute
 * out:
 * 		exit status of pipe
 *
 * 	descr: execute a pipe command, return status to execute andor function
 *
 */
int execute_pipe_command(command_t c)
{
	// save output stream of last command
	int fd = execute_pipe_helper(c,0);
	if(c->status == 0) {
		int n = 0;
		char buffer[1024];
		// print output of last command in stdout
		while((n = read(fd, buffer, 1024)) != 0)
			write(STDOUT_FILENO, buffer, n);
			
	}
	// return status for andor uses
	return c->status;
}

/**
 * execute_andor_command
 * in:
 * 		c(command_t):	andor command to be execute
 * out: 
 * 		exit status
 * descr: 
 * 		recursive call this function with left associative
 */
int execute_andor_command(command_t c) {
	// end of recursive
	if(c->type != AND_COMMAND && c->type != OR_COMMAND) {
		execute_pipe_command(c);
		// status of command
		return c->status;
	}
	else {
		command_t left = c->u.command[0];
		command_t right = c->u.command[1];
		if(c->type == AND_COMMAND) 
		{
			// !(left && right)
			return !(((c->status = execute_andor_command(left)) == 0) 
				&& ((c->status = execute_andor_command(right)) == 0));
		}
		else
			// !(left || right)
			return !(((c->status = execute_andor_command(left)) == 0) 
				|| ((c->status = execute_andor_command(right)) == 0));
		
	}
}

/**
 * execute_seq_command
 * descr: subshell uses this only
 */
int execute_subshell_helper(command_t c) {
	if(c->type != SEQUENCE_COMMAND){
		c->status = execute_andor_command(c);
	}
	else {
		execute_subshell_helper(c->u.command[0]);
		c->status = execute_andor_command(c->u.command[1]);

	}
	return c->status;
}
/**
 * execute_subshell
 * descr: execute commands within subshell in sequence
 *
 */
int execute_subshell(command_t c) {
	if(c->type != SUBSHELL_COMMAND)
		error(0,1,"non subshell command has called execute subshell");

	command_t cmd = c->u.subshell_command;
	c->status = execute_subshell_helper(cmd);
	return c->status;
}

