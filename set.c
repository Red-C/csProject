#include "set.h"
//call this function to create an empty set
//don't declare any set without this fucntion.
struct fileSet createFileSet()
{
	struct fileSet s;
	s.size = 0;
	s.sizeCap = 10; //a new set can hold up to 10 file names.
	s.fileName = (char**)malloc(s.sizeCap * sizeof(char*));
	return s;
}


//return 1 if s contains the file, returns 0 otherwise.
int contain(const struct fileSet *s, char* file)
{
	int i;

	for (i = 0; i < s->size; i++)
		if (strcmp(s->fileName[i], file) == 0)
			return 1;

	return 0;
}


//add file to the set if it doesn't exit yet.
void addFile(struct fileSet *s, char* file)
{
	//if the set already contains that file, do nothing and return.
	if (contain(s, file))
		return;

	//if the set is full, extend its capacity by a factor of 1.6 first.
	if (s->size == s->sizeCap - 1)
	{
		s->sizeCap = s->sizeCap * 1.6;
		s->fileName = (char**)realloc(s->fileName, s->sizeCap*sizeof(char*));
	}

	//then add the file.
	s->fileName[s->size] = (char*)malloc((strlen(file) + 1) * sizeof(char));
	strcpy(s->fileName[s->size], file);

	//lastly, increment the size.
	s->size++;

}

//return the union of the two sets.
struct fileSet* unionSet(struct fileSet *s1, const struct fileSet *s2)
{
	//add files in s2 into u.
	int i = 0;
	for (i = 0; i < s2->size; i++)
		addFile(s1, s2->fileName[i]);

	return s1;
}

//return the intersection of the two sets.
struct fileSet interSet(const struct fileSet *s1, const struct fileSet *s2)
{
	struct fileSet i = createFileSet();
	int j;

	//check if any file in s1 is also contained in s2.
	//If so, add into i.
	for (j = 0; j < s1->size; j++)
		if (contain(s2, s1->fileName[j]))
			addFile(&i, s1->fileName[j]);

	return i;
}

//return the difference of the two sets, i.e., s1 - s2.
struct fileSet diffSet(struct fileSet *s1, struct fileSet *s2)
{
	struct fileSet d = createFileSet();
	int j;

	//check if any file in s1 is also contained in s2.
	//If not, add into i.
	for (j = 0; j < s1->size; j++)
		if (!contain(s2, s1->fileName[j]))
			addFile(&d, s1->fileName[j]);

	return d;
}

//free the spaces used by the set.
void cleanSet(struct fileSet *s)
{
	//first free the space taken by the file names.
	int i;
	for (i = 0; i < s->size; i++)
		free(s->fileName[i]);

	//then free the array of pointers to those file names.
	free(s->fileName);
}

bool is_intersect(const struct fileSet *left, const struct fileSet *right) 
{
	struct fileSet inter = interSet(left, right);
	bool flag = (inter.size != 0);
	cleanSet(&inter);
	return flag;
}
