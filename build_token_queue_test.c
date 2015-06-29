#include "alloc.h"
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <error.h>
#include <getopt.h>
#include "command.h"


#define build_token_queue partition
void test_queue(const char*);
void test_eatspace(const char*);
void test_isword(const char*);
void test_eatword(const char*);
void test_eatspecial(char* argv);
void test_comment(char* argv);
void test_make_command_stream(char* argv);
void test_simple_command(char* argv);
void test_pipeline_command(char* argv);
void test_andor_command(char* argv);
void test_seq_command(char* argv);
void test_print_queue(char* argv);
void test_command_stream_t(char* argv);

int main(int argc, char** argv) {
	printf("test program start\n");
	printf("%d\n", '\n');
	
	int size = 100;
	char* str;
	if(argc == 2){
		str = (char*)checked_malloc(sizeof(char) * 100);
		char k;
		int n = 0;
		
		while((k = getc(stdin)) != EOF){
			str[n] = k;
			n++;
			if( n+1 >= size) {
				size *= 1.5;
				str = (char*)checked_realloc(str, size);
			}

		}
		str[n] = '\0';
		printf("read from stdin: %s", str);
	}
	else if(argc == 3) {
		str = (char*)checked_malloc(sizeof(char) * strlen(argv[2])+1);
		strcpy(str, argv[2]);
		str[strlen(argv[2])] = '\0';
	}
	char c = argv[1][0];


	switch(c)
	{
		case 'a':
			printf("test queue\n");
			test_queue(str);
			break;
		case 'b':
			printf("test eat space\n");
			test_eatspace(str);
			break;
		case 'c':
			printf("test is word\n");
			test_isword(str);
			break;
		case 'd':
			printf("test eat word\n");
			test_eatword(str);
			break;
		case 'e':
			printf("test eat special\n");
			test_eatspecial(str);
			break;
		case 'f':
			printf("test comment\n");
			test_comment(str);
			break;
		case 'g':
			printf("test split token with whitespace\n");
			test_make_command_stream(str);
			break;

		case 'h':
			printf("test read simple command\n");
			test_simple_command(str);
			break;
		case 'i':
			printf("test read pipeline command\n");
			test_pipeline_command(str);
			break;
		case 'j':
			printf("test read andor command\n");
			test_andor_command(str);
			break;
		case 'k':
			printf("test read seq command\n");
			test_seq_command(str);
			break;
		case 'p':
			printf("test print command\n");
			test_print_queue(str);
			break;
		case 'l':
			printf("test command stream\n");
			test_command_stream_t(str);
	}

	return 0;
}	

void test_queue(const char* argv){
	queue Q;
	Q = q_empty();
	
	char* str = (char*) checked_malloc(sizeof(char) * 5 + 1);
	strcpy(str, "12345");
	Q = enqueue(b_pair(WORD, str), Q);
	printf("%s\n", str);
	
	str = (char*) checked_malloc(sizeof(char) * 5 + 1);
	strcpy(str, "abcde");
	Q = enqueue(b_pair(WORD, str), Q);
	printf("%s\n", str);

	while(!isempty(Q)) {
		pair *temp = next(Q);
		printf("%s\n", temp->value);
		Q = dequeue(Q);
		free(temp->value);
		free(temp);
	}

}

void test_eatspace(const char* argv) {
	printf("DISCARD.\n");
}

void test_isword(const char* argv) {
	printf("DISCARD.\n");
}

void test_eatword(const char* argv) {
	printf("DISCARD.\n");

}

void test_eatspecial(char* argv) {
	printf("DISCARD.\n");
}


void test_comment(char* argv) {
	printf("DISCARD.\n");
}

static int
get_next_byte (void *stream)
{
  return getc(stream);
}

void test_make_command_stream(char* argv) {
	printf("file name: %s\n", argv);
  	FILE *script_stream = fopen (argv, "r");
	make_command_stream(get_next_byte, script_stream);
}


void printf_command(command_t cmd, int indent) {
	if(cmd == NULL)
		return;
	int i = 0;
	for(i = 0; i < indent; i++)
		printf("[");
	switch(cmd->type) {
	case AND_COMMAND:
		if(cmd->type == AND_COMMAND)
			printf("printing read_and commands\n");
	case SEQUENCE_COMMAND:
		if(cmd->type == SEQUENCE_COMMAND)
			printf("printing read_seq commands\n");
	case OR_COMMAND:
		if(cmd->type == OR_COMMAND)
			printf("printing read_OR commands\n");
	case PIPE_COMMAND:
		if(cmd->type == PIPE_COMMAND)
			printf("printing read_pipeline commands\n");
		printf_command(cmd->u.command[0], indent+ 1);
		printf_command(cmd->u.command[1], indent+ 1);
		break;
	case SIMPLE_COMMAND:
		printf("printing SIMPLE_COMMAND commands\n");
		printf("in:%s\n", (cmd->input == NULL)?"NULL":cmd->input);
		printf("out:%s\n", (cmd->output == NULL)?"NULL":cmd->output);
		printf("word:[");
		char** it = cmd->u.word;
		while(*it != NULL) {
			printf("%s ", *it);
			it++;
		}
		printf("]\n");
		break;
	case SUBSHELL_COMMAND:
		printf("printing SUB_SHELL commands \n");
		printf("{\n");
		printf_command(cmd->u.subshell_command, indent);
		printf("}\n");

		break;
	default:
		printf("UNKNOWN\n");
	}
	for(i = 0; i < indent; i++)
		printf("]");
	printf("\n");
}

queue print_queue(queue Q) {
	queue P = q_empty();
	pair* p;
	while(isempty(Q) == false) {
		p = (pair*)next(Q);
		P = enqueue(b_pair(p->key, p->value), P);
		printf("[%d: %s]\n", p->key, p->value);
		
		Q = dequeue(Q);
	}
	return P;
}

void test_print_queue(char* argv) {
	queue Q = partition(argv);
	print_queue(Q);
}
void test_simple_command(char* argv) {
	printf("command:%s\n", argv);
	queue Q = partition(argv);

	command_t cmd = read_simple_command(&Q);
	printf_command(cmd,1);
}
void test_pipeline_command(char* argv) {

	printf("command:%s\n", argv);

	queue Q = build_token_queue(argv);

	Q = print_queue(Q);

	command_t cmd = read_pipeline(NULL, &Q);

	printf_command(cmd, 1);
}

void test_andor_command(char* argv) {

	printf("command:%s\n", argv);

	queue Q = build_token_queue(argv);

	command_t cmd = read_andor(NULL, &Q);

	printf_command(cmd, 1);
}

void test_seq_command(char* argv) {

	printf("command:%s\n", argv);

	queue Q = build_token_queue(argv);

	command_t cmd = read_seq(NULL, &Q);

	printf_command(cmd, 1);

	printf("stop at:\n");

	print_queue(Q);	

}

void test_command_stream_t(char* argv) {
	
	queue Q;
	Q = partition(argv);
	free(argv);
	command_stream_t T = build_token_tree(Q);
	command_t cmd;
	while((cmd = read_command_stream(T)))
		print_command(cmd);
}
