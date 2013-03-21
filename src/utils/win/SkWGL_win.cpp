
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkWGL.h"

#include "SkTDArray.h"
#include "SkTSearch.h"

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

struct PixelFormat {
    int fFormat;
    int fCoverageSamples;
    int fColorSamples;
    int fChoosePixelFormatRank;
};

int compare_pf(const PixelFormat* a, const PixelFormat* b) {
    if (a->fCoverageSamples < b->fCoverageSamples) {
        return -1;
    } else if (b->fCoverageSamples < a->fCoverageSamples) {
        return 1;
    } else if (a->fColorSamples < b->fColorSamples) {
        return -1;
    } else if (b->fColorSamples < a->fColorSamples) {
        return 1;
    } else if (a->fChoosePixelFormatRank < b->fChoosePixelFormatRank) {
        return -1;
    } else if (b->fChoosePixelFormatRank < a->fChoosePixelFormatRank) {
        return 1;
    }
    return 0;
}
}

int SkWGLExtensions::selectFormat(const int formats[],
                                  int formatCount,
                                  HDC dc,
                                  int desiredSampleCount) {
    PixelFormat desiredFormat = {
        0,
        desiredSampleCount,
        0,
        0,
    };
    SkTDArray<PixelFormat> rankedFormats;
    rankedFormats.setCount(formatCount);
    bool supportsCoverage = this->hasExtension(dc,
                                               "WGL_NV_multisample_coverage");
    for (int i = 0; i < formatCount; ++i) {
        static const int queryAttrs[] = {
            SK_WGL_COVERAGE_SAMPLES,
            // Keep COLOR_SAMPLES at the end so it can be skipped
            SK_WGL_COLOR_SAMPLES,
        };
        int answers[2];
        int queryAttrCnt = supportsCoverage ?
                                    SK_ARRAY_COUNT(queryAttrs) :
                                    SK_ARRAY_COUNT(queryAttrs) - 1;
        this->getPixelFormatAttribiv(dc,
                                     formats[i],
                                     0,
                                     queryAttrCnt,
                                     queryAttrs,
                                     answers);
        rankedFormats[i].fFormat =  formats[i];
        rankedFormats[i].fCoverageSamples = answers[0];
        rankedFormats[i].fColorSamples = answers[supportsCoverage ? 1 : 0];
        rankedFormats[i].fChoosePixelFormatRank = i;
    }
    qsort(rankedFormats.begin(),
            rankedFormats.count(),
            sizeof(PixelFormat),
            SkCastForQSort(compare_pf));
    int idx = SkTSearch<PixelFormat>(rankedFormats.begin(),
                                     rankedFormats.count(),
                                     desiredFormat,
                                     sizeof(PixelFormat),
                                     compare_pf);
    if (idx < 0) {
        idx = ~idx;
    }
    return rankedFormats[idx].fFormat;
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

HGLRC SkCreateWGLContext(HDC dc, int msaaSampleCount, bool preferCoreProfile) {
    SkWGLExtensions extensions;
    if (!extensions.hasExtension(dc, "WGL_ARB_pixel_format")) {
        return NULL;
    }

    HDC prevDC = wglGetCurrentDC();
    HGLRC prevGLRC = wglGetCurrentContext();
    PIXELFORMATDESCRIPTOR pfd;

    int format = 0;

    static const int iAttrs[] = {
        SK_WGL_DRAW_TO_WINDOW, TRUE,
        SK_WGL_DOUBLE_BUFFER, TRUE,
        SK_WGL_ACCELERATION, SK_WGL_FULL_ACCELERATION,
        SK_WGL_SUPPORT_OPENGL, TRUE,
        SK_WGL_COLOR_BITS, 24,
        SK_WGL_ALPHA_BITS, 8,
        SK_WGL_STENCIL_BITS, 8,
        0, 0
    };

    float fAttrs[] = {0, 0};

    if (msaaSampleCount > 0 &&
        extensions.hasExtension(dc, "WGL_ARB_multisample")) {
        static const int kIAttrsCount = SK_ARRAY_COUNT(iAttrs);
        int msaaIAttrs[kIAttrsCount + 6];
        memcpy(msaaIAttrs, iAttrs, sizeof(int) * kIAttrsCount);
        SkASSERT(0 == msaaIAttrs[kIAttrsCount - 2] &&
                 0 == msaaIAttrs[kIAttrsCount - 1]);
        msaaIAttrs[kIAttrsCount - 2] = SK_WGL_SAMPLE_BUFFERS;
        msaaIAttrs[kIAttrsCount - 1] = TRUE;
        msaaIAttrs[kIAttrsCount + 0] = SK_WGL_SAMPLES;
        msaaIAttrs[kIAttrsCount + 1] = msaaSampleCount;
        if (extensions.hasExtension(dc, "WGL_NV_multisample_coverage")) {
            msaaIAttrs[kIAttrsCount + 2] = SK_WGL_COLOR_SAMPLES;
            // We want the fewest number of color samples possible.
            // Passing 0 gives only the formats where all samples are color
            // samples.
            msaaIAttrs[kIAttrsCount + 3] = 1;
            msaaIAttrs[kIAttrsCount + 4] = 0;
            msaaIAttrs[kIAttrsCount + 5] = 0;
        } else {
            msaaIAttrs[kIAttrsCount + 2] = 0;
            msaaIAttrs[kIAttrsCount + 3] = 0;
        }
        unsigned int num;
        int formats[64];
        extensions.choosePixelFormat(dc, msaaIAttrs, fAttrs, 64, formats, &num);
        num = min(num,64);
        int formatToTry = extensions.selectFormat(formats,
                                                  num,
                                                  dc,
                                                  msaaSampleCount);
        DescribePixelFormat(dc, formatToTry, sizeof(pfd), &pfd);
        if (SetPixelFormat(dc, formatToTry, &pfd)) {
            format = formatToTry;
        }
    }

    if (0 == format) {
        // Either MSAA wasn't requested or creation failed
        unsigned int num;
        extensions.choosePixelFormat(dc, iAttrs, fAttrs, 1, &format, &num);
        DescribePixelFormat(dc, format, sizeof(pfd), &pfd);
        SkDEBUGCODE(BOOL set =) SetPixelFormat(dc, format, &pfd);
        SkASSERT(TRUE == set);
    }

    HGLRC glrc = NULL;
    if (preferCoreProfile && extensions.hasExtension(dc, "WGL_ARB_create_context")) {
        static const int kCoreGLVersions[] = {
            4, 3,
            4, 2,
            4, 1,
            4, 0,
            3, 3,
            3, 2,
        };
        int coreProfileAttribs[] = {
            SK_WGL_CONTEXT_MAJOR_VERSION, -1,
            SK_WGL_CONTEXT_MINOR_VERSION, -1,
            SK_WGL_CONTEXT_PROFILE_MASK,  SK_WGL_CONTEXT_CORE_PROFILE_BIT,
            0,
        };
        for (int v = 0; v < SK_ARRAY_COUNT(kCoreGLVersions) / 2; ++v) {
            coreProfileAttribs[1] = kCoreGLVersions[2 * v];
            coreProfileAttribs[3] = kCoreGLVersions[2 * v + 1];
            glrc = extensions.createContextAttribs(dc, NULL, coreProfileAttribs);
            if (NULL != glrc) {
                break;
            }
        }
    }

    if (NULL == glrc) {
        glrc = wglCreateContext(dc);
    }
    SkASSERT(glrc);

    wglMakeCurrent(prevDC, prevGLRC);
    return glrc;
}
