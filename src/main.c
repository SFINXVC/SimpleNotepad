#include <Windows.h>
#include <commctrl.h>
#include <heapapi.h>
#include <shellapi.h>
#include <wchar.h>
#include <wingdi.h>
#include <winuser.h>
#include <Richedit.h>
#include <Uxtheme.h>
#include <dwmapi.h>
#include <CommCtrl.h>

#include "exception.h"
#include "dlgabout.h"
#include "config.h"
#include "utils.h"
#include "ids.h"

#define DEFAULT_WINDOW_WIDTH 1080
#define DEFAULT_WINDOW_HEIGHT 680

#define CLASS_NAMEW L"SimpleNotepad"
#define CLASS_NAME TEXT("SimpleNotepad")

static HWND ghWindow;
static HWND ghEdit;
static HWND ghStatusBar;

#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

LRESULT CALLBACK EditSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwrefData)
{
    switch (uMsg)
    {
        case WM_MOUSEWHEEL:
        {
            if (GetKeyState(VK_CONTROL) & 0x8000)
            {
                int nDelta = GET_WHEEL_DELTA_WPARAM(wParam);

                if (nDelta > 0)
                    SendMessage(ghWindow, WM_COMMAND, MAKEWPARAM(ID_VIEW_ZOOM_IN, 0), (LPARAM)hwnd);
                else if (nDelta < 0)
                    SendMessage(ghWindow, WM_COMMAND, MAKEWPARAM(ID_VIEW_ZOOM_OUT, 0), (LPARAM)hwnd);

                return 0;
            }

            break;
        }
    }

    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            InitCommonControls();
            break;
        }
        case WM_MOUSEWHEEL:
        {
            MessageBox(hwnd, "", "I Know and  u know", MB_OK | MB_ICONINFORMATION);
            break;
        }
        case WM_CREATE:
        {
            HMENU hMenu = CreateMenu();

            HMENU hFileMenu = CreatePopupMenu();
            AppendMenu(hFileMenu, MF_STRING | MF_ENABLED | MF_RIGHTJUSTIFY, ID_FILE_NEW, TEXT("New\tCtrl+N"));
            AppendMenu(hFileMenu, MF_STRING | MF_ENABLED | MF_RIGHTJUSTIFY, ID_FILE_OPEN, TEXT("Open\tCtrl+O"));
            AppendMenu(hFileMenu, MF_STRING | MF_ENABLED | MF_RIGHTJUSTIFY, ID_FILE_SAVE, TEXT("Save\tCtrl+S"));
            AppendMenu(hFileMenu, MF_STRING | MF_ENABLED | MF_RIGHTJUSTIFY, ID_FILE_SAVE_AS, TEXT("Save As\tCtrl+Shift+S"));
            AppendMenu(hFileMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(hFileMenu, MF_STRING | MF_ENABLED | MF_RIGHTJUSTIFY, ID_FILE_EXIT, TEXT("Exit\tAlt+F4"));

            HMENU hEditMenu = CreateMenu();
            AppendMenu(hEditMenu, MF_STRING | MF_ENABLED | MF_RIGHTJUSTIFY, ID_EDIT_UNDO, TEXT("Undo\tCtrl+Z"));
            AppendMenu(hEditMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(hEditMenu, MF_STRING | MF_ENABLED | MF_RIGHTJUSTIFY, ID_EDIT_CUT, TEXT("Cut\tCtrl+X"));
            AppendMenu(hEditMenu, MF_STRING | MF_ENABLED | MF_RIGHTJUSTIFY, ID_EDIT_COPY, TEXT("Copy\tCtrl+C"));
            AppendMenu(hEditMenu, MF_STRING | MF_ENABLED | MF_RIGHTJUSTIFY, ID_EDIT_PASTE, TEXT("Paste\tCtrl+V"));
            AppendMenu(hEditMenu, MF_STRING | MF_ENABLED | MF_RIGHTJUSTIFY, ID_EDIT_DELETE, TEXT("Delete\tDel"));
            AppendMenu(hEditMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(hEditMenu, MF_STRING | MF_DISABLED | MF_RIGHTJUSTIFY, ID_EDIT_GEMINI, TEXT("Summarize with Gemini...\tCtrl+E"));
            AppendMenu(hEditMenu, MF_STRING | MF_DISABLED | MF_RIGHTJUSTIFY, ID_EDIT_FIND, TEXT("Find...\tCtrl+F"));
            AppendMenu(hEditMenu, MF_STRING | MF_DISABLED | MF_RIGHTJUSTIFY, ID_EDIT_FIND_NEXT, TEXT("Find Next\tF3"));
            AppendMenu(hEditMenu, MF_STRING | MF_DISABLED | MF_RIGHTJUSTIFY, ID_EDIT_FIND_PREV, TEXT("Find Previous\tShift+F3"));
            AppendMenu(hEditMenu, MF_STRING | MF_DISABLED | MF_RIGHTJUSTIFY, ID_EDIT_REPLACE, TEXT("Replace...\tCtrl+H"));
            AppendMenu(hEditMenu, MF_STRING | MF_DISABLED | MF_RIGHTJUSTIFY, ID_EDIT_GOTO, TEXT("Go To...\tCtrl+G"));
            AppendMenu(hEditMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(hEditMenu, MF_STRING | MF_ENABLED | MF_RIGHTJUSTIFY, ID_EDIT_SELECT_ALL, TEXT("Select All\tCtrl+A"));
            AppendMenu(hEditMenu, MF_STRING | MF_DISABLED | MF_RIGHTJUSTIFY, ID_EDIT_TIMENDATE, TEXT("Time/Date\tF5"));
            
            HMENU hFormatMenu = CreateMenu();
            AppendMenu(hFormatMenu, MF_STRING | MF_ENABLED, ID_FORMAT_FONT, TEXT("Font..."));

            HMENU hVZoomMenu = CreatePopupMenu();
            AppendMenu(hVZoomMenu, MF_STRING | MF_ENABLED | MF_RIGHTJUSTIFY, ID_VIEW_ZOOM_IN, "Zoom In\tCtrl+Plus");
            AppendMenu(hVZoomMenu, MF_STRING | MF_ENABLED | MF_RIGHTJUSTIFY, ID_VIEW_ZOOM_OUT, "Zoom Out\tCtrl+Minus");
            AppendMenu(hVZoomMenu, MF_STRING | MF_ENABLED | MF_RIGHTJUSTIFY, ID_VIEW_ZOOM_RESET, "Restore Default Zoom\tCtrl+0");

            HMENU hViewMenu = CreateMenu();
            AppendMenu(hViewMenu, MF_STRING | MF_POPUP, (UINT_PTR)hVZoomMenu, "Zoom");

            HMENU hToolsMenu = CreateMenu();
            AppendMenu(hToolsMenu, MF_STRING | MF_ENABLED | MF_RIGHTJUSTIFY | MF_CHECKED, 0, TEXT("Enable Discord RPC"));
            AppendMenu(hToolsMenu, MF_STRING | MF_ENABLED | MF_RIGHTJUSTIFY | MF_CHECKED, 0, TEXT("Enable Word Warp"));
            AppendMenu(hToolsMenu, MF_STRING | MF_ENABLED | MF_RIGHTJUSTIFY | MF_CHECKED, 0, TEXT("Show Lines"));

            HMENU hAboutMenu = CreateMenu();
            AppendMenu(hAboutMenu, MF_STRING | MF_ENABLED | MF_RIGHTJUSTIFY, ID_HELP_REPO, TEXT("View Repository\tF1"));
            AppendMenu(hAboutMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(hAboutMenu, MF_STRING | MF_ENABLED | MF_RIGHTJUSTIFY, ID_HELP_ABOUT, TEXT("About\tF2"));

            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, TEXT("File"));
            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hEditMenu, TEXT("Edit"));
            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFormatMenu, TEXT("Format"));
            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hToolsMenu, TEXT("Tools"));
            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hViewMenu, TEXT("View"));
            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hAboutMenu, TEXT("Help"));
            
            SetMenu(hwnd, hMenu);

            ghEdit = CreateWindowExW(
                ES_EX_ZOOMABLE,
                MSFTEDIT_CLASS,
                NULL,
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_DISABLENOSCROLL,
                0, 0, 0, 0,
                hwnd, (HMENU)ID_CONTROL_EDIT, ((LPCREATESTRUCT)lParam)->hInstance, NULL
            );

            if (ghEdit == NULL)
                ShowLastError(L"Failed to create an EDIT control");

            SetWindowSubclass(ghEdit, EditSubclassProc, 0, 0);

            ghStatusBar = CreateWindowExW(0, STATUSCLASSNAMEW, NULL,
                WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0,
                hwnd, (HMENU)ID_CONTROL_STATUSBAR, ((LPCREATESTRUCT)lParam)->hInstance, NULL
            );

            if (ghStatusBar == NULL)
                ShowLastError(L"Failed to create an STATUSBAR control");

            int statwidths[] = { -1 };
            SendMessage(ghStatusBar, SB_SETPARTS, 1, (LPARAM)statwidths);
            SendMessage(ghStatusBar, SB_SETTEXT, 0, (LPARAM)TEXT("This status bar should display more information, but it hasn't been implemented yet."));


            HFONT hFont = CreateFontW(
                16, 0, 0, 0,
                FW_NORMAL, FALSE, FALSE, FALSE,
                ANSI_CHARSET,
                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY, FF_MODERN,
                L"Consolas"
            );

            SendMessage(ghEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

            // fancy tweaks for ghEdit, the goal is to make it 100% similar with original notepad
            SendMessage(ghEdit, EM_SETLIMITTEXT, 0, 0);
            SendMessage(ghEdit, EM_SETMARGINS, EC_LEFTMARGIN, 5);

            SetFocus(ghEdit);

            break;
        }
        case WM_SIZE:
        {
            RECT rcClient;
            GetClientRect(hwnd, &rcClient);

            RECT rcStatus;
            GetWindowRect(ghStatusBar, &rcStatus);
            int iStatusHeight = rcStatus.bottom - rcStatus.top;

            MoveWindow(ghEdit, 0, 0, rcClient.right, rcClient.bottom - iStatusHeight, TRUE);

            SendMessage(ghStatusBar, WM_SIZE, 0, 0);
            break;
        }
        case WM_SETFOCUS:
        {
            SetFocus(ghEdit);
        }
        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case ID_FILE_EXIT:
                {
                    DestroyWindow(ghWindow);
                    break;
                }
                case ID_FILE_OPEN:
                {
                    WCHAR* pFileContent = NULL;
                    WCHAR* pFileName = NULL;
                    OpenFileDialog(hwnd, L"Text Documents (*.txt)\0*.txt\0All Files\0*.*\0", L"", &pFileContent, &pFileName);

                    if (pFileContent != NULL)
                    {
                        SETTEXTEX stx;
                        ZeroMemory(&stx, sizeof(SETTEXTEX));

                        stx.flags = ST_DEFAULT;
                        stx.codepage = 1200;
                        SendMessage(ghEdit, EM_SETTEXTEX, (WPARAM)&stx, (LPARAM)pFileContent);
                        HeapFree(GetProcessHeap(), 0, pFileContent);
                    }

                    if (pFileName != NULL)
                    {
                        // Ubah title window
                        WCHAR title[256];
                        wsprintfW(title, L"%s - SimpleNotepad", pFileName);
                        SetWindowTextW(hwnd, title);
                    }

                    break;
                }
                case ID_FILE_SAVE:
                {
                    int textLength = SendMessage(ghEdit, WM_GETTEXTLENGTH, 0, 0);
                    WCHAR* fileContents = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, (SIZE_T)((textLength + 1) * sizeof(WCHAR)));
                    SendMessage(ghEdit, WM_GETTEXT, textLength + 1, (LPARAM)fileContents);

                    SaveFileDialog(hwnd, L"Text Documents (*.txt)\0*.txt\0All Files\0*.*\0", NULL, L"Untitled.txt", &fileContents);

                    HeapFree(GetProcessHeap(), 0, fileContents);
                    break;
                }
                case ID_FORMAT_FONT:
                {
                    CHOOSEFONTW cf;
                    LOGFONTW lf;
                    
                    ZeroMemory(&cf, sizeof(CHOOSEFONTW));
                    ZeroMemory(&lf, sizeof(LOGFONTW));

                    cf.lStructSize = sizeof(CHOOSEFONTW);
                    cf.hwndOwner = hwnd;
                    cf.lpLogFont = &lf;
                    cf.Flags = CF_SCREENFONTS;

                    HFONT hOldFont = (HFONT)SendMessage(ghEdit, WM_GETFONT, 0, 0);
                    
                    LOGFONTW lfCurrent;
                    GetObjectW(hOldFont, sizeof(LOGFONTW), &lfCurrent);
                    
                    DeleteObject(hOldFont);

                    if (ChooseFontW(&cf))
                    {
                        WCHAR* pFontName = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, (SIZE_T)((wcslen(lf.lfFaceName) + 1) * sizeof(WCHAR)));
                        wcscpy_s(pFontName, wcslen(lf.lfFaceName) + 1, lf.lfFaceName);

                        HFONT hFont = CreateFontIndirectW(&lf);
                        SendMessage(ghEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

                        // release the new font object after we done with it
                        // (not necessary in this case, but good practice)
                        DeleteObject(hFont);

                        HeapFree(GetProcessHeap(), 0, pFontName);
                    }

                    break;
                }
                case ID_HELP_REPO:
                {
                    int result = MessageBox(hwnd, "Go to the SimpleNotepad's GitHub repository?", "View Repository", MB_YESNO | MB_ICONQUESTION);
                    
                    if (result == IDYES)
                        ShellExecuteW(NULL, L"open", g_wszRepoLink, NULL, NULL, SW_SHOW);
                    
                    break;
                }
                case ID_HELP_ABOUT:
                {
                    HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);

                    HWND dlgabout = ShowAboutDialog(hInstance, hwnd);

                    if (dlgabout == NULL)
                        MessageBox(NULL, "Failed to show AboutDialogBox (handle returned NULL)", "Error", MB_OK | MB_ICONERROR);

                    ShowWindow(dlgabout, SW_SHOW);
                    break;
                }
                case ID_VIEW_ZOOM_IN:
                {
                    int num, denom;
                    SendMessage(ghEdit, EM_GETZOOM, (WPARAM)&num, (LPARAM)&denom);

                    if (num == 0 || denom == 0)
                    {
                        num = 1;
                        denom = 1;
                    }

                    if (num < 64)
                        num++;

                    SendMessage(ghEdit, EM_SETZOOM, num, denom);
                    break;
                }
                case ID_VIEW_ZOOM_OUT:
                {
                    int num, denom;
                    SendMessage(ghEdit, EM_GETZOOM, (WPARAM)&num, (LPARAM)&denom);

                    if (num == 0 || denom == 0)
                    {
                        num = 1;
                        denom = 1;
                    }

                    if (num > 1)
                        num--;

                    SendMessage(ghEdit, EM_SETZOOM, num, denom);
                    break;
                }
                case ID_VIEW_ZOOM_RESET:
                {
                    SendMessage(ghEdit, EM_SETZOOM, 0, 0);
                    break;
                }
            }

            break;
        }
        case WM_CLOSE:
        {
            DestroyWindow(hwnd);
            break;
        }
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            break;
        }
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icc.dwICC = ICC_WIN95_CLASSES | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icc);

    WNDCLASSEX wc;
    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(ID_APP_ICO), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
    wc.hIconSm = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(ID_APP_ICO), IMAGE_ICON, 16, 16, LR_SHARED);

    if (!RegisterClassEx(&wc))
    {
        ShowLastError(L"Failed to register a class");
        return -1;
    }

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    HMODULE hModule = LoadLibraryW(L"msftedit.dll");
    ghWindow = CreateWindowExW(
        0,
        CLASS_NAMEW,
        L"Simple Notepad",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        (screenWidth - DEFAULT_WINDOW_WIDTH) / 2, (screenHeight - DEFAULT_WINDOW_HEIGHT) / 2,
        DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, 
        NULL, NULL, hInstance, NULL
    );

    if (ghWindow == NULL)
    {
        ShowLastError(L"Failed to create a window");
        return -1;
    }

    // accelerator table - a better way to handle shortcuts
    ACCEL accel[] = {
        { FVIRTKEY | FCONTROL, 'O', ID_FILE_OPEN }, // CTRL + O
        { FVIRTKEY | FCONTROL, 'S', ID_FILE_SAVE }, // CTRL + S
        { FVIRTKEY | FCONTROL, VK_OEM_PLUS, ID_VIEW_ZOOM_IN }, // CTRL + +
        { FVIRTKEY | FCONTROL, VK_OEM_MINUS, ID_VIEW_ZOOM_OUT }, // CTRL + -
        { FVIRTKEY | FCONTROL, '0', ID_VIEW_ZOOM_RESET }, // CTRL + 0
    };


    HACCEL hAccel = CreateAcceleratorTable(accel, sizeof(accel) / sizeof(ACCEL));

    ShowWindow(ghWindow, nCmdShow);
    UpdateWindow(ghWindow);

    BOOL bRet;
    MSG msg;
    while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
    {
        if (bRet == -1)
        {
            ShowLastError(L"GetMessage failed");
            return -1;
        }

        if (!TranslateAccelerator(ghWindow, hAccel, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return msg.wParam; 
}