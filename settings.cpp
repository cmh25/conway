#include "settings.h"
#include "resource.h"
#include <commctrl.h>
#include <stdlib.h>
#include <stdio.h>
#include "view.h"

LRESULT CALLBACK editProc(HWND hWnd, UINT uMsg, WPARAM wParam,
    LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
LRESULT CALLBACK buttonProc(HWND hWnd, UINT uMsg, WPARAM wParam,
    LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

LRESULT CALLBACK settings_wndproc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    char sb[512];
    static CHOOSECOLOR cc;
    static COLORREF custom[16] = { RGB(0,0,0) };

    switch (message) {
    case WM_INITDIALOG:
        SetWindowSubclass(GetDlgItem(hDlg, IDC_EDIT_ROWS), editProc, 0, 0);
        SetWindowSubclass(GetDlgItem(hDlg, IDC_EDIT_COLS), editProc, 0, 0);
        sprintf_s(sb, 512, "%d", view_get_rows());
        SetDlgItemText(hDlg, IDC_EDIT_ROWS, sb);
        sprintf_s(sb, 512, "%d", view_get_cols());
        SetDlgItemText(hDlg, IDC_EDIT_COLS, sb);
        SetWindowSubclass(GetDlgItem(hDlg, IDC_BUTTON_ALIVE), buttonProc, 0, 0);
        SetWindowSubclass(GetDlgItem(hDlg, IDC_BUTTON_DEAD), buttonProc, 1, 0);
        ZeroMemory(&cc, sizeof(CHOOSECOLOR));
        cc.lStructSize = sizeof(CHOOSECOLOR);
        cc.hwndOwner = hDlg;
        cc.lpCustColors = custom;
        cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            GetDlgItemText(hDlg, IDC_EDIT_ROWS, sb, 512);
            view_set_rows(atoi(sb));
            GetDlgItemText(hDlg, IDC_EDIT_COLS, sb, 512);
            view_set_cols(atoi(sb));
            EndDialog(hDlg, LOWORD(wParam));
            return TRUE;
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return TRUE;
        case IDC_EDIT_ROWS:
            switch (HIWORD(wParam)) {
            case EN_CHANGE:
                return TRUE;
                break;
            }
            break;
        case IDC_EDIT_COLS:
            switch (HIWORD(wParam)) {
            case EN_CHANGE:
                return TRUE;
                break;
            }
            break;
        case IDC_BUTTON_ALIVE:
            cc.rgbResult = RGB(view_get_alivec(0),
                               view_get_alivec(1),
                               view_get_alivec(2));
            ChooseColor(&cc);
            view_set_alivec(GetRValue(cc.rgbResult),
                            GetGValue(cc.rgbResult),
                            GetBValue(cc.rgbResult));
            break;
        case IDC_BUTTON_DEAD:
            cc.rgbResult = RGB(view_get_deadc(0),
                               view_get_deadc(1),
                               view_get_deadc(2));
            ChooseColor(&cc);
            view_set_deadc(GetRValue(cc.rgbResult),
                GetGValue(cc.rgbResult),
                GetBValue(cc.rgbResult));
            break;
        }
    }
    return FALSE;
}

/* subclassed edit control that only allows numbers and backspace */
LRESULT CALLBACK editProc(HWND hWnd, UINT uMsg, WPARAM wParam,
    LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    switch (uMsg) {
    case WM_CHAR:
        if ((wParam < 0x30 || wParam > 0x39) && wParam != VK_BACK
            && wParam != VK_LEFT && wParam != VK_RIGHT)
            return TRUE;
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

/* subclassed button control that draws itself */
LRESULT CALLBACK buttonProc(HWND hWnd, UINT uMsg, WPARAM wParam,
    LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    PAINTSTRUCT ps;
    HDC dc;
    RECT r;
    HBRUSH br;
    int res;
    switch (uMsg) {
    case WM_PAINT:
        dc = BeginPaint(hWnd, &ps);
        if(uIdSubclass==0)
            br = CreateSolidBrush(RGB(view_get_alivec(0),
                                      view_get_alivec(1),
                                      view_get_alivec(2)));
        else if(uIdSubclass==1)
            br = CreateSolidBrush(RGB(view_get_deadc(0),
                                      view_get_deadc(1),
                                      view_get_deadc(2)));
        GetClientRect(hWnd, &r);
        res = FillRect(dc, &r, br);
        DeleteObject(br);
        EndPaint(hWnd, &ps);
        break;
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}