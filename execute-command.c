// UCLA CS 111 Lab 1 command execution
//  team group
//  Zhen Feng UID: 904499798
//  Yi Wang UID: 504426290  
#include "command.h"
#include "command-internals.h"
#include "alloc.h"
#include <stdio.h>
#include <error.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

int
command_status (command_t c)
{
  return c->status;
}

struct list_word
{
  char* word;
  struct list_word* next;
};

void
exec_simple( command_t comm, int init) // just put init as 0
{
  int re;
  int filefrom;
  int filedesc;
  int stat;
  if (init != 1)
    {fprintf(stderr," exec_simple put at the wrong place");
    exit(-1);}
  pid_t p = fork();
  if(p==0)//child
    {
      if(comm->input) //input
	{
	  filefrom = open(comm->input,O_RDONLY);
	  if(filefrom < 0)
	    {
	      fprintf(stderr,"problem with input");
	      exit(-1);
	    }
	  re=dup2(filefrom,STDIN_FILENO);
	  if(re<0)
	    {
	      fprintf(stderr,"can not read from file");
	      exit(-1);
	    }
	  close(filefrom);
	}
      if(comm->output) // has output to a file
	{
	  filedesc = open(comm->output,O_CREAT | O_TRUNC | O_WRONLY,0777);
	  if(filedesc < 0)
	    {
	      fprintf(stderr,"problem with output");
	      exit(-1);
	    }
	  re=dup2(filedesc,STDOUT_FILENO);
	  if(re<0)
	    {
	      fprintf(stderr,"can not write to file");
	      exit(-1);
	    }
	  close(filedesc);
	}
      /* char * ha;
      int i=0;
      for(i=0;ha!=NULL;i++)
      {
       ha=comm->u.word[i];
	  printf("omg is here %s \n",ha);
	  }*/
      execvp(comm->u.word[0],comm->u.word);
    }
  else if(p >0)
    {
      waitpid(p,&stat,0);
      comm->status=stat;
    }
  else
    {
      fprintf(stderr,"can not create process");
      exit(-1);
    }  
}

void
exec_or(command_t comm, int init)
{
  execute_command (comm->u.command[0], 0);
  comm->status=comm->u.command[0]->status;
  init =  comm->status ;
  if(init) //run when first command fail;
    {
      execute_command(comm->u.command[1],0);
      comm->status=comm->u.command[1]->status;
    }
  /* else
     {
       printf("the first command is true");
       }*/
  
}

void
exec_and(command_t comm , int init)
{
  execute_command (comm->u.command[0], 0);
  comm->status=comm->u.command[0]->status;
  init = comm->status;
  if(!init)//run when first command true;
    {
      execute_command(comm->u.command[1],0);
      comm->status=comm->u.command[1]->status;
    }
  /*else
    {
      printf("and first command is false");
      }*/
}

void
exec_seq(command_t comm , int init)
{
  if (init != 5)
    {fprintf(stderr," exec_seq  put at the wrong place");
      exit(-1);}
  execute_command(comm->u.command[0],0);
  execute_command(comm->u.command[1],0);
  comm->status=comm->u.command[1]->status;
}


void
exec_sub(command_t comm, int init)
{
  int re;
  int filefrom;
  int filedesc;
  int stat;
  int in=0;
  int out=0;
  int saved_stdout;
  int saved_stdin;
  if (init != 4)
    {fprintf(stderr," exec_simple put at the wrong place");
      exit(-1);}
      if(comm->input) //input
	{
	  in=1;
	  saved_stdin=dup(0);
	  // printf("%s\n",comm->input); //haha
	  filefrom = open(comm->input,O_RDONLY);
	  if(filefrom < 0)
	    {
	      fprintf(stderr,"problem with input");
	      exit(-1);
	    }
	  re=dup2(filefrom,STDIN_FILENO);
	  if(re<0)
	    {
	      fprintf(stderr,"can not read from file");
	      exit(-1);
	    }
	  close(filefrom);
	}
      if(comm->output) // has output to a file
	{
	  //printf("%s",comm->output);
	  out=1;
	  saved_stdout=dup(1);
	  filedesc = open(comm->output,O_CREAT |O_TRUNC |O_WRONLY,0777);
	  if(filedesc < 0)
	    {
	      fprintf(stderr,"problem with output");
	      exit(-1);
	    }
	  re=dup2(filedesc,STDOUT_FILENO);
	  if(re<0)
	    {
	      fprintf(stderr,"can not write to file");
	      exit(-1);
	    }
	  close(filedesc);
	}
  execute_command (comm->u.subshell_command, 0);
  if(in)
    {
      dup2(saved_stdin,0);
      close(saved_stdin);
      in =0;
    }
  if(out)
    {
      dup2(saved_stdout,1);
      close(saved_stdout);
      out=0;
    }
  comm->status=comm->u.subshell_command->status;
}

void
exec_pipe(command_t comm, int init)
{
  int re;
  int filefrom;
  int filedesc;
  int stat;
  int PipeEnd[2];
  pipe(PipeEnd);
  pid_t p = fork();
  if (init != 6)
    {fprintf(stderr," exec_simple put at the wrong place");
      exit(-1);}
  if(p==0) //child run first command as input;
    {
      stat=dup2(PipeEnd[1],STDOUT_FILENO);
      if(stat<0)
	{
	  fprintf(stderr,"something wrong with pipe");
	  exit(-1);
	}
      else
	{
	  close(PipeEnd[0]);
	  close(PipeEnd[1]);
	}
	execute_command(comm->u.command[0],0);
	comm->status=comm->u.command[0]->status;
	exit(0);
    }
  else if(p>0)
    {
      waitpid(p,&stat,0);
      comm->status=stat;
      stat=dup2(PipeEnd[0],STDIN_FILENO);
      if(stat<0)
	{
	  fprintf(stderr,"something wrong with pipe");
	  exit(-1);
	}
      else
	{
	  close(PipeEnd[1]);
	  close(PipeEnd[0]);
	}
      execute_command(comm->u.command[1],0);
      comm->status=comm->u.command[1]->status;
    }
  else
    {
      fprintf(stderr,"fork has error");
      exit(-1);
    }
      comm->status=comm->u.command[1]->status;
}

/********************************************************************/
int
string_compare(char * a, char * b)
{
  return strcmp(a,b);
}

/*********************************************************************************/

void
execute_command (command_t c, int time_travel)
{
  /* FIXME: Replace this with your implementation.  You may need to                         
     add auxiliary functions and otherwise modify the source code.                          
     You can also use external functions defined in the GNU C Library.  */
  enum command_type Scrtype=c->type;
  if (Scrtype == SIMPLE_COMMAND )
    {
      exec_simple(c, 1);
    }
  else if (Scrtype == AND_COMMAND)
    {
      exec_and(c, 2);
    }
  else if (Scrtype == OR_COMMAND )
    {
      exec_or(c,3);
    }
  else if (Scrtype == SUBSHELL_COMMAND)
    {
      exec_sub(c, 4);
    }
  else if (Scrtype == SEQUENCE_COMMAND)
    {
      exec_seq(c, 5);
    }
  else if (Scrtype == PIPE_COMMAND )
    {
      exec_pipe(c,6);
    }
  else
    {
      fprintf(stderr,"no command like that");
      exit(-1);
    }
}
