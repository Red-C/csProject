#ifndef LOCKER_H
#define LOCKER_H
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>


pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER;

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


locker
create_locker(char** all_files, int n);

bool 
release_locks(locker* L, int* r_locker_id, int n, int* w_locker_id, int m);

bool 
get_locks(locker* L, int* r_locker_id, int n, int* w_locker_id, int m);

void print_locker(locker* L);
#endif
