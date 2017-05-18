
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "WindowContextFactory_win.h"
#include <GL/gl.h>

 // windows stuff
#include "win/SkWGL.h"

#include "../GLWindowContext.h"

using sk_app::GLWindowContext;
using sk_app::DisplayParams;

namespace {

class GLWindowContext_win : public GLWindowContext {
public:
    GLWindowContext_win(HWND, const DisplayParams&);
    ~GLWindowContext_win() override;

protected:
    void onSwapBuffers() override;

    void onInitializeContext() override;
    void onDestroyContext() override;

private:
    HWND              fHWND;
    HGLRC             fHGLRC;

    typedef GLWindowContext INHERITED;
};

GLWindowContext_win::GLWindowContext_win(HWND wnd, const DisplayParams& params)
    : INHERITED(params)
    , fHWND(wnd)
    , fHGLRC(NULL) {

    // any config code here (particularly for msaa)?

    this->initializeContext();
}

GLWindowContext_win::~GLWindowContext_win() {
    this->destroyContext();
}

void GLWindowContext_win::onInitializeContext() {
    HDC dc = GetDC(fHWND);

    fHGLRC = SkCreateWGLContext(dc, fDisplayParams.fMSAASampleCount, false /* deepColor */,
                                kGLPreferCompatibilityProfile_SkWGLContextRequest);
    if (NULL == fHGLRC) {
        return;
    }

    // Look to see if RenderDoc is attached. If so, re-create the context with a core profile
    if (wglMakeCurrent(dc, fHGLRC)) {
        const GrGLInterface* glInterface = GrGLCreateNativeInterface();
        bool renderDocAttached = glInterface->hasExtension("GL_EXT_debug_tool");
        SkSafeUnref(glInterface);
        if (renderDocAttached) {
            wglDeleteContext(fHGLRC);
            fHGLRC = SkCreateWGLContext(dc, fDisplayParams.fMSAASampleCount, false /* deepColor */,
                                        kGLPreferCoreProfile_SkWGLContextRequest);
            if (NULL == fHGLRC) {
                return;
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
        SkWGLExtensions extensions;
        if (extensions.hasExtension(dc, "WGL_ARB_multisample")) {
            static const int kSampleCountAttr = SK_WGL_SAMPLES;
            extensions.getPixelFormatAttribiv(dc,
                                              pixelFormat,
                                              0,
                                              1,
                                              &kSampleCountAttr,
                                              &fSampleCount);
        } else {
            fSampleCount = 0;
        }

        RECT rect;
        GetClientRect(fHWND, &rect);
        fWidth = rect.right - rect.left;
        fHeight = rect.bottom - rect.top;
        glViewport(0, 0, fWidth, fHeight);
    }
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

namespace sk_app {
namespace window_context_factory {

WindowContext* NewGLForWin(HWND wnd, const DisplayParams& params) {
    GLWindowContext_win* ctx = new GLWindowContext_win(wnd, params);
    if (!ctx->isValid()) {
        delete ctx;
        return nullptr;
    }
    return ctx;
}

}  // namespace window_context_factory
}  // namespace sk_app
