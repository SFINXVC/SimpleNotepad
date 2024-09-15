#ifndef __UTILS_H
#define __UTILS_H

#include <Windows.h>

#include <png.h>

int DetectFileEncoding(const BYTE* buffer, DWORD bufferSize);
void OpenFileDialog(HWND hwnd, WCHAR* filter, WCHAR* initialDir, WCHAR** ppFileContents, WCHAR** ppFileName);
void SaveFileDialog(HWND hwnd, WCHAR* filter, WCHAR* initialDir, WCHAR* initialFileName, WCHAR** ppFileContents);

HBITMAP LoadPngFromResource(HINSTANCE hInstance, int nResId);
void DrawImage(HDC hdc, HBITMAP hBitmap, int x, int y, int width, int height);

#endif // __UTILS_H