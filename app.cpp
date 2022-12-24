#include <windows.h>
#include <commctrl.h>
#include <process.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include "resource.h"
#include "conway.h"
#include "about.h"
#include "main.h"
#include "view.h"
#include "globals.h"

TCHAR *g_szAppTitle = "conway";
HINSTANCE g_hinst;
static HWND m_hwndMain;
ID2D1Factory *g_pDirect2dFactory;

static BOOL InitInstance(int);

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
                   _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
    MSG msg;
    HACCEL hAccelTable;
    HRESULT hr;

    g_hinst = hInstance;

    main_register_class();
    view_register_class();

    if (FAILED(CoInitialize(NULL))) { MessageBox(0, "failed to initialize com", "error", MB_OK); exit(1); }
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &g_pDirect2dFactory);
    if (FAILED(hr)) { MessageBox(0, "failed to create d2d factory", "error", MB_OK); exit(1); }

    if(!InitInstance(nCmdShow)) {
        return FALSE;
    }

    hAccelTable = LoadAccelerators(g_hinst, (LPCSTR)IDR_ACCELERATOR);

    while(GetMessage(&msg, NULL, 0, 0)) {
        if(!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    SafeRelease(&g_pDirect2dFactory);
    CoUninitialize();

    return (int)msg.wParam;
}

BOOL InitInstance(int nCmdShow) {
    
    m_hwndMain = main_create();
    if(!m_hwndMain) return FALSE;

    ShowWindow(m_hwndMain, nCmdShow);
    UpdateWindow(m_hwndMain);

    return TRUE;
}
