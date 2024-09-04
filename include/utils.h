#ifndef __UTILS_H
#define __UTILS_H

#include <Windows.h>

void OpenFileDialog(HWND hwnd, WCHAR* filter, WCHAR* initialDir, WCHAR** ppFileContents);
void SaveFileDialog(HWND hwnd, WCHAR* filter, WCHAR* initialDir, WCHAR* initialFileName, WCHAR** ppFileContents);

#endif // __UTILS_H