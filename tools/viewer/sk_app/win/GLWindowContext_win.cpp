
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GLWindowContext_win.h"

#include <GL/gl.h>

 // windows stuff
#include "win/SkWGL.h"
#include "Window_win.h"

namespace sk_app {

// platform-dependent create
GLWindowContext* GLWindowContext::Create(void* platformData, const DisplayParams& params) {
    GLWindowContext_win* ctx = new GLWindowContext_win(platformData, params);
    if (!ctx->isValid()) {
        delete ctx;
        return nullptr;
    }
    return ctx;
}

GLWindowContext_win::GLWindowContext_win(void* platformData, const DisplayParams& params)
    : GLWindowContext(platformData, params)
    , fHWND(0)
    , fHGLRC(NULL) {

    // any config code here (particularly for msaa)?

    this->initializeContext(platformData, params);
}

GLWindowContext_win::~GLWindowContext_win() {
    this->destroyContext();
}

void GLWindowContext_win::onInitializeContext(void* platformData, const DisplayParams& params) {

    ContextPlatformData_win* winPlatformData =
        reinterpret_cast<ContextPlatformData_win*>(platformData);

    if (winPlatformData) {
        fHWND = winPlatformData->fHWnd;
    }
    HDC dc = GetDC(fHWND);

    fHGLRC = SkCreateWGLContext(dc, params.fMSAASampleCount, params.fDeepColor,
                                kGLPreferCompatibilityProfile_SkWGLContextRequest);
    if (NULL == fHGLRC) {
        return;
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
        // pfd.cColorBits includes alpha, so it will be 32 in 8/8/8/8 and 10/10/10/2
        fColorBits = pfd.cRedBits + pfd.cGreenBits + pfd.cBlueBits;

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
    wglMakeCurrent(wglGetCurrentDC(), NULL);
    wglDeleteContext(fHGLRC);
    fHGLRC = NULL;
}


void GLWindowContext_win::onSwapBuffers() {
    HDC dc = GetDC((HWND)fHWND);
    SwapBuffers(dc);
    ReleaseDC((HWND)fHWND, dc);
}


}   //namespace sk_app
