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

int
command_status (command_t c)
{
  return c->status;
}

int execute_simple_command ( command_t c);

void
execute_command (command_t c, int time_travel)
{
	/*
	pid_t pid;
	if((pid = fork()) == 0)
	{
		char *cmd[3];
		cmd[0] = (char*) malloc(sizeof(char) * 5);
		cmd[1] = (char*) malloc(sizeof(char) * 3);
		strcpy(cmd[0], "less");
		strcpy(cmd[1], "f1");
		cmd[2] = NULL;
		execvp(cmd[0],cmd);
	}
	else {
		int status = 0;
		while(wait(&status) != pid)
			;
	}
	*/
	int exit_status = execute_andor_command(c);
	printf("exit status: %d\n", exit_status);
	if(c->status != 0)
		error(0,1,"command execute unsuccessful");
}

int execute_simple_command ( command_t c)
{
	if(c->input != NULL)
	{
		int fd = open(c->input, O_RDONLY);
		if(fd < 0)
			error(0,1, "invalid input file");
		dup2(fd, 0);
		close(fd);
			
	}
	if(c->output != NULL)
	{
		int fd = open(c->output, O_WRONLY|O_CREAT
				,S_IRUSR|S_IWUSR|S_IWGRP|S_IRGRP|S_IROTH|S_IWOTH);
		if(fd < 0)
			error(0,1, "invalid output file");
		dup2(fd, 1);
		close(fd);
	}

	execvp(c->u.word[0], c->u.word);
	exit(1);

}


int execute_pipe_helper(command_t c)
{
	int fds[2];
	pipe(fds);
	if(pipe(fds) == -1)
		error(0,1,"pipe error");

	if(c->type != PIPE_COMMAND) {
		// end of recursion
		if(c->type != SIMPLE_COMMAND)
			error(0,1, "non simple command has called execute simple");
		
		pid_t pid;
		if((pid = fork()) == -1)
			error(0,1,"fork error");

		else if(pid == 0) {
			close(fds[0]);
			dup2(fds[1], 1);
			close(fds[1]);
			execute_simple_command(c);
			exit(c->status);

		}
		else {
			int status = 0;
			while(wait(&status) != pid)
				;
			c->status = status;
			close(fds[1]);
			if(c->status != 0) {
				close(fds[0]);
				return -1;
			}
			return fds[0];
		}
		
	}
	
	else {
		command_t left = c->u.command[0];
		command_t right = c->u.command[1];

		// recursive call left command
		int fd_in = execute_pipe_helper(left);
		if(left->status != 0)
		{
			c->status = left->status;
			return -1;
		}
		else {
			pid_t pid;	
			if((pid = fork()) == 0)
			{	// child process	
				// redirect input fd to fd_in
				dup2(fd_in, 0);
				close(fd_in);
				close(fds[0]);
				// output stream	
				dup2(fds[1], 1);
				// redirect output stream
				execute_simple_command(right);
				// exit with exit status of right command
				// return non zero value if execute unsuccessful
				// zero if successful
				exit(right->status);
				
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
				return fds[0];
			}

		}
	}
	// return fds2
}
int execute_pipe_command(command_t c)
{
	if(c->type != PIPE_COMMAND) {

		if(c->type != SIMPLE_COMMAND)
			error(0,1, "non simple command has called execute simple");
		
		pid_t pid;
		if((pid = fork()) == -1)
			error(0,1,"fork error");

		else if(pid == 0) {
			execvp(c->u.word[0], c->u.word);
			exit(1);
			
		}
		else {
			while(wait(&c->status) != pid)
				;
		}
	}
	else {
		int fd = execute_pipe_helper(c);
		if(c->status == 0) {
			int n = 0;
			char buffer[1024];
			while((n = read(fd, buffer, 1024)) != 0)
				write(STDOUT_FILENO, buffer, n);
			write(STDOUT_FILENO, "\n", 1);
				
		}
	}
	return c->status;
}
int execute_andor_command(command_t c) {
	if(c->type != AND_COMMAND && c->type != OR_COMMAND) {
		execute_pipe_command(c);
		return c->status;
	}
	else {
		command_t left = c->u.command[0];
		command_t right = c->u.command[1];
		if(c->type == AND_COMMAND) 
		{
			return !(((c->status = execute_andor_command(left)) == 0) 
				&& ((c->status = execute_andor_command(right)) == 0));
		}
		else
			return !(((c->status = execute_andor_command(left)) == 0) 
				|| ((c->status = execute_andor_command(right)) == 0));
		
	}
}

/*
int main(int argc, char** argv) 
{
	command_t cmd = (command_t) malloc(sizeof(struct command) );
	cmd->type = SIMPLE_COMMAND;
	cmd->u.word = (char**) malloc(sizeof(char*) * 4);
	cmd->u.word[3] = NULL;
	cmd->u.word[0] = (char*) malloc(sizeof(char) * 3);
	cmd->u.word[1] = (char*) malloc(sizeof(char) * 4);
	cmd->u.word[2] = (char*) malloc(sizeof(char) * 2);

	strcpy(cmd->u.word[0], "ls");
	strcpy(cmd->u.word[1], "-al");
	strcpy(cmd->u.word[2], ".");
	cmd->status = -1;
	cmd->input = NULL;
	



	//cmd->output = (char*) malloc(sizeof(char) * 3);
	//strcpy(cmd->output, "f1");

	int fd = execute_pipe_command(cmd);
	if(cmd->status == 0)
	{
		printf("status = %d", cmd->status);
		char buffer[1024];
		int n = 0;
		while((n = read(fd, buffer, sizeof(buffer))) != 0) 
		{
			printf("n=%d\n", n );
			write(STDOUT_FILENO, buffer, n);
		}
		close(fd);

	}
	
	printf("\n");
	printf("exit status:%d\n", cmd->status);
	return 0;
}
*/
