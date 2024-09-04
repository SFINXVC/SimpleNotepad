#include "exception.h"
#include <winuser.h>

void ShowLastError(const WCHAR* errorMessagePrefix)
{
    DWORD errorCode = GetLastError();
    LPWSTR errorMessage = NULL;

    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        errorCode,
        0,
        (LPWSTR)&errorMessage,
        0,
        NULL
    );

    HANDLE hHeap = GetProcessHeap();
    if (hHeap == NULL)
    {
        MessageBoxW(NULL, L"Failed to get process heap", L"Fatal Error", MB_OK | MB_ICONERROR);
        return;
    }

    if (errorMessage == NULL)
    {
        WCHAR* fullErrorMessage = (WCHAR*)HeapAlloc(hHeap, 0, (wcslen(errorMessagePrefix) + wcslen(L": unknown error") + 1) * sizeof(WCHAR));
        if (fullErrorMessage == NULL)
        {
            MessageBoxW(NULL, L"Failed to allocate memory for error message", L"Error", MB_OK | MB_ICONERROR);
            return;
        }
        wsprintfW(fullErrorMessage, L"%s: unknown error", errorMessagePrefix);
        MessageBoxW(NULL, fullErrorMessage, L"Error", MB_OK | MB_ICONERROR);
        HeapFree(hHeap, 0, fullErrorMessage);
        return;
    }

    WCHAR* fullErrorMessage = (WCHAR*)HeapAlloc(hHeap, 0, (wcslen(errorMessagePrefix) + wcslen(errorMessage) + 1) * sizeof(WCHAR));
    if (fullErrorMessage == NULL)
    {
        MessageBoxW(NULL, L"Failed to allocate memory for error message", L"Error", MB_OK | MB_ICONERROR);
        LocalFree(errorMessage);
        return;
    }
    
    wsprintfW(fullErrorMessage, L"%s: %s", errorMessagePrefix, errorMessage);
    MessageBoxW(NULL, fullErrorMessage, L"Error", MB_OK | MB_ICONERROR);
    HeapFree(hHeap, 0, fullErrorMessage);
    LocalFree(errorMessage);
}