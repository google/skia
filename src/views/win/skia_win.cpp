/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <windows.h>
#include <tchar.h>

#include "SkTypes.h"
#include "SkApplication.h"
#include "SkOSWindow_Win.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Returns the main window Win32 class name.
static const TCHAR* register_class(HINSTANCE hInstance) {
    WNDCLASSEX wcex;
    // The main window class name
    static const TCHAR gSZWindowClass[] = _T("SkiaApp");

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = NULL;
    wcex.hCursor        = NULL;
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = gSZWindowClass;
    wcex.hIconSm        = NULL;

    RegisterClassEx(&wcex);

    return gSZWindowClass;
}

static char* tchar_to_utf8(const TCHAR* str) {
#ifdef _UNICODE
    int size = WideCharToMultiByte(CP_UTF8, 0, str, wcslen(str), NULL, 0, NULL, NULL);
    char* str8 = (char*) sk_malloc_throw(size+1);
    WideCharToMultiByte(CP_UTF8, 0, str, wcslen(str), str8, size, NULL, NULL);
    str8[size] = '\0';
    return str8;
#else
    return _strdup(str);
#endif
}

// This file can work with GUI or CONSOLE subsystem types since we define _tWinMain and main().

static int main_common(HINSTANCE hInstance, int show, int argc, char**argv);

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine,
                       int nCmdShow) {

    // convert from lpCmdLine to argc, argv.
    char* argv[4096];
    int argc = 0;
    TCHAR exename[1024], *next;
    int exenameLen = GetModuleFileName(NULL, exename, SK_ARRAY_COUNT(exename));
    // we're ignoring the possibility that the exe name exceeds the exename buffer
    (void) exenameLen;
    argv[argc++] = tchar_to_utf8(exename);
    TCHAR* arg = _tcstok_s(lpCmdLine, _T(" "), &next);
    while (arg != NULL) {
       argv[argc++] = tchar_to_utf8(arg);
       arg = _tcstok_s(NULL, _T(" "), &next);
    }
    int result = main_common(hInstance, nCmdShow, argc, argv);
    for (int i = 0; i < argc; ++i) {
       sk_free(argv[i]);
    }
    return result;
}

int main(int argc, char**argv) {
    return main_common(GetModuleHandle(NULL), SW_SHOW, argc, argv);
}

static int main_common(HINSTANCE hInstance, int show, int argc, char**argv) {
    const TCHAR* windowClass = register_class(hInstance);

    application_init();

    SkOSWindow::WindowInit winInit;
    winInit.fInstance = hInstance;
    winInit.fClass = windowClass;

    create_sk_window(&winInit, argc, argv);
    SkOSWindow::ForAllWindows([show](void* hWnd, SkOSWindow**) {
        ShowWindow((HWND)hWnd, show);
        UpdateWindow((HWND)hWnd); }
    );

    MSG msg;
    // Main message loop
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (true) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    application_term();

    return (int) msg.wParam;
}

extern SkOSWindow* create_sk_window(void* hwnd, int argc, char** argv);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_COMMAND:
            return DefWindowProc(hWnd, message, wParam, lParam);
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default: {
            SkOSWindow* window = SkOSWindow::GetOSWindowForHWND(hWnd);
            if (window && window->wndProc(hWnd, message, wParam, lParam)) {
                return 0;
            } else {
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
    }
    return 0;
}
