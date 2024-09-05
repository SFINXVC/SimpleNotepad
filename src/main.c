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

#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    // CTRL & A handle 
    // (unecessarry since we're using a RichEdit instead of the traditional EDIT ctrl class)
    // (it's just not rlly needed since it's alr handled)
    /*if (nCode == HC_ACTION && wParam == 0x41 && GetAsyncKeyState(VK_CONTROL) < 0)
    {
        SendMessage(ghEdit, EM_SETSEL, 0, -1);
        return 1;
    }*/

    if (nCode == HC_ACTION && wParam == 0x4F && GetAsyncKeyState(VK_CONTROL) < 0)
    {
        if (GetAsyncKeyState(0x4F) >= 0)
            return CallNextHookEx(NULL, nCode, wParam, lParam);

        PostMessage(ghWindow, WM_COMMAND, ID_FILE_OPEN, 0);
        return 1;
    }

    if (nCode == HC_ACTION && wParam == 0x53 && GetAsyncKeyState(VK_CONTROL) < 0)
    {
        if (GetAsyncKeyState(0x53) >= 0)
            return CallNextHookEx(NULL, nCode, wParam, lParam);

        PostMessage(ghWindow, WM_COMMAND, ID_FILE_SAVE, 0);
        return 1;
    }

    if (nCode == HC_ACTION && wParam == 0x2B && GetAsyncKeyState(VK_CONTROL) < 0)
    {
        int zoomLevel = SendMessage(ghEdit, EM_GETZOOM, 0, 0);
        zoomLevel += 10;

        SendMessage(ghEdit, EM_SETZOOM, zoomLevel, 0);
        return 1;
    }

    if (nCode == HC_ACTION && wParam == 0x2D && GetAsyncKeyState(VK_CONTROL) < 0)
    {
        int zoomLevel = SendMessage(ghEdit, EM_GETZOOM, 0, 0);

        zoomLevel -= 10;

        if (zoomLevel < 50) 
            zoomLevel = 50;

        SendMessage(ghEdit, EM_SETZOOM, zoomLevel, 0);
        return 1;
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
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

            HMENU hAboutMenu = CreateMenu();
            AppendMenu(hAboutMenu, MF_STRING | MF_ENABLED | MF_RIGHTJUSTIFY, ID_HELP_REPO, TEXT("View Repository\tF1"));
            AppendMenu(hAboutMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(hAboutMenu, MF_STRING | MF_ENABLED | MF_RIGHTJUSTIFY, ID_HELP_ABOUT, TEXT("About\tF2"));

            HMENU hFormatMenu = CreateMenu();
            AppendMenu(hFormatMenu, MF_STRING | MF_ENABLED, ID_FORMAT_FONT, TEXT("Font..."));

            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, TEXT("File"));
            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFormatMenu, TEXT("Format"));
            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hAboutMenu, TEXT("Help"));
            
            SetMenu(hwnd, hMenu);

            ghEdit = CreateWindowExW(
                0,
                MSFTEDIT_CLASS,
                NULL,
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
                0, 0, 0, 0,
                hwnd, (HMENU)ID_CONTROL_EDIT, ((LPCREATESTRUCT)lParam)->hInstance, NULL
            );

            if (ghEdit == NULL)
                ShowLastError(L"Failed to create an EDIT window");

            SetWindowsHookEx(WH_KEYBOARD, KeyboardProc, NULL, GetCurrentThreadId());

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
            MoveWindow(ghEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
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
                    OpenFileDialog(hwnd, L"All Files\0*.*\0Text Documents (*.txt)\0*.txt\0", L"", &pFileContent);
                    
                    if (pFileContent != NULL)
                    {
                        SendMessage(ghEdit, WM_SETTEXT, 0, (LPARAM)pFileContent);
                        HeapFree(GetProcessHeap(), 0, pFileContent);
                    }
                    break;
                }
                case ID_FILE_SAVE:
                {
                    int textLength = SendMessage(ghEdit, WM_GETTEXTLENGTH, 0, 0);
                    WCHAR* fileContents = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, (SIZE_T)((textLength + 1) * sizeof(WCHAR)));
                    SendMessage(ghEdit, WM_GETTEXT, textLength + 1, (LPARAM)fileContents);

                    SaveFileDialog(hwnd, L"All Files\0*.*\0Text Documents (*.txt)\0*.txt\0", NULL, L"example.txt", &fileContents);

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
                        ShellExecuteW(NULL, L"open", sgwRepoLink, NULL, NULL, SW_SHOW);
                    
                    break;
                }
                case ID_HELP_ABOUT:
                {
                    HWND dlgabout = ShowAboutDialog(NULL, hwnd);

                    if (dlgabout == NULL)
                        MessageBox(NULL, "Failed to show AboutDialogBox (handle returned NULL)", "Error", MB_OK | MB_ICONERROR);

                    ShowWindow(dlgabout, SW_SHOW);
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

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam; 
}