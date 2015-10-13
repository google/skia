//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// PBufferSurfaceWGL.h: WGL implementation of egl::Surface for PBuffers

#ifndef LIBANGLE_RENDERER_GL_WGL_PBUFFERSURFACEWGL_H_
#define LIBANGLE_RENDERER_GL_WGL_PBUFFERSURFACEWGL_H_

#include "libANGLE/renderer/gl/SurfaceGL.h"

#include <GL/wglext.h>

namespace rx
{

class FunctionsWGL;

class PbufferSurfaceWGL : public SurfaceGL
{
  public:
    PbufferSurfaceWGL(RendererGL *renderer,
                      EGLint width,
                      EGLint height,
                      EGLenum textureFormat,
                      EGLenum textureTarget,
                      bool largest,
                      int pixelFormat,
                      HDC deviceContext,
                      HGLRC wglContext,
                      const FunctionsWGL *functions);
    ~PbufferSurfaceWGL() override;

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
    EGLint mWidth;
    EGLint mHeight;
    bool mLargest;
    EGLenum mTextureFormat;
    EGLenum mTextureTarget;

    int mPixelFormat;

    HGLRC mShareWGLContext;

    HDC mParentDeviceContext;

    HPBUFFERARB mPbuffer;
    HDC mPbufferDeviceContext;

    const FunctionsWGL *mFunctionsWGL;
};

}

#endif // LIBANGLE_RENDERER_GL_WGL_PBUFFERSURFACEWGL_H_
