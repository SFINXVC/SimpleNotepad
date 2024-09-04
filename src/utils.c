#include "utils.h"
#include "exception.h"
#include <winuser.h>

void OpenFileDialog(HWND hwnd, WCHAR* filter, WCHAR* initialDir, WCHAR** ppFileContents)
{
    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(OPENFILENAMEW));

    WCHAR* szFile = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, 1024 * sizeof(WCHAR));
    if (szFile == NULL)
    {
        ShowLastError(L"Failed to allocate string for OpenFileDialog");
        return;
    }

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = 1024;
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = initialDir;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameW(&ofn) == TRUE)
    {
        HANDLE hFile = CreateFileW(ofn.lpstrFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
        
        if (hFile != INVALID_HANDLE_VALUE)
        {
            DWORD dwFileSize = GetFileSize(hFile, NULL);
            BYTE* pBuffer = (BYTE*)HeapAlloc(GetProcessHeap(), 0, dwFileSize + 2);
            
            if (pBuffer == NULL)
            {
                CloseHandle(hFile);
                HeapFree(GetProcessHeap(), 0, szFile);
                return;
            }

            DWORD dwBytesRead = 0;
            ReadFile(hFile, pBuffer, dwFileSize, &dwBytesRead, NULL);

            if (dwFileSize >= 2 && pBuffer[0] == 0xFF && pBuffer[1] == 0xFE)
            {
                WCHAR* pUnicodeBuffer = (WCHAR*)(pBuffer + 2);
                DWORD dwUnicodeBufferSize = (dwFileSize - 2) / sizeof(WCHAR);

                pUnicodeBuffer[dwUnicodeBufferSize] = '\0';

                *ppFileContents = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, (dwUnicodeBufferSize + 1) * sizeof(WCHAR));
                wcsncpy_s(*ppFileContents, dwUnicodeBufferSize + 1, pUnicodeBuffer, _TRUNCATE);
            }
            else
            {
                MessageBox(NULL, "Unsupported file encoding. Only UTF-16LE were supported at the moment!", "Error!", MB_OK | MB_ICONERROR);
                return;
            }

            HeapFree(GetProcessHeap(), 0, pBuffer);
            CloseHandle(hFile);
        }
    }

    HeapFree(GetProcessHeap(), 0, szFile);
}

void SaveFileDialog(HWND hwnd, WCHAR* filter, WCHAR* initialDir, WCHAR* initialFileName, WCHAR** ppFileContents)
{
    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(OPENFILENAMEW));

    WCHAR* szFile = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, 1024 * sizeof(WCHAR));
    if (szFile == NULL)
    {
        ShowLastError(L"Failed to allocate string for SaveFileDialog");
        return;
    }

    if (initialFileName != NULL)
    {
        wcsncpy_s(szFile, 1024, initialFileName, _TRUNCATE);
    }

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = 1024;
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = initialDir;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetSaveFileNameW(&ofn) == TRUE)
    {
        HANDLE hFile = CreateFileW(ofn.lpstrFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_WRITE_THROUGH, NULL);

        if (hFile != INVALID_HANDLE_VALUE)
        {
            DWORD dwBytesWritten = 0;
            BYTE bom[2] = { 0xFF, 0xFE };
            WriteFile(hFile, bom, 2, &dwBytesWritten, NULL);
            WriteFile(hFile, *ppFileContents, wcslen(*ppFileContents) * sizeof(WCHAR), &dwBytesWritten, NULL);

            CloseHandle(hFile);
        }
    }

    HeapFree(GetProcessHeap(), 0, szFile);
}