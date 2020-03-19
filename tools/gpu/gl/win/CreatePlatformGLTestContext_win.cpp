
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/gl/GLTestContext.h"

#if defined(_M_ARM64)

namespace sk_gpu_test {

GLTestContext* CreatePlatformGLTestContext(GrGLStandard, GLTestContext*) { return nullptr; }

}  // namespace sk_gpu_test

#else

#include <windows.h>
#include <GL/GL.h>
#include "src/utils/win/SkWGL.h"

#include <windows.h>

namespace {

std::function<void()> context_restorer() {
    auto glrc = wglGetCurrentContext();
    auto dc = wglGetCurrentDC();
    return [glrc, dc] { wglMakeCurrent(dc, glrc); };
}

class WinGLTestContext : public sk_gpu_test::GLTestContext {
public:
    WinGLTestContext(GrGLStandard forcedGpuAPI, WinGLTestContext* shareContext);
    ~WinGLTestContext() override;

private:
    void destroyGLContext();

    void onPlatformMakeNotCurrent() const override;
    void onPlatformMakeCurrent() const override;
    std::function<void()> onPlatformGetAutoContextRestore() const override;
    GrGLFuncPtr onPlatformGetProcAddress(const char* name) const override;

    HWND fWindow;
    HDC fDeviceContext;
    HGLRC fGlRenderContext;
    static ATOM gWC;
    sk_sp<SkWGLPbufferContext> fPbufferContext;
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
    fPbufferContext = SkWGLPbufferContext::Create(fDeviceContext, contextType, winShareContext);

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

    SkScopeExit restorer(context_restorer());
    if (!(wglMakeCurrent(dc, glrc))) {
        SkDebugf("Could not set the context.\n");
        this->destroyGLContext();
        return;
    }

#ifdef SK_GL
    auto gl = GrGLMakeNativeInterface();
    if (!gl) {
        SkDebugf("Could not create GL interface.\n");
        this->destroyGLContext();
        return;
    }
    if (!gl->validate()) {
        SkDebugf("Could not validate GL interface.\n");
        this->destroyGLContext();
        return;
    }

    this->init(std::move(gl));
#else
    // Allow the GLTestContext creation to succeed without a GrGLInterface to support
    // GrContextFactory's persistent GL context workaround for Vulkan. We won't need the
    // GrGLInterface since we're not running the GL backend.
    this->init(nullptr);
#endif
}

WinGLTestContext::~WinGLTestContext() {
    this->teardown();
    this->destroyGLContext();
}

void WinGLTestContext::destroyGLContext() {
    fPbufferContext = nullptr;
    if (fGlRenderContext) {
        // This deletes the context immediately even if it is current.
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

void WinGLTestContext::onPlatformMakeNotCurrent() const {
    if (!wglMakeCurrent(NULL, NULL)) {
        SkDebugf("Could not null out the rendering context.\n");
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
        SkDebugf("Could not make current.\n");
    }
}

std::function<void()> WinGLTestContext::onPlatformGetAutoContextRestore() const {
    if (wglGetCurrentContext() == fGlRenderContext) {
        return nullptr;
    }
    return context_restorer();
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

#endif
