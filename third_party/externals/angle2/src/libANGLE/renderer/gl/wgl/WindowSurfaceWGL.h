//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// WindowSurfaceWGL.h: WGL implementation of egl::Surface for windows

#ifndef LIBANGLE_RENDERER_GL_WGL_WINDOWSURFACEWGL_H_
#define LIBANGLE_RENDERER_GL_WGL_WINDOWSURFACEWGL_H_

#include "libANGLE/renderer/gl/SurfaceGL.h"

#include <GL/wglext.h>

namespace rx
{

class FunctionsWGL;

class WindowSurfaceWGL : public SurfaceGL
{
  public:
    WindowSurfaceWGL(RendererGL *renderer,
                     EGLNativeWindowType window,
                     int pixelFormat,
                     HGLRC wglContext,
                     const FunctionsWGL *functions);
    ~WindowSurfaceWGL() override;

    egl::Error initialize() override;
    egl::Error makeCurrent() override;

    egl::Error swap() override;
    egl::Error postSubBuffer(EGLint x, EGLint y, EGLint width, EGLint height) override;
    egl::Error querySurfacePointerANGLE(EGLint attribute, void **value) override;
    egl::Error bindTexImage(EGLint buffer) override;
    egl::Error releaseTexImage(EGLint buffer) override;
    void setSwapInterval(EGLint interval) override;

    EGLint getWidth() const override;
    EGLint getHeight() const override;

    EGLint isPostSubBufferSupported() const override;
    EGLint getSwapBehavior() const override;

  private:
    int mPixelFormat;

    HGLRC mWGLContext;

    HWND mWindow;
    HDC mDeviceContext;

    const FunctionsWGL *mFunctionsWGL;

    EGLint mSwapBehavior;
};

}

#endif // LIBANGLE_RENDERER_GL_WGL_WINDOWSURFACEWGL_H_
