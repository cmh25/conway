#include "main.h"
#include "resource.h"
#include "view.h"
#include "conway.h"
#include "about.h"
#include "settings.h"
#include <commctrl.h>
#include <process.h>
#include <stdio.h>
#include "timer.h"

static HWND m_hwnd;
static HWND m_hwndRB;
static HWND m_hwndSB;
static HWND m_hwndView;
static HWND m_hwndTB;
static BOOL m_bRunning;
static int  m_delay;

static HWND CreateToolBar(HWND);
static HWND CreateStatusBar(HWND);
static HWND CreateRebar(HWND);
static HWND CreateTrackBar(HWND);
static void Go(void* p);

ATOM main_register_class() {
    WNDCLASSEX wcex;
    ZeroMemory(&wcex, sizeof(wcex));
    wcex.cbSize = sizeof(wcex);
    wcex.lpszClassName = "main";
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = main_wndproc;
    wcex.hInstance = g_hinst;
    wcex.hIcon = LoadIcon(g_hinst,  MAKEINTRESOURCE(IDI_ICON1));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.lpszMenuName = g_szAppTitle;
    wcex.hIconSm = LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_ICON1));
    return RegisterClassEx(&wcex);
}

HWND main_create() {
    HMENU hMenu = LoadMenu(g_hinst, MAKEINTRESOURCE(IDR_MENU_MAIN));

    m_hwnd = CreateWindow("main",
                          g_szAppTitle,
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT, 0,
                          CW_USEDEFAULT, 0,
                          NULL, hMenu,
                          g_hinst, NULL);
    return m_hwnd;
}

LRESULT CALLBACK main_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;
    RECT rc, rr, rs;
    int rrHeight, sbHeight, x, y, width, height;
    char sbt[512];

    switch(message) {
    case WM_CREATE:
        m_hwndRB = CreateRebar(hWnd);
        m_hwndSB = CreateStatusBar(hWnd);
        m_hwndView = view_create(hWnd);
        GetWindowRect(m_hwndRB, &rr);
        GetWindowRect(m_hwndSB, &rs);
        rrHeight = rr.bottom - rr.top - 2;
        sbHeight = rs.bottom - rs.top - 1;
        sprintf_s(sbt, 512, "generations: %d", conway_get_generations());
        SendMessage(m_hwndSB, SB_SETTEXT, MAKELONG(1, 0), (LPARAM)sbt);
        sprintf_s(sbt, 512, "%g ms/gen", 0.0);
        SendMessage(m_hwndSB, SB_SETTEXT, MAKELONG(2, 0), (LPARAM)sbt);
        sprintf_s(sbt, 512, "alive: %d/%d", 0, 0);
        SendMessage(m_hwndSB, SB_SETTEXT, MAKELONG(3, 0), (LPARAM)sbt);
        break;
    case WM_DESTROY:
        DestroyWindow(m_hwndView);
        DestroyWindow(m_hwndSB);
        DestroyWindow(m_hwndRB);
        PostQuitMessage(0);
        break;
    case WM_COMMAND:
        wmId    = LOWORD(wParam);
        wmEvent = HIWORD(wParam);
        switch(wmId) {
        case ID_FILE_EXIT:
            DestroyWindow(m_hwnd);
            break;
        case ID_BUTTON_RED:
            m_bRunning = FALSE;
            Sleep(100);
            break;
        case ID_BUTTON_GREEN:
            if(!conway_is_inited())
                conway_init_cells(view_get_rows(),view_get_cols());
            view_refresh();
            view_draw_cells();
            _beginthread(Go, 0, NULL);
            break;
        case ID_BUTTON_BLUE:
            m_bRunning = FALSE;
            Sleep(100);
            conway_init_cells(view_get_rows(),view_get_cols());
            view_refresh();
            view_draw_cells();
            sprintf_s(sbt, 512, "alive: %d/%d", conway_get_nalive(), conway_get_rows() * conway_get_cols());
            SendMessage(m_hwndSB, SB_SETTEXT, MAKELONG(3, 0), (LPARAM)sbt);
            break;
        case ID_HELP_ABOUT:
			DialogBox(g_hinst, (LPCTSTR)IDD_ABOUT, hWnd, (DLGPROC)about_wndproc);
            break;
        case ID_EDIT_SETTINGS:
            DialogBox(g_hinst, (LPCTSTR)IDD_SETTINGS, hWnd, (DLGPROC)settings_wndproc);
            break;
        case ID_COMBOBOX:
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
            break;
        }
        break;
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;
    case WM_SIZE:
        SendMessage(m_hwndSB, WM_SIZE, wParam, lParam);
        SendMessage(m_hwndRB, WM_SIZE, wParam, lParam);
        GetClientRect(m_hwnd, &rc);
        GetWindowRect(m_hwndRB, &rr);
        GetWindowRect(m_hwndSB, &rs);
        rrHeight = rr.bottom - rr.top - 2;
        sbHeight = rs.bottom - rs.top - 1;
        x = 0;
        y = rrHeight;
        width = rc.right - rc.left;
        height = rc.bottom - rc.top - rrHeight - sbHeight - 1;
        SetWindowPos(m_hwndView, HWND_TOP, x, y, width, height, SWP_SHOWWINDOW);
        SendMessage(m_hwndView, WM_SIZE, wParam, lParam);
        sprintf_s(sbt, 512, "%dx%d", width, height);
        SendMessage(m_hwndSB, SB_SETTEXT, MAKELONG(0,0), (LPARAM)sbt);
        break;
    case WM_NOTIFY:
        switch(wParam) {
        case ID_REBAR:
            if(((LPNMHDR)lParam)->code == RBN_HEIGHTCHANGE)
                SendMessage(m_hwnd, WM_SIZE, 0, 0);
            break;
        case ID_TRACKBAR:
            m_delay = 99 - (int)SendMessage(m_hwndTB, TBM_GETPOS, 0, 0);
            break;
        default:
            break;
        }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
   }
   return 0;
}

HWND CreateStatusBar(HWND hwndParent) {
    HWND hwndSB;
    int a[5] = { 60, 200, 300, 500, 600 };

    hwndSB = CreateWindow(STATUSCLASSNAME,
                          NULL,
                          WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | SBARS_SIZEGRIP,
                          0, 0, 0, 0,
                          hwndParent,
                          (HMENU)ID_STATUS,
                          g_hinst, NULL);
    SendMessage(hwndSB, SB_SETPARTS, 5, (LPARAM)a);
    return hwndSB;
}

HWND CreateRebar(HWND hwndParent) {
    HWND hwndRB, hwndTB, hwndTrackBar;
    REBARBANDINFO rbbi;
    INITCOMMONCONTROLSEX icex;
    RECT tbr;
    
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC  = ICC_BAR_CLASSES | ICC_COOL_CLASSES | ICC_USEREX_CLASSES;
    InitCommonControlsEx(&icex);

    hwndRB = CreateWindowEx(WS_EX_TOOLWINDOW,
                            REBARCLASSNAME,
                            NULL,
                            WS_VISIBLE | WS_BORDER | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
                                RBS_VARHEIGHT | RBS_BANDBORDERS,
                            0, 0, 0, 0,
                            hwndParent,
                            0,
                            g_hinst,
                            NULL );

    hwndTB = CreateToolBar(hwndRB);

    // Get the height of the toolbar.
    DWORD dwBtnSize = (DWORD)SendMessage(hwndTB, TB_GETBUTTONSIZE, 0, 0);

    ZeroMemory(&rbbi, sizeof(rbbi));
    rbbi.cbSize = REBARBANDINFO_V3_SIZE;
    rbbi.fMask = RBBIM_COLORS | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_STYLE;
    rbbi.clrFore = GetSysColor(COLOR_BTNTEXT);
    rbbi.clrBack = GetSysColor(COLOR_BTNFACE);
    rbbi.fStyle = RBBS_GRIPPERALWAYS | RBBS_CHILDEDGE;
    rbbi.hwndChild = hwndTB;
    rbbi.cxMinChild = 3*LOWORD(dwBtnSize);
    rbbi.cyMinChild = 10+HIWORD(dwBtnSize);
    SendMessage(hwndRB, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbbi);

    hwndTrackBar = CreateTrackBar(hwndRB);
    GetWindowRect(hwndTrackBar, &tbr);
    rbbi.hwndChild = hwndTrackBar;
    rbbi.cxMinChild = tbr.right - tbr.left - 1;
    rbbi.cyMinChild = 24;
    rbbi.lpText = "speed:";
    rbbi.fMask |= RBBIM_TEXT;
    SendMessage(hwndRB, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbbi);

    return hwndRB;
}

HWND CreateToolBar(HWND hWndParent) {
    const int numButtons = 3;
    const int bitmapSize = 16;

    HWND hWndToolbar = CreateWindowEx(0, TOOLBARCLASSNAME, NULL,
        WS_CHILD | CCS_NORESIZE | TBSTYLE_TOOLTIPS | TBSTYLE_TRANSPARENT,
        0, 0, 0, 0,
        hWndParent, NULL, g_hinst, NULL);

    if (hWndToolbar == NULL)
        return NULL;

    // Create the image list.
    HIMAGELIST hil = ImageList_Create(bitmapSize, bitmapSize, ILC_COLOR16, numButtons, 0);
    SendMessage(hWndToolbar, TB_SETIMAGELIST, 0, (LPARAM)hil);

    // Load the button images.
    TBADDBITMAP tb = { g_hinst, IDR_TOOLBAR_MAIN };
    SendMessage(hWndToolbar, TB_ADDBITMAP, numButtons, (LPARAM)&tb);
    TBBUTTON tbButtons[] =
    { { 0, ID_BUTTON_RED,  TBSTATE_ENABLED, BTNS_AUTOSIZE, {0}, 0, (INT_PTR)"stop" },
      { 1, ID_BUTTON_GREEN, TBSTATE_ENABLED, BTNS_AUTOSIZE, {0}, 0, (INT_PTR)"go" },
      { 2, ID_BUTTON_BLUE, TBSTATE_ENABLED, BTNS_AUTOSIZE, {0}, 0, (INT_PTR)"init" } };
    SendMessage(hWndToolbar, TB_SETMAXTEXTROWS, 0, 0);
    SendMessage(hWndToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
    SendMessage(hWndToolbar, TB_ADDBUTTONS, (WPARAM)numButtons, (LPARAM)&tbButtons);

    return hWndToolbar; 
}

HWND CreateTrackBar(HWND hwndParent) {
    HWND st;

    st = CreateWindow("static",
                      "",
                      WS_CHILD,
                      0, 0, 200, 30,
                      hwndParent,
                      NULL,
                      g_hinst,
                      NULL);

    m_hwndTB = CreateWindowEx(0,
                        TRACKBAR_CLASS,
                        "Trackbar Control",
                        WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS,
                        0, 0, 200, 30,
                        hwndParent,
                        (HMENU)ID_TRACKBAR,
                        g_hinst,
                        NULL);

    SetParent(m_hwndTB, st);

    SendMessage(m_hwndTB, TBM_SETRANGE, (WPARAM) TRUE,  (LPARAM) MAKELONG(0, 99));
    SendMessage(m_hwndTB, TBM_SETPAGESIZE, 0, (LPARAM)4);
    SendMessage(m_hwndTB, TBM_SETPOS, (WPARAM) TRUE, (LPARAM) 99);

    return st;
}

void Go(void* p) {
    char sbt[512];
    if(m_bRunning) return;
    m_bRunning = TRUE;
    UINT g = 1;
    float gt, dt, ut;
    while (m_bRunning) {
        timer_start();
        conway_calc_next_gen();
        gt = timer_stop();

        timer_start();
        view_draw_cells();
        dt = timer_stop();

        timer_start();
        conway_update_cells();
        ut = timer_stop();

        if (0 == g % 10) {
            sprintf_s(sbt, 512, "generations: %d", conway_get_generations());
            SendMessage(m_hwndSB, SB_SETTEXT, MAKELONG(1, 0), (LPARAM)sbt);
            //sprintf_s(sbt, 512, "gt:%g dt:%g ut:%g", gt, dt, ut);
            sprintf_s(sbt, 512, "%g ms/gen", gt+dt+ut);
            SendMessage(m_hwndSB, SB_SETTEXT, MAKELONG(2, 0), (LPARAM)sbt);
            sprintf_s(sbt, 512, "alive: %d/%d", conway_get_nalive(), conway_get_rows() * conway_get_cols());
            SendMessage(m_hwndSB, SB_SETTEXT, MAKELONG(3, 0), (LPARAM)sbt);
        }

        Sleep(m_delay);
        g++;
    }
}
