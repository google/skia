
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkWGL.h"

bool SkWGLExtensions::hasExtension(HDC dc, const char* ext) const {
    if (NULL == this->fGetExtensionsString) {
        return false;
    }
    if (!strcmp("WGL_ARB_extensions_string", ext)) {
        return true;
    }
    const char* extensionString = this->getExtensionsString(dc);
    int extLength = strlen(ext);

    while (true) {
        int n = strcspn(extensionString, " ");
        if (n == extLength && 0 == strncmp(ext, extensionString, n)) {
            return true;
        }
        if (0 == extensionString[n]) {
            return false;
        }
        extensionString += n+1;
    }

    return false;
}

const char* SkWGLExtensions::getExtensionsString(HDC hdc) const {
    return fGetExtensionsString(hdc);
}

BOOL SkWGLExtensions::choosePixelFormat(HDC hdc,
                                        const int* piAttribIList,
                                        const FLOAT* pfAttribFList,
                                        UINT nMaxFormats,
                                        int* piFormats,
                                        UINT* nNumFormats) const {
    return fChoosePixelFormat(hdc, piAttribIList, pfAttribFList,
                              nMaxFormats, piFormats, nNumFormats);
}

BOOL SkWGLExtensions::getPixelFormatAttribiv(HDC hdc,
                                             int iPixelFormat,
                                             int iLayerPlane,
                                             UINT nAttributes,
                                             const int *piAttributes,
                                             int *piValues) const {
    return fGetPixelFormatAttribiv(hdc, iPixelFormat, iLayerPlane,
                                   nAttributes, piAttributes, piValues);
}

BOOL SkWGLExtensions::getPixelFormatAttribfv(HDC hdc,
                                             int iPixelFormat,
                                             int iLayerPlane,
                                             UINT nAttributes,
                                             const int *piAttributes,
                                             float *pfValues) const {
    return fGetPixelFormatAttribfv(hdc, iPixelFormat, iLayerPlane,
                                   nAttributes, piAttributes, pfValues);
}
HGLRC SkWGLExtensions::createContextAttribs(HDC hDC,
                                            HGLRC hShareContext,
                                            const int *attribList) const {
    return fCreateContextAttribs(hDC, hShareContext, attribList);
}

namespace {

#if defined(UNICODE)
    #define STR_LIT(X) L## #X
#else
    #define STR_LIT(X) #X
#endif

#define DUMMY_CLASS STR_LIT("DummyClass")

HWND create_dummy_window() {
    HMODULE module = GetModuleHandle(NULL);
    HWND dummy;
    RECT windowRect;
    windowRect.left = 0;
    windowRect.right = 8;
    windowRect.top = 0;
    windowRect.bottom = 8;

    WNDCLASS wc;

    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = (WNDPROC) DefWindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = module;
    wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = DUMMY_CLASS;

    if(!RegisterClass(&wc)) {
        return 0;
    }

    DWORD style, exStyle;
    exStyle = WS_EX_CLIENTEDGE;
    style = WS_SYSMENU;

    AdjustWindowRectEx(&windowRect, style, false, exStyle);
    if(!(dummy = CreateWindowEx(exStyle,
                                DUMMY_CLASS,
                                STR_LIT("DummyWindow"),
                                WS_CLIPSIBLINGS | WS_CLIPCHILDREN | style,
                                0, 0,
                                windowRect.right-windowRect.left,
                                windowRect.bottom-windowRect.top,
                                NULL, NULL,
                                module,
                                NULL))) {
        UnregisterClass(DUMMY_CLASS, module);
        return NULL;
    }
    ShowWindow(dummy, SW_HIDE);

    return dummy;
}

void destroy_dummy_window(HWND dummy) {
    DestroyWindow(dummy);
    HMODULE module = GetModuleHandle(NULL);
    UnregisterClass(DUMMY_CLASS, module);
}
}

#define GET_PROC(NAME, SUFFIX) f##NAME = \
                     (##NAME##Proc) wglGetProcAddress("wgl" #NAME #SUFFIX)

SkWGLExtensions::SkWGLExtensions()
    : fGetExtensionsString(NULL)
    , fChoosePixelFormat(NULL)
    , fGetPixelFormatAttribfv(NULL)
    , fGetPixelFormatAttribiv(NULL)
    , fCreateContextAttribs(NULL) {
    HDC prevDC = wglGetCurrentDC();
    HGLRC prevGLRC = wglGetCurrentContext();

    PIXELFORMATDESCRIPTOR dummyPFD;

    ZeroMemory(&dummyPFD, sizeof(dummyPFD));
    dummyPFD.nSize = sizeof(dummyPFD);
    dummyPFD.nVersion = 1;
    dummyPFD.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
    dummyPFD.iPixelType = PFD_TYPE_RGBA;
    dummyPFD.cColorBits  = 32;
    dummyPFD.cDepthBits  = 0;
    dummyPFD.cStencilBits = 8;
    dummyPFD.iLayerType = PFD_MAIN_PLANE;
    HWND dummyWND = create_dummy_window();
    if (dummyWND) {
        HDC dummyDC = GetDC(dummyWND);
        int dummyFormat = ChoosePixelFormat(dummyDC, &dummyPFD);
        SetPixelFormat(dummyDC, dummyFormat, &dummyPFD);
        HGLRC dummyGLRC = wglCreateContext(dummyDC);
        SkASSERT(dummyGLRC);
        wglMakeCurrent(dummyDC, dummyGLRC);

        GET_PROC(GetExtensionsString, ARB);
        GET_PROC(ChoosePixelFormat, ARB);
        GET_PROC(GetPixelFormatAttribiv, ARB);
        GET_PROC(GetPixelFormatAttribfv, ARB);
        GET_PROC(CreateContextAttribs, ARB);

        wglMakeCurrent(dummyDC, NULL);
        wglDeleteContext(dummyGLRC);
        destroy_dummy_window(dummyWND);
    }

    wglMakeCurrent(prevDC, prevGLRC);
}
