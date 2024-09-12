#include "utils.h"
#include "exception.h"

#include <Windows.h>
#include <commdlg.h>
#include <fileapi.h>
#include <handleapi.h>
#include <heapapi.h>
#include <stringapiset.h>
#include <winnt.h>
#include <winternl.h>

int DetectFileEncoding(const BYTE* buffer, DWORD bufferSize)
{
    if (bufferSize >= 2)
    {
        const BYTE utf16le_bom[] = { 0xFF, 0xFE };
        const BYTE utf16be_bom[] = { 0xFE, 0xFF };
        const BYTE utf8_bom[] = { 0xEF, 0xBB, 0xBF };

        if (buffer[0] == 0xFF && buffer[1] == 0xFE) // UTF-16LE
            return 1200;
        if (buffer[0] == 0xFE && buffer[1] == 0xFF) // UTF-16BE
            return 1201;
        if ( // UTF-8
            bufferSize >= 3 &&
            buffer[0] == 0xEF &&
            buffer[1] == 0xBB &&
            buffer[2] == 0xBF
        )
            return CP_UTF8;
    }

    return CP_ACP;
}

void OpenFileDialog(HWND hwnd, WCHAR* filter, WCHAR* initialDir, WCHAR** ppFileContents)
{
    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));

    LPWSTR lpFilePath = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_PATH * sizeof(WCHAR));
    if (lpFilePath == NULL)
    {
        ShowLastError(L"Failed to allocate string for OpenFileDialog");
        return;
    }

    ofn.lStructSize = sizeof(OPENFILENAMEW);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = lpFilePath;
    ofn.nMaxFile = MAX_PATH;  // FIXED: Jangan kali dengan sizeof(WCHAR)
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.lpstrInitialDir = initialDir;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (!GetOpenFileNameW(&ofn))
    {
        HeapFree(GetProcessHeap(), 0, lpFilePath);
        return;
    }

    HANDLE hFile = CreateFileW(ofn.lpstrFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        HeapFree(GetProcessHeap(), 0, lpFilePath);
        ShowLastError(L"Failed to open file");
        return;
    }

    DWORD dwFileSize = GetFileSize(hFile, NULL);
    if (dwFileSize == INVALID_FILE_SIZE)
    {
        CloseHandle(hFile);
        HeapFree(GetProcessHeap(), 0, lpFilePath);
        ShowLastError(L"Failed to get file size");
        return;
    }

    BYTE* lpFileBuffer = (BYTE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwFileSize);
    if (!lpFileBuffer)
    {
        CloseHandle(hFile);
        HeapFree(GetProcessHeap(), 0, lpFilePath);
        ShowLastError(L"Failed to allocate buffer to load file contents");
        return;
    }

    DWORD dwBytesRead;
    if (!ReadFile(hFile, lpFileBuffer, dwFileSize, &dwBytesRead, NULL) || dwBytesRead != dwFileSize)
    {
        HeapFree(GetProcessHeap(), 0, lpFileBuffer);
        CloseHandle(hFile);
        HeapFree(GetProcessHeap(), 0, lpFilePath);
        ShowLastError(L"Failed to read file");
        return;
    }

    CloseHandle(hFile);

    // detect encoding
    int nEncoding = DetectFileEncoding(lpFileBuffer, dwBytesRead);

    // convert to wide string (LPWSTR) based on the encoding
    int nWideCharCount = MultiByteToWideChar(nEncoding, 0, (LPCCH)lpFileBuffer, dwBytesRead, NULL, 0);
    if (nWideCharCount > 0)
    {
        *ppFileContents = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (nWideCharCount + 1) * sizeof(WCHAR));

        if (*ppFileContents)
        {
            MultiByteToWideChar(nEncoding, 0, (LPCCH)lpFileBuffer, dwBytesRead, *ppFileContents, nWideCharCount);
            (*ppFileContents)[nWideCharCount] = L'\0';  // null terminator (just to make sure)
        }
    }

    // clean up
    HeapFree(GetProcessHeap(), 0, lpFileBuffer);  // Clean up buffer
    HeapFree(GetProcessHeap(), 0, lpFilePath);    // Clean up file path
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