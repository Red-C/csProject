#ifndef VECTOR_H
#define VECTOR_H
#define VECTOR_INIT_CAP 100
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct vector {
	int cap;
	int size;
	void** array;

} vector;

vector *vector_init();
int vector_size();
void vector_insert(vector*, void*);
void vector_set(vector*, int, void*);
void *vector_get(vector*, int);
void vector_free(vector*);
void* vector_delete(vector* v, int i);
int vector_contains(vector* v, void* item, int (*cmp)(void*, void*));
bool vector_isempty(vector* v);
void find_and_delete(vector* v, void* node);
#define vector_sizeof(v) v->size


#endif
