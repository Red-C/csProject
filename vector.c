#include "vector.h"
vector *vector_init() {
	vector *v = (vector*)malloc(sizeof(vector));
	v->cap = VECTOR_INIT_CAP; 
	v->size = 0;
	v->array = (void**)malloc(sizeof(void *) * v->cap);
	return v;

}


void vector_insert(vector* v, void* x)
{
	if(v->cap == v->size) {
		void **n;
		n = (void**)realloc(v->array, v->cap * 2 * sizeof(void*));
		if(n != NULL) {
			v->array = n;
			v->cap = v->cap * 2;
		}
	}
	v->array[v->size++] = x;
}

void vector_set(vector* v, int i, void* n)
{
	if(i >= 0 && i < v->size) 
		v->array[i] = n;
}
void *vector_get(vector* v, int i)
{
	if(i >= 0 && i < v->size) 
		return v->array[i];
	return NULL;

}
void* vector_delete(vector* v, int i) {
	if(i < 0 || i >= v->size)
		return NULL;
	if(v->size == 0)
		return NULL;

	void* ret = v->array[i];
	for(; i < v->size; i++) {
		v->array[i] = v->array[i+1];
	}
	v->size--;
	return ret;
}
void find_and_delete(vector* v, void* node) {
	int i = 0;
	if(v->size == 0)
		return;

	for(i = 0; i < v->size; i++ )
		if(v->array[i] == node)
			vector_delete(v, i);


}

bool vector_isempty(vector* v) {
	return v->size == 0;
}

void vector_free(vector* v)
{
	free(v->array);
	free(v);
}
int vector_contains(vector* v, void* item, int (*cmp)(void*, void*))
{
	int i = 0;
	for(i = 0; i<v->size; i++) {
		if(cmp(v->array[i],item) == 0)
			return i;
	}
	return -1;


}
