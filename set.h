#ifndef SET_H
#define SET_H
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

//set declaration.
typedef struct fileSet
{
	char** fileName; //an array of pointers to file names.
	int size;
	int sizeCap; //size is the number of pointers in the array.
} set;

struct fileSet createFileSet();

//return 1 if s contains the file, returns 0 otherwise.
int contain(const struct fileSet *s, char* file);

//add file to the set if it doesn't exit yet.
void addFile(struct fileSet *s, char* file);

//return the union of the two sets.
struct fileSet* unionSet(struct fileSet *s1, const struct fileSet *s2);

//return the intersection of the two sets.
struct fileSet interSet(const struct fileSet *s1, const struct fileSet *s2);

//return the difference of the two sets, i.e., s1 - s2.
struct fileSet diffSet(struct fileSet *s1, struct fileSet *s2);

//free the spaces used by the set.
void cleanSet(struct fileSet *s);

bool is_intersect(const struct fileSet *left, const struct fileSet *right) ;

int indexOf(struct fileSet *s, char* file);


#endif
