#ifndef CONWAY_H
#define CONWAY_H

void conway_init_cells(int rows, int cols);
void conway_calc_next_gen();
void conway_update_cells();
int* conway_get_cells();
int  conway_get_rows();
int  conway_get_cols();
int  conway_is_inited();
int  conway_get_generations();
int  conway_get_nalive();

#endif /* CONWAY_H */
