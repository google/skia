
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/SkGLContext.h"

#include <windows.h>
#include <GL/GL.h>
#include "win/SkWGL.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace {

class WinGLContext : public SkGLContext {
public:
    WinGLContext(GrGLStandard forcedGpuAPI);
	~WinGLContext() override;
    void makeCurrent() const override;
    void swapBuffers() const override;

private:
    void destroyGLContext();

    HWND fWindow;
    HDC fDeviceContext;
    HGLRC fGlRenderContext;
    static ATOM gWC;
    SkWGLPbufferContext* fPbufferContext;
};

ATOM WinGLContext::gWC = 0;

WinGLContext::WinGLContext(GrGLStandard forcedGpuAPI)
    : fWindow(NULL)
    , fDeviceContext(NULL)
    , fGlRenderContext(0)
    , fPbufferContext(NULL) {
    HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);

    if (!gWC) {
        WNDCLASS wc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hbrBackground = NULL;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        wc.hInstance = hInstance;
        wc.lpfnWndProc = (WNDPROC) DefWindowProc;
        wc.lpszClassName = TEXT("Griffin");
        wc.lpszMenuName = NULL;
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;

        gWC = RegisterClass(&wc);
        if (!gWC) {
            SkDebugf("Could not register window class.\n");
            return;
        }
    }

    if (!(fWindow = CreateWindow(TEXT("Griffin"),
                                 TEXT("The Invisible Man"),
                                 WS_OVERLAPPEDWINDOW,
                                 0, 0, 1, 1,
                                 NULL, NULL,
                                 hInstance, NULL))) {
        SkDebugf("Could not create window.\n");
        return;
    }

    if (!(fDeviceContext = GetDC(fWindow))) {
        SkDebugf("Could not get device context.\n");
        this->destroyGLContext();
        return;
    }
    // Requesting a Core profile would bar us from using NVPR. So we request
    // compatibility profile or GL ES.
    SkWGLContextRequest contextType =
        kGLES_GrGLStandard == forcedGpuAPI ?
        kGLES_SkWGLContextRequest : kGLPreferCompatibilityProfile_SkWGLContextRequest;

    fPbufferContext = SkWGLPbufferContext::Create(fDeviceContext, 0, contextType);

    HDC dc;
    HGLRC glrc;

    if (NULL == fPbufferContext) {
        if (!(fGlRenderContext = SkCreateWGLContext(fDeviceContext, 0, contextType))) {
            SkDebugf("Could not create rendering context.\n");
            this->destroyGLContext();
            return;
        }
        dc = fDeviceContext;
        glrc = fGlRenderContext;
    } else {
        ReleaseDC(fWindow, fDeviceContext);
        fDeviceContext = 0;
        DestroyWindow(fWindow);
        fWindow = 0;

        dc = fPbufferContext->getDC();
        glrc = fPbufferContext->getGLRC();
    }

    if (!(wglMakeCurrent(dc, glrc))) {
        SkDebugf("Could not set the context.\n");
        this->destroyGLContext();
        return;
    }

    fGL.reset(GrGLCreateNativeInterface());
    if (NULL == fGL.get()) {
        SkDebugf("Could not create GL interface.\n");
        this->destroyGLContext();
        return;
    }
    if (!fGL->validate()) {
        SkDebugf("Could not validate GL interface.\n");
        this->destroyGLContext();
        return;
    }
}

WinGLContext::~WinGLContext() {
    this->destroyGLContext();
}

void WinGLContext::destroyGLContext() {
    fGL.reset(NULL);
    SkSafeSetNull(fPbufferContext);
    if (fGlRenderContext) {
        wglDeleteContext(fGlRenderContext);
        fGlRenderContext = 0;
    }
    if (fWindow && fDeviceContext) {
        ReleaseDC(fWindow, fDeviceContext);
        fDeviceContext = 0;
    }
    if (fWindow) {
        DestroyWindow(fWindow);
        fWindow = 0;
    }
}

void WinGLContext::makeCurrent() const {
    HDC dc;
    HGLRC glrc;

    if (NULL == fPbufferContext) {
        dc = fDeviceContext;
        glrc = fGlRenderContext;
    } else {
        dc = fPbufferContext->getDC();
        glrc = fPbufferContext->getGLRC();
    }

    if (!wglMakeCurrent(dc, glrc)) {
        SkDebugf("Could not create rendering context.\n");
    }
}

void WinGLContext::swapBuffers() const {
    HDC dc;

    if (NULL == fPbufferContext) {
        dc = fDeviceContext;
    } else {
        dc = fPbufferContext->getDC();
    }
    if (!SwapBuffers(dc)) {
        SkDebugf("Could not complete SwapBuffers.\n");
    }
}

} // anonymous namespace

SkGLContext* SkCreatePlatformGLContext(GrGLStandard forcedGpuAPI) {
    WinGLContext* ctx = SkNEW_ARGS(WinGLContext, (forcedGpuAPI));
    if (!ctx->isValid()) {
        SkDELETE(ctx);
        return NULL;
    }
    return ctx;
}

