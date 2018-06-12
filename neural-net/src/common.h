#ifndef COMMON_H_DEF
#define COMMON_H_DEF

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ARRAY_SIZE(x) sizeof(x)/sizeof(x[0])
#define RANDOM_WEIGHT() ((double)rand()/(double)RAND_MAX)*2*maxWeight-maxWeight

static int maxWeight = 1;
static double learnRate = 0.5;
static int batchSize = 100;

/*** some primitives to work on strings ***/
struct LinesArray {
	int number;
	char** lines;
};

char* readFileToString(char* file);
void writeStringToFile(char* file, char* str);
struct LinesArray readFileToLinesArray(char* file);
void LinesArray_free(struct LinesArray* l);
// WARNING: this will alter the string ('str') !!!
// DO NOT free, this doesn't allocate memory, but points to existing memory locations in 'str'
char** cut_on_delim(char* str, char delim);
// assume str is allocated on the heap using the generic allocator
int append_text(char* str, char* to_append, int size);

#endif
