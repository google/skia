/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include <windows.h>
#include <tchar.h>

#include "SkTypes.h"
#include "Timer.h"
#include "Window_win.h"
#include "../Application.h"

using sk_app::Application;

static char* tchar_to_utf8(const TCHAR* str) {
#ifdef _UNICODE
    int size = WideCharToMultiByte(CP_UTF8, 0, str, wcslen(str), NULL, 0, NULL, NULL);
    char* str8 = (char*)sk_malloc_throw(size + 1);
    WideCharToMultiByte(CP_UTF8, 0, str, wcslen(str), str8, size, NULL, NULL);
    str8[size] = '\0';
    return str8;
#else
    return _strdup(str);
#endif
}

static double now_ms() { return SkTime::GetNSecs() * 1e-6; }

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
    (void)exenameLen;
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

    Application* app = Application::Create(argc, argv, (void*)hInstance);

    MSG msg = { 0 };

    double currentTime = 0.0;
    double previousTime = 0.0;

    // Main message loop
    while (WM_QUIT != msg.message) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            previousTime = currentTime;
            currentTime = now_ms();
            app->onIdle(currentTime - previousTime);
        }
    }

    delete app;

    return (int)msg.wParam;
}
