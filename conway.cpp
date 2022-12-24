#include "conway.h"
#include <stdlib.h>

static int inited=0, gens=0, na=0;
static int *a=0, *b=0, *nc=0, c, r, n, le, re, te, be;

static void livedie(int i, int ld) {
	int j[8], k;
	if (a[i] == ld) return;
	a[i] = ld;
	b[i] = 0;
	/* neighbor indices */
	j[0] = i - 1;
	j[1] = i - c - 1;
	j[2] = i - c;
	j[3] = i - c + 1;
	j[4] = i + 1;
	j[5] = i + c + 1;
	j[6] = i + c;
	j[7] = i + c - 1;
	if (i % c == 0) { /* left edge */
		j[0] += c;
		j[1] += c;
		j[7] += c;
	}
	else if ((i + 1) % c == 0) { /* right edge */
		j[3] -= c;
		j[4] -= c;
		j[5] -= c;
	}
	if (i < c) { /* top edge */
		j[1] += n;
		j[2] += n;
		j[3] += n;
	}
	else if (i >= n - c) { /* bottom edge */
		j[5] -= n;
		j[6] -= n;
		j[7] -= n;
	}
	/* live or die */
	if(ld) for (k = 0; k < 8; k++) nc[j[k]]++;
	else for (k = 0; k < 8; k++) nc[j[k]]--;
}

int conway_get_nalive() { return na; }
int* conway_get_cells() { return a; }
int conway_get_rows() { return r; }
int conway_get_cols() { return c; }
int conway_is_inited() { return inited; }
int conway_get_generations() { return gens; }

void conway_init_cells(int rows, int cols) {
	int i;

	if (a) free(a);
	if (b) free(b);
	if (nc) free(nc);

	r = rows;
	c = cols;
	n = r * c;

	a = (int*)calloc(n, sizeof(int));
	b = (int*)calloc(n, sizeof(int));
	nc = (int*)calloc(n, sizeof(int));

	/* random life */
	na = n * 3 / 4;
	for (i = 0; i < na; i++)
		livedie((rand()<<16|rand())%n, 1);

	inited = 1;
	gens = 0;
}

void conway_calc_next_gen() {
	for (int i = 0; i < n; i++) {
		b[i] = 0;
		if (a[i] && nc[i] == 2 || nc[i] == 3) b[i] = 1;
	}
	gens++;
}

void conway_update_cells() {
	na = 0;
	for (int i = 0; i < n; i++) {
		na += b[i];
		livedie(i, b[i]);
	}
}