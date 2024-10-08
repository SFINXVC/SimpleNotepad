#include "dlgabout.h"
#include "exception.h"

#include <libloaderapi.h>
#include <windef.h>
#include <wingdi.h>
#include <winuser.h>
#include <Windows.h>

#include "ids.h"
#include "utils.h"

static HBITMAP hImage1 = NULL;
static HBITMAP hImage2 = NULL;

LRESULT CALLBACK AboutWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
        {                
            DestroyWindow(hwnd);

            return 1;
        }
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            DrawImage(hdc, hImage1, 12, 12, 428, 103);
            DrawImage(hdc, hImage2, 12, 138, 50, 50);

            EndPaint(hwnd, &ps);
            return 1;
        }
        case WM_COMMAND:
        {
            if (LOWORD(wParam) == 1)
            {
                SendMessage(hwnd, WM_CLOSE, 0, 0);
                EndDialog(hwnd, 0);
            }

            return 1;
        }
        case WM_ERASEBKGND:
        {
            HDC hdc = (HDC)wParam;
            RECT rect;
            GetClientRect(hwnd, &rect);
            FillRect(hdc, &rect, CreateSolidBrush(RGB(0xF0, 0xF0, 0xF0)));
            return 1;
        }
        default:
            return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

HWND ShowAboutDialog(HINSTANCE hInstance, HWND parent)
{
    // get the parent size
    RECT parentRect;
    GetWindowRect(parent, &parentRect);

    // calculate windows start pos
    int x = parentRect.left + (parentRect.right - parentRect.left - 468) / 2;
    int y = parentRect.top + (parentRect.bottom - parentRect.top - 427) / 2;

    HWND hAboutWindow = CreateWindowExW(
        WS_EX_DLGMODALFRAME,
        L"#32770",
        L"About SimpleNotepad",
        WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        x, y, 468, 427,
        parent, NULL, hInstance, NULL
    );

    if (!hAboutWindow)
    {
        ShowLastError(L"Failed to show about dialog");
        return NULL;
    }

    /*
     * we're using "SetWindowLongPtrW" to set the window procedure.
     * This avoids recreating a new WNDCLASS on every fucking dialogs.
     * but however, using this method will make the WM_CREATE event not called
     * since WM_CREATE is called after the window creation, but the window
     * is alr created before signed to a new window procdure. So yeah
     * we need to initialize all of the controls here... :)
    */
    HFONT hFont = CreateFontW(
        -MulDiv(9, GetDeviceCaps(GetDC(hAboutWindow), LOGPIXELSY), 72),
        0, 0, 0, 
        FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
        CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        VARIABLE_PITCH, L"Segoe UI"
    );

    hImage1 = LoadPngFromResource(hInstance, ID_APP_BNR);
    hImage2 = LoadPngFromResource(hInstance, ID_APP_ICO_BIG);

    HWND hSeparator = CreateWindowW(L"STATIC", NULL, 
        WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ, 
        12, 125, 428, 2, 
        hAboutWindow, NULL, hInstance, NULL
    );

    HWND hText1 = CreateWindowExW(
        WS_EX_TRANSPARENT, L"STATIC", L"SimpleNotepad\n© 2024 SFINXV. All rights reserved.",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        68, 147, 189, 30,
        hAboutWindow, NULL, hInstance, NULL
    );

    HWND hText2 = CreateWindowExW(0, L"STATIC", L"SimpleNotepad is a lightweight, no-frills text editor, kinda like the classic Windows Notepad but with some extra features. It includes a dark mode to make late-night writing easier on the eyes. Simple and gets the job done.",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        68, 195, 335, 68, hAboutWindow, NULL, hInstance, NULL
    );

    HWND hOkBtn = CreateWindowExW(
        0, L"BUTTON", L"OK",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        350, 353, 90, 23,
        hAboutWindow, (HMENU)1, NULL, NULL
    );

    SendMessage(hText1, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hText2, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hOkBtn, WM_SETFONT, (WPARAM)hFont, TRUE);

    SetWindowLongPtrW(hAboutWindow, GWLP_WNDPROC, (LONG_PTR)AboutWndProc);

    return hAboutWindow;
}