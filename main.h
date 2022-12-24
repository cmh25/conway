#ifndef MAIN_H
#define MAIN_H

#include "globals.h"

LRESULT CALLBACK main_wndproc(HWND, UINT, WPARAM, LPARAM);

ATOM main_register_class();
HWND main_create();

#endif /* MAIN_H */
