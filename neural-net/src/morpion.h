#ifndef MORPION_H_DEF
#define MORPION_H_DEF

#include "brain.h"

struct morpion {
	int playCount;
	char grid[9];
};

struct morpion* morpion_new(int playCount);
void morpion_reset(struct morpion *m);
int morpion_intToGrid(struct morpion *m, char s, int n);
void morpion_AIplay(struct morpion *m, struct brain* b);

#endif