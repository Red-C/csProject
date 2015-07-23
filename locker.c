#include "locker.h"

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
		// if someone is writing, return false
		if(WLOCK(L, j)) 
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
		write_lock(L, j);
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
	printf("acqouring locks\n");
	for( i = 0; i < L->n_locks; i++) {
		printf("%s: %d %d\n", L->storage[i]->filename, RLOCK(L, i), WLOCK(L, i));
	}
	return true;
}

bool 
release_locks(locker* L, int* r_locker_id, int n, int* w_locker_id, int m) {
	// LOCK
	printf("waiting for mutex_locks in release_locks function\n");
	pthread_mutex_lock(&mutex_lock);
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
	printf("releasing locks\n");
	for( i = 0; i < L->n_locks; i++) {
		printf("%s: %d %d\n", L->storage[i]->filename, RLOCK(L, i), WLOCK(L, i));
	}
	pthread_mutex_unlock(&mutex_lock);
	return true;
}


void print_locker(locker* L) {
	int i = 0;
	for( i = 0; i < L->n_locks; i++) {
		printf("%s: %d %d\n", L->storage[i]->filename, RLOCK(L, i), WLOCK(L, i));
	}

}
