
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/GLTestContext.h"

#include <windows.h>
#include <GL/GL.h>
#include "win/SkWGL.h"

#include <windows.h>

namespace {

class WinGLTestContext : public sk_gpu_test::GLTestContext {
public:
    WinGLTestContext(GrGLStandard forcedGpuAPI, WinGLTestContext* shareContext);
    ~WinGLTestContext() override;

private:
    void destroyGLContext();

    void onPlatformMakeCurrent() const override;
    void onPlatformSwapBuffers() const override;
    GrGLFuncPtr onPlatformGetProcAddress(const char* name) const override;

    HWND fWindow;
    HDC fDeviceContext;
    HGLRC fGlRenderContext;
    static ATOM gWC;
    SkWGLPbufferContext* fPbufferContext;
};

ATOM WinGLTestContext::gWC = 0;

WinGLTestContext::WinGLTestContext(GrGLStandard forcedGpuAPI, WinGLTestContext* shareContext)
    : fWindow(nullptr)
    , fDeviceContext(nullptr)
    , fGlRenderContext(0)
    , fPbufferContext(nullptr) {
    HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(nullptr);

    if (!gWC) {
        WNDCLASS wc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hbrBackground = nullptr;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
        wc.hInstance = hInstance;
        wc.lpfnWndProc = (WNDPROC) DefWindowProc;
        wc.lpszClassName = TEXT("Griffin");
        wc.lpszMenuName = nullptr;
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
                                 nullptr, nullptr,
                                 hInstance, nullptr))) {
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

    HGLRC winShareContext = nullptr;
    if (shareContext) {
        winShareContext = shareContext->fPbufferContext ? shareContext->fPbufferContext->getGLRC()
                                                        : shareContext->fGlRenderContext;
    }
    fPbufferContext = SkWGLPbufferContext::Create(fDeviceContext, 0, contextType, winShareContext);

    HDC dc;
    HGLRC glrc;
    if (nullptr == fPbufferContext) {
        if (!(fGlRenderContext = SkCreateWGLContext(fDeviceContext, 0, false, contextType,
                                                    winShareContext))) {
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

    sk_sp<const GrGLInterface> gl(GrGLCreateNativeInterface());
    if (nullptr == gl.get()) {
        SkDebugf("Could not create GL interface.\n");
        this->destroyGLContext();
        return;
    }
    if (!gl->validate()) {
        SkDebugf("Could not validate GL interface.\n");
        this->destroyGLContext();
        return;
    }

    this->init(gl.release());
}

WinGLTestContext::~WinGLTestContext() {
    this->teardown();
    this->destroyGLContext();
}

void WinGLTestContext::destroyGLContext() {
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

void WinGLTestContext::onPlatformMakeCurrent() const {
    HDC dc;
    HGLRC glrc;

    if (nullptr == fPbufferContext) {
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

void WinGLTestContext::onPlatformSwapBuffers() const {
    HDC dc;

    if (nullptr == fPbufferContext) {
        dc = fDeviceContext;
    } else {
        dc = fPbufferContext->getDC();
    }
    if (!SwapBuffers(dc)) {
        SkDebugf("Could not complete SwapBuffers.\n");
    }
}

GrGLFuncPtr WinGLTestContext::onPlatformGetProcAddress(const char* name) const {
    return reinterpret_cast<GrGLFuncPtr>(wglGetProcAddress(name));
}

} // anonymous namespace

namespace sk_gpu_test {
GLTestContext* CreatePlatformGLTestContext(GrGLStandard forcedGpuAPI,
                                           GLTestContext *shareContext) {
    WinGLTestContext* winShareContext = reinterpret_cast<WinGLTestContext*>(shareContext);
    WinGLTestContext *ctx = new WinGLTestContext(forcedGpuAPI, winShareContext);
    if (!ctx->isValid()) {
        delete ctx;
        return nullptr;
    }
    return ctx;
}
}  // namespace sk_gpu_test

