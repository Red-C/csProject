#include "locker.h"
#define set_write_lock(L, i, state) L->storage[i]->is_wlocked = state 
#define write_lock(L, i) set_write_lock(L, i, true)
#define write_unlock(L, i) set_write_lock(L,i, false)
#define RLOCK(L, i) (L->storage[i]->n_reading != 0)
#define WLOCK(L, i) (L->storage[i]->is_wlocked)

pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER;

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
		L.storage[i]->w_holder = NULL;
		L.storage[i]->r_holder_list = vector_init();
	}
	return L;
}


bool 
get_locks(void* cmd, locker* L, int* r_locker_id, int n, 
								int* w_locker_id, int m,
								int (*cmp)(void*, void*)) {
	
	int i = 0,j = 0;
	int n_holds = 0;
	// check reading locks
	for(i = 0; i < n; i++) {
		j = r_locker_id[i];
		// if someone is writing
		if(RLOCK(L,j) == false && WLOCK(L, j)) 
		{
			// ignore
		}
		else {
			// add cmd into reading list
			if(vector_contains(L->storage[j]->r_holder_list, cmd, cmp) == -1) {
				vector_insert(L->storage[j]->r_holder_list, cmd);
				L->storage[j]->n_reading++;
			}
			n_holds++;
			
		}
	}

	// check writing locks
	for(i = 0; i < m; i++) {
		j = w_locker_id[i];
		// if someone is reading, do not get write lock
		if(RLOCK(L, j)) 
		{
			if(L->storage[j]->r_holder_list->size == 1 
					&& vector_contains(L->storage[j]->r_holder_list,
										cmd, cmp) != -1){

				if(WLOCK(L,j) == false) {
					
					// if write holder is NULL
					// let current command holds the box
					L->storage[j]->w_holder = cmd;
					write_lock(L, j);				
					n_holds++;
				}
				else if(WLOCK(L,j) == true){
					if(L->storage[j]->w_holder == cmd)
						n_holds++;
				}

			}

		}
		else if(WLOCK(L,j) == false) {
			
			// if write holder is NULL
			// let current command holds the box
			L->storage[j]->w_holder = cmd;
			write_lock(L, j);				
			n_holds++;
		}
		else if(WLOCK(L,j) == true){
			if(L->storage[j]->w_holder == cmd)
				n_holds++;
		}
	}
#ifdef DEBUG_MODE
	printf("unlocked\n");
	for( i = 0; i < L->n_locks; i++) {
		printf("%s: %d %d %d\n",  L->storage[i]->filename,L->storage[i]->n_reading, RLOCK(L, i), WLOCK(L, i));
	}
#endif
	return (n_holds == (n + m));
}

bool 
release_locks(void* cmd, locker* L, 
				int* r_locker_id, int n, 
				int* w_locker_id, int m,
				int (*cmp)(void*, void*)) {
	// LOCK
	// pthread_mutex_lock(&mutex_lock);
	int i = 0, j = 0;
	// release read locks
	for(i = 0; i < n; i++) {
	    j = r_locker_id[i];
		box* container = L->storage[j];

		int index = vector_contains(container->r_holder_list,(void*)cmd, cmp);
		assert(index != -1);
		// decrement number of people that is reading
		container->n_reading--;
		
		vector_delete(container->r_holder_list, index);

		assert(container->n_reading >= 0);
		// if no one is reading, allow other people to write
		if(RLOCK(L, j) == false)
			write_unlock(L, j);
#ifdef DEBUG_MODE
		else 
			printf("reading queue: %d\n", L->storage[j] ->n_reading);
#endif

	}
	for(i = 0; i < m; i++) {
		// release write lock
	    j = w_locker_id[i];
		box* container = L->storage[j];
		assert(container->w_holder == cmd);
		container->w_holder = NULL;
		write_unlock(L, j);


	}
#ifdef DEBUG_MODE
	// UNLOCK
	printf("unlocked\n");
	for( i = 0; i < L->n_locks; i++) {
		printf("%s: %d %d %d\n",  L->storage[i]->filename,L->storage[i]->n_reading, RLOCK(L, i), WLOCK(L, i));
	}
#endif
	//pthread_mutex_unlock(&mutex_lock);
	return true;
}


void print_locker(locker* L) {
	int i = 0;
	for( i = 0; i < L->n_locks; i++) {
		printf("%s: %d %d\n", L->storage[i]->filename, RLOCK(L, i), WLOCK(L, i));
	}

}

