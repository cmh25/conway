#ifndef GLOBALS_H
#define GLOBALS_H

#include <windows.h>
#include <d2d1.h>

extern HINSTANCE g_hinst;
extern TCHAR *g_szAppTitle;
extern ID2D1Factory *g_pDirect2dFactory;

template<class Interface>
inline void SafeRelease(
    Interface** ppInterfaceToRelease)
{
    if (*ppInterfaceToRelease != NULL)
    {
        (*ppInterfaceToRelease)->Release();
        (*ppInterfaceToRelease) = NULL;
    }
}

#endif /* GLOBALS_H */
