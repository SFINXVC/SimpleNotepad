#include "utils.h"
#include "exception.h"

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
            WCHAR* pBuffer = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, (dwFileSize + 1) * sizeof(WCHAR));
            
            if (pBuffer == NULL)
            {
                CloseHandle(hFile);
                HeapFree(GetProcessHeap(), 0, szFile);
                return;
            }

            DWORD dwBytesRead = 0;
            ReadFile(hFile, pBuffer, dwFileSize, &dwBytesRead, NULL);
            pBuffer[dwFileSize] = '\0';
            
            *ppFileContents = pBuffer;

            CloseHandle(hFile);
        }
    }

    HeapFree(GetProcessHeap(), 0, szFile);
}