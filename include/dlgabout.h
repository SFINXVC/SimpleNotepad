#ifndef __DLGABOUT_H
#define __DLGABOUT_H

#include <Windows.h>

LRESULT CALLBACK AboutWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HWND ShowAboutDialog(HINSTANCE hInstance, HWND parent);

#endif // __DLGABOUT_H