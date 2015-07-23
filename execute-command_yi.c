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

void addtwolist(struct list_word**a,struct list_word **b)
{
  struct list_word* cur=*a;
  if(cur!=NULL)
    {
      // printf("inside list word %s \n",cur->word);
      while(cur->next!=NULL)
	{
	  //printf("%s\n",cur->word);
	  cur=cur->next;
	}
      //printf("%s \n",*b->word);
      if(*b!=NULL)
	{
	  cur->next=*b;
	}
    }
  else
    {
      //printf("haha\n");
      *a=*b;
    }
}
struct list_word* inputlist(command_t c)
{
  struct list_word *head=checked_malloc(sizeof(struct list_word));
  head->next=checked_malloc(sizeof(struct list_word));
  head->next=NULL;
  struct list_word *myhead=head;
  switch(c->type)
    {
    case AND_COMMAND:
    case OR_COMMAND:
    case PIPE_COMMAND:
    case SEQUENCE_COMMAND:
      {
	//printf("adding \n");
      struct list_word *a =inputlist(c->u.command[0]);
      struct list_word *b =inputlist(c->u.command[1]);
      //myhead->next=a;
      //printf("%s \n",a->word);
      //printf("%s \n",b->word);
      //printf("int put \n");
	addtwolist(&a,&b);
	myhead->next=a;
	/*while(a!=NULL)
	  {
	    printf("this inside while %s \n",a->word);
	    a=a->next;
	    }*/
      break;
      }
      //case SEQUENCE_COMMAND:
      //printf("there is something wrong\n");
      //break;
    case SUBSHELL_COMMAND:
      if(c->input)
	{
	  struct list_word *node=checked_malloc(sizeof(struct list_word));
	  node->word=c->input;
	  head->next=node;
	  head=head->next;
	  // head=checked_malloc(sizeof(struct list_word));
	  //printf("haha\n");
	  head->next=inputlist(c->u.subshell_command);
	  /*while(head!=NULL)
	    {
	      printf("%s",head->word);
	      head=head->next;
	      }*/
	}
      else
	{
	  head->next=inputlist(c->u.subshell_command);
	  // printf("head ->next %s\n",head->next->word);
	}
      break;
    case SIMPLE_COMMAND:
      if(c->input)
	{
	  struct list_word *node=checked_malloc(sizeof(struct list_word));
	  node->word=c->input;
	  head->next=node;
	  head=head->next;
	  head->next=checked_malloc(sizeof(struct list_word));
	  head->next=NULL;
	}
      char **w=c->u.word;
      while(*++w)
	{
	  struct list_word *node=checked_malloc(sizeof(struct list_word));
	  node->word=*w;
	  //printf("%s\n",node->word);
	  head->next=node;
	  head=head->next;
	  head->next=checked_malloc(sizeof(struct list_word));
	  head->next=NULL;
	}
      break;
    default:
      {
	printf("somthing wrong \n");
      }
    }
  return myhead->next;
}
struct list_word* outputlist(command_t c)
{
  struct list_word *head=checked_malloc(sizeof(struct list_word));
  head->next=checked_malloc((sizeof(struct list_word)));
  head->next=NULL;
  struct list_word *myhead=head;
  switch(c->type)
    {
    case AND_COMMAND:
    case OR_COMMAND:
    case PIPE_COMMAND:
    case SEQUENCE_COMMAND:
      {
      struct list_word *a =outputlist(c->u.command[0]);
      struct list_word *b =outputlist(c->u.command[1]);
      //myhead->next=a;
      //printf("output\n");
      addtwolist(&a,&b);
      myhead->next=a;
      break;
      }
      //case SEQUENCE_COMMAND:
      //printf("there is something wrong\n");
      //break;
    case SUBSHELL_COMMAND:
      if(c->output)
	{
	  struct list_word *node=checked_malloc(sizeof(struct list_word));
	  node->word=c->output;
	  head->next=node;
	  head=head->next;
	  head->next=outputlist(c->u.subshell_command);
	  printf("%s \n",c->output);
	}
      else
	{
	  head->next=outputlist(c->u.subshell_command);
	}
      break;
    case SIMPLE_COMMAND:
      if(c->output)
	{
	  struct list_word *node=checked_malloc(sizeof(struct list_word));
	  node->word=c->output;
	  head->next=node;
	  head=head->next;
	  head->next=checked_malloc(sizeof(struct list_word));
	  head->next=NULL;
	}
      break;
    }
  return myhead->next;
  }
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
int
command_compare(struct list_word *first ,struct list_word * second)// 0 means have same word,  1 means otherwise
{
  struct list_word * firstcurr = first;
  struct list_word * secondcurr = second;
  int check;
  int firstsize;
  int secondsize;
  int size;
  if ( first == NULL)
    {
      if ( second == NULL )
	{
	  return 1;
	}
      return 1;
    }
  else
    {
      if ( second == NULL )
	{
	  return 1;
	}
    }
  while ( firstcurr != NULL )
    {
      while (secondcurr != NULL)
	{
	  check = string_compare(firstcurr->word,secondcurr->word);
	  if (check == 0 ) { return 0;}
	  secondcurr= secondcurr->next;
	}
      firstcurr= firstcurr->next;
    }
  return 1;
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
command_stream_t rebuild(command_stream_t s)
{
  command_t com;
  command_stream_t iterator=s;
  command_stream_t head=checked_malloc(sizeof(command_stream_t));
  command_stream_t myhead=head;
  while(iterator!=NULL&&iterator->comm!=NULL)
    {
      com=iterator->comm;
      //print_command(com);
      if(com->type!=SEQUENCE_COMMAND)
	{
	  command_stream_t node=checked_malloc(sizeof(command_stream_t));
	  node->comm=com;
	  head->next=node;
	  head=head->next;
	  //head=checked_malloc(sizeof(command_stream_t));
	}
      else
	{
	  //printf("yes\n");
	  command_t comleft=com->u.command[0];
	  command_t comright=com->u.command[1];
	  struct stack mystack;
	  mystack.item=0;
	  push(&mystack,comright);
	  push(&mystack,comleft);
	  while(!empty(&mystack))
	    {
	      //printf("size of stack %d \n",sizeofstack(&mystack));
	      command_t curr=top(&mystack);
	      
	      pop(&mystack);
	      //print_command(curr);
	      if(curr->type==SEQUENCE_COMMAND)
		{
		  comleft=curr->u.command[0];
		  comright=curr->u.command[1];
		  push(&mystack,comright);
		  push(&mystack,comleft);
		}
	      else
		{
		  // printf("aaa\n");
		  command_stream_t node=checked_malloc(sizeof(command_stream_t));
		  node->comm=curr;
		  head->next=node;
		  head=head->next;
		}
	    }
	}
      // printf("time \n");
      iterator=iterator->next;
    }
  return myhead->next;
}
int Dependent(command_t a, command_t b) //return 1 if a b can't run together.
{  
  struct list_word *ain=inputlist(a);
  struct list_word *aout=outputlist(a);
  struct list_word *bin=inputlist(b);
  struct list_word *bout=outputlist(b);
  if(!command_compare(ain,bout)) return 1;
  if(!command_compare(aout,bin)) return 1;
  if(!command_compare(aout,bout))return 1;
  return 0;
}
void * ThreadFunction(void* arg)
{
  command_t c=(command_t)arg;
  execute_command (c, 0);
  pthread_exit(0);
}
void
timetravel_mode(command_stream_t s)
{
  command_stream_t start=rebuild(s);
  command_stream_t it=start;
  command_stream_t startlist;
  command_stream_t iterator;
  int numofcommand=0;
  struct stack myqueue;
  int independent=11;// no depend 
  myqueue.item=0;
  // printf("haha\n");
  while(it!=NULL&&it->comm!=NULL)
    {
      it->run=0;
      //push(&myqueue,it->comm);
      numofcommand++;
      it->index=numofcommand;
      //print_command(it->comm);
      /*struct list_word* in=inputlist(it->comm);
      struct list_word* out=outputlist(it->comm);
       print_command(it->comm);
      while(in!=NULL)
	{
	  printf("%s ",in->word);
	  in=in->next;
	}
      printf("\n");
      while(out!=NULL)
	{
	  printf("%s ",out->word);
	  out=out->next;
	}
	printf("\n");*/
      it=it->next;
    }
  //printf("%d\n",numofcommand); /* comment */
  int n=1;
  while(numofcommand>0)
    {
      it=start;
      independent=1;
      printf("round %d \n",n++);
      while(it!=NULL&&it->comm!=NULL) //push the first un run command into ready queue.
	{
	  if(it->run==0)
	    {
	      it->run=1;
	      startlist=it;
	      iterator=startlist;
	      push(&myqueue,it->comm);
	      //print_command(it->comm);
	      //printf("start index %d \n",it->index);
	      it=it->next;
	      start=it;
	      break;
	    }
	  it=it->next;
	}
      while(it!=NULL&&it->comm!=NULL)
	{
	  iterator=startlist;
	  // printf("start index is %d \n",iterator->index);
	  //printf("second index is %d \n",it->index);
	  //print_command(it->comm);
	  //print_command(iterator->comm);
	  //printf("%d \n",numofcommand);
	  while(iterator->index<it->index)
	    {
	      if(numofcommand==1)
	      {
	       break;
	      }
	      if(iterator->run==0) //comapre with unrun;
		{
		  //printf("compare %d %d \n",iterator->index,it->index);
		  if(Dependent(iterator->comm,it->comm))
		    {
		      // printf("%d %d depend\n",iterator->index,it->index);
		      independent=0;
		      break;
		    }
		  //printf("haha\n");
		}
	      //printf("haha\n");
	      int sizeofqueue=sizeofstack(&myqueue);
	      //printf("%d\n", sizeofqueue);
	      int i=0;
	      for(i=0;i<sizeofqueue;i++) //compare with gonna run
		{
		  if(Dependent(element(&myqueue,i),it->comm))
		    {
		      printf("this two command is depend.\n");
		      print_command(element(&myqueue,i));
		      print_command(it->comm);
		      printf("-------------------------\n");
		      independent=0;
		      break;
		    }
		}
	      //printf("hahaha\n");
	      iterator=iterator->next;
	    }
	  // printf("hahaha\n");
	  if(independent && it->run!=1)
	    {
	      it->run=1;
	      //      printf("%d ",it->index);
	      push(&myqueue,it->comm);
	    }
	  it=it->next;
	}      
      startlist->run=1;
      //printf("this number run together \n");
      numofcommand-=sizeofstack(&myqueue);
      int size=sizeofstack(&myqueue);
      pthread_t *threadID;
      long nthreads=size;
      long t;
      threadID=(pthread_t*)malloc(sizeof(pthread_t)*nthreads);
      for(t=0;t<nthreads;t++)
	{
	  print_command(element(&myqueue,t));
	  int rs=pthread_create(&threadID[t],0,ThreadFunction,(void *)element(&myqueue,t));
	  if(rs)
	    {
	      fprintf(stderr,"error creating thread\n");
	      exit(-1);
	    }
	}
      for(t=0;t<nthreads;t++)
	{
	  void *retVal;
	  int rs=pthread_join(threadID[t],&retVal);
	  if(rs)
	    {
	      fprintf(stderr,"Error joining thread \n");
	      exit(-1);
	    }
	}
      free(threadID);
      clear(&myqueue);
    }
  /*command_t a;
  command_t b;
  a=it->comm;
  it=it->next;
  b=it->comm;
  if(Dependent(a,b))
    {
      printf("a depend b\n");
    }
  else
    { 
      printf("a b could run together\n");
      }*/
  
}

