#ifndef VIEW_H
#define VIEW_H

#include "globals.h"

LRESULT CALLBACK view_wndproc(HWND, UINT, WPARAM, LPARAM);

ATOM view_register_class();
HWND view_create(HWND);
void view_draw_cells();
void view_refresh();
int  view_get_rows();
int  view_get_cols();
void view_set_rows(int r);
void view_set_cols(int c);
int  view_get_alivec(int c);
void view_set_alivec(int r, int g, int b);
int  view_get_deadc(int c);
void view_set_deadc(int r, int g, int b);

#endif /* VIEW_H */
