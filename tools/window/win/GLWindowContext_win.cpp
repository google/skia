
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/gl/GrGLInterface.h"
#include "tools/gpu/gl/win/SkWGL.h"
#include "tools/window/GLWindowContext.h"
#include "tools/window/win/WindowContextFactory_win.h"

#include <Windows.h>
#include <GL/gl.h>

using skwindow::DisplayParams;
using skwindow::internal::GLWindowContext;

#if defined(_M_ARM64)

namespace skwindow {

std::unique_ptr<WindowContext> MakeGLForWin(HWND, const DisplayParams&) { return nullptr; }

}  // namespace skwindow

#else

namespace {

class GLWindowContext_win : public GLWindowContext {
public:
    GLWindowContext_win(HWND, const DisplayParams&);
    ~GLWindowContext_win() override;

protected:
    void onSwapBuffers() override;

    sk_sp<const GrGLInterface> onInitializeContext() override;
    void onDestroyContext() override;

private:
    HWND              fHWND;
    HGLRC             fHGLRC;
};

GLWindowContext_win::GLWindowContext_win(HWND wnd, const DisplayParams& params)
        : GLWindowContext(params)
        , fHWND(wnd)
        , fHGLRC(nullptr) {

    // any config code here (particularly for msaa)?

    this->initializeContext();
}

GLWindowContext_win::~GLWindowContext_win() {
    this->destroyContext();
}

sk_sp<const GrGLInterface> GLWindowContext_win::onInitializeContext() {
    HDC dc = GetDC(fHWND);

    fHGLRC = SkCreateWGLContext(dc, fDisplayParams.fMSAASampleCount, false /* deepColor */,
                                kGLPreferCompatibilityProfile_SkWGLContextRequest);
    if (nullptr == fHGLRC) {
        return nullptr;
    }

    SkWGLExtensions extensions;
    if (extensions.hasExtension(dc, "WGL_EXT_swap_control")) {
        extensions.swapInterval(fDisplayParams.fDisableVsync ? 0 : 1);
    }

    // Look to see if RenderDoc is attached. If so, re-create the context with a core profile
    if (wglMakeCurrent(dc, fHGLRC)) {
        auto interface = GrGLMakeNativeInterface();
        bool renderDocAttached = interface->hasExtension("GL_EXT_debug_tool");
        interface.reset(nullptr);
        if (renderDocAttached) {
            wglDeleteContext(fHGLRC);
            fHGLRC = SkCreateWGLContext(dc, fDisplayParams.fMSAASampleCount, false /* deepColor */,
                                        kGLPreferCoreProfile_SkWGLContextRequest);
            if (nullptr == fHGLRC) {
                return nullptr;
            }
        }
    }

    if (wglMakeCurrent(dc, fHGLRC)) {
        glClearStencil(0);
        glClearColor(0, 0, 0, 0);
        glStencilMask(0xffffffff);
        glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        // use DescribePixelFormat to get the stencil and color bit depth.
        int pixelFormat = GetPixelFormat(dc);
        PIXELFORMATDESCRIPTOR pfd;
        DescribePixelFormat(dc, pixelFormat, sizeof(pfd), &pfd);
        fStencilBits = pfd.cStencilBits;

        // Get sample count if the MSAA WGL extension is present
        if (extensions.hasExtension(dc, "WGL_ARB_multisample")) {
            static const int kSampleCountAttr = SK_WGL_SAMPLES;
            extensions.getPixelFormatAttribiv(dc,
                                              pixelFormat,
                                              0,
                                              1,
                                              &kSampleCountAttr,
                                              &fSampleCount);
            fSampleCount = std::max(fSampleCount, 1);
        } else {
            fSampleCount = 1;
        }

        RECT rect;
        GetClientRect(fHWND, &rect);
        fWidth = rect.right - rect.left;
        fHeight = rect.bottom - rect.top;
        glViewport(0, 0, fWidth, fHeight);
    }
    return GrGLMakeNativeInterface();
}


void GLWindowContext_win::onDestroyContext() {
    wglDeleteContext(fHGLRC);
    fHGLRC = NULL;
}


void GLWindowContext_win::onSwapBuffers() {
    HDC dc = GetDC((HWND)fHWND);
    SwapBuffers(dc);
    ReleaseDC((HWND)fHWND, dc);
}


}  // anonymous namespace

namespace skwindow {

std::unique_ptr<WindowContext> MakeGLForWin(HWND wnd, const DisplayParams& params) {
    std::unique_ptr<WindowContext> ctx(new GLWindowContext_win(wnd, params));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}

}  // namespace skwindow

#endif
