#include "common.h"
#include "morpion.h"
#include "data.h"

struct morpion* morpion_new(int playCount) {
	struct morpion *m = malloc(sizeof(struct morpion));
	if (m == NULL)
		exit(1);
	m->playCount = playCount;
	memset(m->grid, '_', 9);
	return m;
}

int morpion_intToGrid(struct morpion *m, char s, int n) {
	if (m->grid[n-1] != '_') {
		printf("impossible to play at %i\n", n);
		return -1;
	}
	m->grid[n-1] = s;
	m->playCount++;
}

void morpion_AIplay(struct morpion *m, struct brain *b) {
	float in[9];
	for (int i=0; i<9; i++) {
		in[i] = inStrToFl(m->grid[i]);
	}
	brain_run(b, (double*)in);
	morpion_intToGrid(m, 'o', brain_outToInt(b)+1);
}

void morpion_reset(struct morpion *m) {
	memset(m->grid, '_', 9);
	m->playCount = 0;
}
