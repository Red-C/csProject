#ifndef LOCKER_H
#define LOCKER_H
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include "vector.h"
//#define DEBUG_MODE


static pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct box 
{
	volatile int n_reading;
	bool is_wlocked;
	char* filename;
	vector* r_holder_list;
	void* w_holder;
	int n;

} box;

typedef struct locker 
{
	box** storage;
	int n_locks;
	pthread_mutex_t mutex_lock;
	int hasChanged;
	int current_holder;
	
} locker;

typedef box* box_t;


locker create_locker(char** all_files, int n);
bool release_locks(void* cmd, locker* L, int* r_locker_id, int n, 
		int* w_locker_id, int m,
		int (*)(void*, void*));

bool get_locks(void* cmd, locker* L, int* r_locker_id, int n, 
		int* w_locker_id, int m,
		int (*)(void*, void*));
void print_locker(locker* L);

#endif
