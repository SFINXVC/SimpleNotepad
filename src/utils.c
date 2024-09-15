#include "utils.h"
#include "exception.h"

#include <Windows.h>
#include <commdlg.h>
#include <fileapi.h>
#include <handleapi.h>
#include <heapapi.h>
#include <libloaderapi.h>
#include <minwindef.h>
#include <stringapiset.h>
#include <wchar.h>
#include <windef.h>
#include <wingdi.h>
#include <winnt.h>
#include <winternl.h>
#include <Shlwapi.h>
#include <winuser.h>

#include <png.h>

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

void OpenFileDialog(HWND hwnd, WCHAR* filter, WCHAR* initialDir, WCHAR** ppFileContents, WCHAR** ppFileName)
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
    ofn.nMaxFile = MAX_PATH;
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

    *ppFileName = PathFindFileNameW(lpFilePath);

    // clean up
    HeapFree(GetProcessHeap(), 0, lpFileBuffer);  // Clean up buffer
    HeapFree(GetProcessHeap(), 0, lpFilePath);    // Clean up file path
}

void SaveFileDialog(HWND hwnd, WCHAR* filter, WCHAR* initialDir, WCHAR* initialFileName, WCHAR** ppFileContents)
{
    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));

    LPWSTR lpFilePath = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_PATH * sizeof(WCHAR));
    if (lpFilePath == NULL)
    {
        ShowLastError(L"Failed to allocate string for SaveFileDialog");
        return;
    }

    if (initialFileName != NULL && wcslen(initialFileName) < MAX_PATH)
        wcscpy_s(lpFilePath, MAX_PATH, initialFileName);

    ofn.lStructSize = sizeof(OPENFILENAMEW);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = lpFilePath;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.lpstrInitialDir = initialDir;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (!GetSaveFileNameW(&ofn))
    {
        HeapFree(GetProcessHeap(), 0, lpFilePath);
        return;
    }

    if (ppFileContents == NULL || *ppFileContents == NULL)
    {
        HeapFree(GetProcessHeap(), 0, lpFilePath);
        ShowLastError(L"No content to save");
        return;
    }

    // determine which encoding to use
    // by detecting if theres some freaky unicode characters
    BOOL isUnicode = FALSE;
    for (WCHAR* p = *ppFileContents; *p != L'\0'; p++)
    {
        if (*p > 127)
        {
            isUnicode = TRUE;
            break;
        }
    }

    HANDLE hFile = CreateFileW(lpFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        HeapFree(GetProcessHeap(), 0, lpFilePath);
        ShowLastError(L"Failed to create file");
        return;
    }

    DWORD dwBytesWritten = 0;

    if (isUnicode)
    {
        DWORD dwFileContentSize = (DWORD)(wcslen(*ppFileContents) * sizeof(WCHAR));
        if (!WriteFile(hFile, *ppFileContents, dwFileContentSize, &dwBytesWritten, NULL) || dwBytesWritten != dwFileContentSize)
        {
            CloseHandle(hFile);
            HeapFree(GetProcessHeap(), 0, lpFilePath);
            ShowLastError(L"Failed to write Unicode content to file");
            return;
        }
    }
    else
    {
        int utf8Size = WideCharToMultiByte(CP_UTF8, 0, *ppFileContents, -1, NULL, 0, NULL, NULL);
        if (utf8Size > 0)
        {
            LPSTR lpUtf8Content = (LPSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, utf8Size);
            if (lpUtf8Content)
            {
                WideCharToMultiByte(CP_UTF8, 0, *ppFileContents, -1, lpUtf8Content, utf8Size, NULL, NULL);
                if (!WriteFile(hFile, lpUtf8Content, utf8Size - 1, &dwBytesWritten, NULL) || dwBytesWritten != (DWORD)(utf8Size - 1))
                {
                    HeapFree(GetProcessHeap(), 0, lpUtf8Content);
                    CloseHandle(hFile);
                    HeapFree(GetProcessHeap(), 0, lpFilePath);
                    ShowLastError(L"Failed to write UTF-8 content to file");
                    return;
                }
                HeapFree(GetProcessHeap(), 0, lpUtf8Content);
            }
        }
    }

    // clean up
    CloseHandle(hFile);
    HeapFree(GetProcessHeap(), 0, lpFilePath);
}

HBITMAP LoadPngFromResource(HINSTANCE hInstance, HWND hwnd, int nResId)
{
    HRSRC hRes = FindResource(hInstance, MAKEINTRESOURCE(nResId), "PNG");
    if (hRes == NULL)
        return NULL;

    HGLOBAL hMem = LoadResource(hInstance, hRes);
    if (hMem == NULL)
        return NULL;

    DWORD dwPngSize = SizeofResource(hInstance, hRes);
    LPVOID lpPngData = LockResource(hMem);

    if (lpPngData == NULL)
        return NULL;

    png_image image;
    ZeroMemory(&image, sizeof(png_image));

    image.version = PNG_IMAGE_VERSION;

    if (png_image_begin_read_from_memory(&image, lpPngData, dwPngSize) == 0)
    {
        MessageBoxA(hwnd, "Failed to read PNG file from memory", "Error", MB_OK | MB_ICONERROR);
        return NULL;
    }

    image.format = PNG_FORMAT_RGBA;

    png_bytep buffer = (png_bytep)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, PNG_IMAGE_SIZE(image));
    if (buffer == NULL)
    {
        png_image_free(&image);
        MessageBoxA(hwnd, "Failed to allocate a buffer for reading PNG file!", "Error", MB_OK | MB_ICONERROR);
        return NULL;
    }

    if (png_image_finish_read(&image, NULL, buffer, 0, NULL) == 0)
    {
        png_image_free(&image);
        HeapFree(GetProcessHeap(), 0, buffer);
        MessageBoxA(hwnd, "Failed to parse PNG file!", "Error", MB_OK | MB_ICONERROR);
        return NULL;
    }

    HBITMAP hBitmap = CreateBitmap(image.width, image.height, 1, 32, buffer);

    HeapFree(GetProcessHeap(), 0, buffer);
    png_image_free(&image);

    return hBitmap;
}

void DrawImage(HDC hdc, HBITMAP hBitmap, int x, int y, int width, int height)
{
    if (hBitmap == NULL)
        return;

    HDC hdcMem = CreateCompatibleDC(hdc);
    HGDIOBJ oldBitmap = SelectObject(hdcMem, hBitmap);

    BITMAP bitmap;
    GetObject(hBitmap, sizeof(BITMAP), &bitmap);

    // calculate aspect ratio to fit the image to the current size
    FLOAT fRatio = (FLOAT)bitmap.bmWidth / bitmap.bmHeight;
    LONG lNewWidth = width;
    LONG lNewHeight = (LONG)(lNewWidth / fRatio);

    if (lNewHeight > height)
    {
        lNewHeight = height;
        lNewWidth = (LONG)(lNewHeight * fRatio);
    }

    LONG offsetX = (width - lNewWidth) / 2;
    LONG offsetY = (height - lNewHeight) / 2;

    StretchBlt(hdc, x, y, lNewWidth, lNewHeight, hdcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);

    SelectObject(hdcMem, oldBitmap);
    DeleteDC(hdcMem);
}