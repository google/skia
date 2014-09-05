
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkWGL.h"

#include "SkTDArray.h"
#include "SkTSearch.h"
#include "SkTSort.h"

bool SkWGLExtensions::hasExtension(HDC dc, const char* ext) const {
    if (NULL == this->fGetExtensionsString) {
        return false;
    }
    if (!strcmp("WGL_ARB_extensions_string", ext)) {
        return true;
    }
    const char* extensionString = this->getExtensionsString(dc);
    size_t extLength = strlen(ext);

    while (true) {
        size_t n = strcspn(extensionString, " ");
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

BOOL SkWGLExtensions::swapInterval(int interval) const {
    return fSwapInterval(interval);
}

HPBUFFER SkWGLExtensions::createPbuffer(HDC hDC,
                                        int iPixelFormat,
                                        int iWidth,
                                        int iHeight,
                                        const int *piAttribList) const {
    return fCreatePbuffer(hDC, iPixelFormat, iWidth, iHeight, piAttribList);
}

HDC SkWGLExtensions::getPbufferDC(HPBUFFER hPbuffer) const {
    return fGetPbufferDC(hPbuffer);
}

int SkWGLExtensions::releasePbufferDC(HPBUFFER hPbuffer, HDC hDC) const {
    return fReleasePbufferDC(hPbuffer, hDC);
}

BOOL SkWGLExtensions::destroyPbuffer(HPBUFFER hPbuffer) const {
    return fDestroyPbuffer(hPbuffer);
}

namespace {

struct PixelFormat {
    int fFormat;
    int fSampleCnt;
    int fChoosePixelFormatRank;
};

bool pf_less(const PixelFormat& a, const PixelFormat& b) {
    if (a.fSampleCnt < b.fSampleCnt) {
        return true;
    } else if (b.fSampleCnt < a.fSampleCnt) {
        return false;
    } else if (a.fChoosePixelFormatRank < b.fChoosePixelFormatRank) {
        return true;
    }
    return false;
}
}

int SkWGLExtensions::selectFormat(const int formats[],
                                  int formatCount,
                                  HDC dc,
                                  int desiredSampleCount) const {
    PixelFormat desiredFormat = {
        0,
        desiredSampleCount,
        0,
    };
    SkTDArray<PixelFormat> rankedFormats;
    rankedFormats.setCount(formatCount);
    for (int i = 0; i < formatCount; ++i) {
        static const int kQueryAttr = SK_WGL_SAMPLES;
        int numSamples;
        this->getPixelFormatAttribiv(dc,
                                     formats[i],
                                     0,
                                     1,
                                     &kQueryAttr,
                                     &numSamples);
        rankedFormats[i].fFormat =  formats[i];
        rankedFormats[i].fSampleCnt = numSamples;
        rankedFormats[i].fChoosePixelFormatRank = i;
    }
    SkTQSort(rankedFormats.begin(),
             rankedFormats.begin() + rankedFormats.count() - 1,
             SkTLessFunctionToFunctorAdaptor<PixelFormat, pf_less>());
    int idx = SkTSearch<PixelFormat, pf_less>(rankedFormats.begin(),
                                              rankedFormats.count(),
                                              desiredFormat,
                                              sizeof(PixelFormat));
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
    , fCreateContextAttribs(NULL)
    , fSwapInterval(NULL)
    , fCreatePbuffer(NULL)
    , fGetPbufferDC(NULL)
    , fReleasePbufferDC(NULL)
    , fDestroyPbuffer(NULL)
 {
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
        GET_PROC(SwapInterval, EXT);
        GET_PROC(CreatePbuffer, ARB);
        GET_PROC(GetPbufferDC, ARB);
        GET_PROC(ReleasePbufferDC, ARB);
        GET_PROC(DestroyPbuffer, ARB);

        wglMakeCurrent(dummyDC, NULL);
        wglDeleteContext(dummyGLRC);
        destroy_dummy_window(dummyWND);
    }

    wglMakeCurrent(prevDC, prevGLRC);
}

///////////////////////////////////////////////////////////////////////////////

static void get_pixel_formats_to_try(HDC dc, const SkWGLExtensions& extensions,
                                     bool doubleBuffered, int msaaSampleCount,
                                     int formatsToTry[2]) {
    int iAttrs[] = {
        SK_WGL_DRAW_TO_WINDOW, TRUE,
        SK_WGL_DOUBLE_BUFFER, (doubleBuffered ? TRUE : FALSE),
        SK_WGL_ACCELERATION, SK_WGL_FULL_ACCELERATION,
        SK_WGL_SUPPORT_OPENGL, TRUE,
        SK_WGL_COLOR_BITS, 24,
        SK_WGL_ALPHA_BITS, 8,
        SK_WGL_STENCIL_BITS, 8,
        0, 0
    };

    float fAttrs[] = {0, 0};

    // Get a MSAA format if requested and possible.
    if (msaaSampleCount > 0 &&
        extensions.hasExtension(dc, "WGL_ARB_multisample")) {
        static const int kIAttrsCount = SK_ARRAY_COUNT(iAttrs);
        int msaaIAttrs[kIAttrsCount + 4];
        memcpy(msaaIAttrs, iAttrs, sizeof(int) * kIAttrsCount);
        SkASSERT(0 == msaaIAttrs[kIAttrsCount - 2] &&
                 0 == msaaIAttrs[kIAttrsCount - 1]);
        msaaIAttrs[kIAttrsCount - 2] = SK_WGL_SAMPLE_BUFFERS;
        msaaIAttrs[kIAttrsCount - 1] = TRUE;
        msaaIAttrs[kIAttrsCount + 0] = SK_WGL_SAMPLES;
        msaaIAttrs[kIAttrsCount + 1] = msaaSampleCount;
        msaaIAttrs[kIAttrsCount + 2] = 0;
        msaaIAttrs[kIAttrsCount + 3] = 0;
        unsigned int num;
        int formats[64];
        extensions.choosePixelFormat(dc, msaaIAttrs, fAttrs, 64, formats, &num);
        num = SkTMin(num, 64U);
        formatsToTry[0] = extensions.selectFormat(formats, num, dc, msaaSampleCount);
    }

    // Get a non-MSAA format
    int* format = -1 == formatsToTry[0] ? &formatsToTry[0] : &formatsToTry[1];
    unsigned int num;
    extensions.choosePixelFormat(dc, iAttrs, fAttrs, 1, format, &num);
}

static HGLRC create_gl_context(HDC dc, SkWGLExtensions extensions, SkWGLContextRequest contextType) {
    HDC prevDC = wglGetCurrentDC();
    HGLRC prevGLRC = wglGetCurrentContext();

    HGLRC glrc = NULL;
    if (kGLES_SkWGLContextRequest == contextType) {
        if (!extensions.hasExtension(dc, "WGL_EXT_create_context_es2_profile")) {
            wglMakeCurrent(prevDC, prevGLRC);
            return NULL;
        }
        static const int glesAttribs[] = {
            SK_WGL_CONTEXT_MAJOR_VERSION, 3,
            SK_WGL_CONTEXT_MINOR_VERSION, 0,
            SK_WGL_CONTEXT_PROFILE_MASK,  SK_WGL_CONTEXT_ES2_PROFILE_BIT,
            0,
        };
        glrc = extensions.createContextAttribs(dc, NULL, glesAttribs);
        if (NULL == glrc) {
            wglMakeCurrent(prevDC, prevGLRC);
            return NULL;
        }
    } else {
        if (kGLPreferCoreProfile_SkWGLContextRequest == contextType &&
            extensions.hasExtension(dc, "WGL_ARB_create_context")) {
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
                if (glrc) {
                    break;
                }
            }
        }
    }

    if (NULL == glrc) {
        glrc = wglCreateContext(dc);
    }
    SkASSERT(glrc);

    wglMakeCurrent(prevDC, prevGLRC);

    // This might help make the context non-vsynced.
    if (extensions.hasExtension(dc, "WGL_EXT_swap_control")) {
        extensions.swapInterval(-1);
    }
    return glrc;
}

HGLRC SkCreateWGLContext(HDC dc, int msaaSampleCount, SkWGLContextRequest contextType) {
    SkWGLExtensions extensions;
    if (!extensions.hasExtension(dc, "WGL_ARB_pixel_format")) {
        return NULL;
    }

    BOOL set = FALSE;

    int pixelFormatsToTry[] = { -1, -1 };
    get_pixel_formats_to_try(dc, extensions, true, msaaSampleCount, pixelFormatsToTry);
    for (int f = 0;
         !set && -1 != pixelFormatsToTry[f] && f < SK_ARRAY_COUNT(pixelFormatsToTry);
         ++f) {
        PIXELFORMATDESCRIPTOR pfd;
        DescribePixelFormat(dc, pixelFormatsToTry[f], sizeof(pfd), &pfd);
        set = SetPixelFormat(dc, pixelFormatsToTry[f], &pfd);
    }

    if (!set) {
        return NULL;
    }

    return create_gl_context(dc, extensions, contextType);}

SkWGLPbufferContext* SkWGLPbufferContext::Create(HDC parentDC, int msaaSampleCount,
                                                 SkWGLContextRequest contextType) {
    SkWGLExtensions extensions;
    if (!extensions.hasExtension(parentDC, "WGL_ARB_pixel_format") ||
        !extensions.hasExtension(parentDC, "WGL_ARB_pbuffer")) {
        return NULL;
    }

    // try for single buffer first
    for (int dblBuffer = 0; dblBuffer < 2; ++dblBuffer) {
        int pixelFormatsToTry[] = { -1, -1 };
        get_pixel_formats_to_try(parentDC, extensions, (0 != dblBuffer), msaaSampleCount,
                                 pixelFormatsToTry);
        for (int f = 0; -1 != pixelFormatsToTry[f] && f < SK_ARRAY_COUNT(pixelFormatsToTry); ++f) {
            HPBUFFER pbuf = extensions.createPbuffer(parentDC, pixelFormatsToTry[f], 1, 1, NULL);
            if (0 != pbuf) {
                HDC dc = extensions.getPbufferDC(pbuf);
                if (dc) {
                    HGLRC glrc = create_gl_context(dc, extensions, contextType);
                    if (glrc) {
                        return SkNEW_ARGS(SkWGLPbufferContext, (pbuf, dc, glrc));
                    }
                    extensions.releasePbufferDC(pbuf, dc);
                }
                extensions.destroyPbuffer(pbuf);
            }
        }
    }
    return NULL;
}

SkWGLPbufferContext::~SkWGLPbufferContext() {
    SkASSERT(fExtensions.hasExtension(fDC, "WGL_ARB_pbuffer"));
    wglDeleteContext(fGLRC);
    fExtensions.releasePbufferDC(fPbuffer, fDC);
    fExtensions.destroyPbuffer(fPbuffer);
}

SkWGLPbufferContext::SkWGLPbufferContext(HPBUFFER pbuffer, HDC dc, HGLRC glrc)
    : fPbuffer(pbuffer)
    , fDC(dc)
    , fGLRC(glrc) {
}
