#include "view.h"
#include "conway.h"
#include <d2d1.h>
#include <math.h>
#include "globals.h"

static HBITMAP  m_memBitmap;
static HBITMAP  m_oldBitmap;
static HDC      m_viewDC;
static HDC      m_memDC;
static HWND     m_hwnd;
static TCHAR*   m_szClassView = "view";
static BOOLEAN  m_bRefresh = TRUE;
static int      m_rows = 200;
static int      m_cols = 400;
static int      m_delay = 0;
static ID2D1HwndRenderTarget* m_pRenderTarget;
static ID2D1SolidColorBrush* m_brDead;
static ID2D1SolidColorBrush* m_brLive;
static float    m_alive_r=0.0, m_alive_g=0.0, m_alive_b=1.0;
static float    m_dead_r=0.0, m_dead_g=0.0, m_dead_b=0.0;

static CRITICAL_SECTION m_cs;

ATOM view_register_class() {
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.lpszClassName = m_szClassView;
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = view_wndproc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = g_hinst;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = NULL;
    wcex.lpszMenuName = NULL;
    wcex.hIconSm = NULL;

    return RegisterClassEx(&wcex);
}

HWND view_create(HWND hwndParent) {
    m_hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, m_szClassView, g_szAppTitle,
                          WS_CHILD | WS_VISIBLE,
                          0, 0, 0, 0,
                          hwndParent, NULL,
                          g_hinst, NULL);

    SendMessage(m_hwnd, WM_SIZE, 0, 0);

    return m_hwnd;
}

LRESULT CALLBACK view_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    PAINTSTRUCT ps;
    HDC hdc;
    static RECT rv, rc, rr, rs;
    static int rrHeight, sbHeight, x, y, width, height;
    static BOOLEAN bFirstPaint = TRUE;

    switch(message) {
    case WM_CREATE:
        InitializeCriticalSection(&m_cs);
        break;
    case WM_DESTROY:
        DeleteCriticalSection(&m_cs);
        SafeRelease(&m_pRenderTarget);
        SafeRelease(&m_brLive);
        SafeRelease(&m_brDead);
        break;
    case WM_PAINT:
        if(bFirstPaint) {
            bFirstPaint = FALSE;
            GetClientRect(m_hwnd, &rv);

            D2D1_SIZE_U size = D2D1::SizeU(
                rv.right - rv.left,
                rv.bottom - rv.top
            );

            HRESULT hr = g_pDirect2dFactory->CreateHwndRenderTarget(
                D2D1::RenderTargetProperties(),
                D2D1::HwndRenderTargetProperties(m_hwnd, size),
                &m_pRenderTarget
            );
            if (SUCCEEDED(hr)) {
                m_pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
            }
            if (SUCCEEDED(hr))
            {
                hr = m_pRenderTarget->CreateSolidColorBrush(
                    D2D1::ColorF(m_alive_r,m_alive_g,m_alive_b,1),
                    &m_brLive
                );
            }
            if (SUCCEEDED(hr))
            {
                hr = m_pRenderTarget->CreateSolidColorBrush(
                    D2D1::ColorF(m_dead_r,m_dead_g,m_dead_b,1),
                    &m_brDead
                );
            }
        }
        hdc = BeginPaint(hWnd, &ps);
        if(!conway_is_inited()) FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
        EndPaint(hWnd, &ps);
        break;
    case WM_SIZE:
        m_bRefresh = TRUE;
        GetClientRect(m_hwnd, &rv);
        if(m_pRenderTarget) m_pRenderTarget->Resize(D2D1::SizeU(rv.right - rv.left, rv.bottom - rv.top));
        if(conway_is_inited()) view_draw_cells();
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }
    return 0;
}

void view_draw_cells() {

    float cellW, cellH;
    int  i, j, rows, cols, ic;
    int *pcells;
    D2D1_RECT_F cellRect;

    EnterCriticalSection(&m_cs);

    pcells = conway_get_cells();
    rows = conway_get_rows();
    cols = conway_get_cols();

    m_pRenderTarget->BeginDraw();
    m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

    D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();

    float width = rtSize.width;
    float height = rtSize.height;

    cellW = width / cols;
    cellH = height / rows;

    cellRect = D2D1::RectF(0, 0, width, height);
    m_pRenderTarget->FillRectangle(&cellRect, m_brDead);

    cellRect = D2D1::RectF(0,0,cellW,cellH);
    
    for(i=0; i<rows; i++) {
        ic = i * cols;
        for(j=0; j<cols; j++) {
            if(pcells[ic+j]) m_pRenderTarget->FillRectangle(&cellRect, m_brLive);
            cellRect.left = cellRect.right;
            cellRect.right += cellW;
        }
        cellRect.left = 0;
        cellRect.right = cellW;
        cellRect.top = cellRect.bottom;
        cellRect.bottom += cellH;
    }

    m_pRenderTarget->EndDraw();

    LeaveCriticalSection(&m_cs);
}

void view_refresh() {
    m_bRefresh = TRUE;
}

int view_get_rows() {
    return m_rows;
}

int view_get_cols() {
    return m_cols;
}

void view_set_rows(int r) {
    m_rows = r;
}

void view_set_cols(int c) {
    m_cols = c;
}

int  view_get_alivec(int c) {
    if (c == 0) return (int)(m_alive_r * 255);
    else if (c == 1) return (int)(m_alive_g * 255);
    else if (c == 2) return (int)(m_alive_b * 255);
    else return 0;
}

void view_set_alivec(int r, int g, int b) {
    m_alive_r = r / 255.0f;
    m_alive_g = g / 255.0f;
    m_alive_b = b / 255.0f;
    SafeRelease(&m_brLive);
    m_pRenderTarget->CreateSolidColorBrush(
        D2D1::ColorF(m_alive_r, m_alive_g, m_alive_b, 1),
        &m_brLive
    );
}

int  view_get_deadc(int c) {
    if (c == 0) return (int)(m_dead_r * 255);
    else if (c == 1) return (int)(m_dead_g * 255);
    else if (c == 2) return (int)(m_dead_b * 255);
    else return 0;
}

void view_set_deadc(int r, int g, int b) {
    m_dead_r = r / 255.0f;
    m_dead_g = g / 255.0f;
    m_dead_b = b / 255.0f;
    SafeRelease(&m_brDead);
    m_pRenderTarget->CreateSolidColorBrush(
        D2D1::ColorF(m_dead_r, m_dead_g, m_dead_b, 1),
        &m_brDead
    );
}