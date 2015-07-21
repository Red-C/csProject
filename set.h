#ifndef SET_H
#define SET_H
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

//set declaration.
typedef struct set
{
	void* item; //an array of pointers to file names.
	int size;
	int sizeCap; //size is the number of pointers in the array.
} set;

//call this function to create an empty set
//don't declare any set without this fucntion.
set create_set();

//return 1 if s contains the file, returns 0 otherwise.
int contain(const set *s, void* item, int (*cmp)(void*, void*));


//add file to the set if it doesn't exit yet.
void addFile(set *s, void* item);

//return the union of the two sets.
set* unionSet(set *s1, const set *s2, int (*cmp)(void*, void*));

//return the intersection of the two sets.
set interSet(const set *s1, const set *s2, int (*cmp)(void*, void*));

//return the difference of the two sets, i.e., s1 - s2.
set* diffSet(set *s1, set *s2, int (*cmp)(void*, void*));

//free the spaces used by the set.
void cleanSet(set *s);

bool is_intersect(const set *left, const set *right, int (*cmp)(void*, void*));


#endif
