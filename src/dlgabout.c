#include "dlgabout.h"
#include "exception.h"

#include <windef.h>
#include <wingdi.h>
#include <winuser.h>

#include "ids.h"

LRESULT CALLBACK AboutWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
        {
            DestroyWindow(hwnd);
            break;
        }
        case WM_COMMAND:
        {
            // Handle the "OK" button
            if (LOWORD(wParam) == 1)
            {
                SendMessage(hwnd, WM_CLOSE, 0, 0);\
                EndDialog(hwnd, 0);
            }

            break;
        }
        case WM_ERASEBKGND:
        {
            HDC hdc = (HDC)wParam;
            RECT rect;
            GetClientRect(hwnd, &rect);
            FillRect(hdc, &rect, CreateSolidBrush(RGB(0xF0, 0xF0, 0xF0))); // Fill background with #F0F0F0
            return 1;
            break;
        }
        default:
            return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

HWND ShowAboutDialog(HINSTANCE hInstance, HWND parent)
{
    // get the parent siz
    RECT parentRect;
    GetWindowRect(parent, &parentRect);

    // calculate windows start pos
    int x = parentRect.left + (parentRect.right - parentRect.left - 468) / 2;
    int y = parentRect.top + (parentRect.bottom - parentRect.top - 427) / 2;

    HWND hAboutWindow = CreateWindowExW(
        WS_EX_DLGMODALFRAME,
        L"#32770",
        L"About SimpleNotepad",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_MINIMIZEBOX,
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

    HWND hPicture1 = CreateWindowExW(0, L"STATIC", NULL, 
        WS_CHILD | WS_VISIBLE | SS_BITMAP,
        12, 12, 428, 103,
        hAboutWindow, NULL, hInstance, NULL
    );
    HWND hPicture2 = CreateWindowExW(0, L"STATIC", NULL, 
        WS_CHILD | WS_VISIBLE | SS_BITMAP,
        12, 138, 50, 50, 
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

    HBITMAP hBitmap1 = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(ID_APP_BNR), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);

    HBITMAP hBitmap2 = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(ID_APP_ICO_BIG), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
    
    if (!(hBitmap1 || hBitmap2))
        ShowLastError(L"Failed to load bitmap image");

    SendMessage(hPicture1, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap1);
    SendMessage(hPicture2, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap2);
    SendMessage(hPicture1, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap1);
    SendMessage(hPicture2, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap2);
    SendMessage(hText1, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hText2, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hOkBtn, WM_SETFONT, (WPARAM)hFont, TRUE);

    SetWindowLongPtrW(hAboutWindow, GWLP_WNDPROC, (LONG_PTR)AboutWndProc);

    return hAboutWindow;
}
