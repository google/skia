//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// PBufferSurfaceGLX.h: GLX implementation of egl::Surface for PBuffers

#ifndef LIBANGLE_RENDERER_GL_GLX_PBUFFERSURFACEGLX_H_
#define LIBANGLE_RENDERER_GL_GLX_PBUFFERSURFACEGLX_H_

#include "libANGLE/renderer/gl/SurfaceGL.h"
#include "libANGLE/renderer/gl/glx/platform_glx.h"

namespace rx
{

class DisplayGLX;
class FunctionsGLX;

class PbufferSurfaceGLX : public SurfaceGL
{
  public:
    PbufferSurfaceGLX(RendererGL *renderer,
                      EGLint width,
                      EGLint height,
                      bool largest,
                      const FunctionsGLX &glx,
                      glx::Context context,
                      glx::FBConfig fbConfig);
    ~PbufferSurfaceGLX() override;

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
    unsigned mWidth;
    unsigned mHeight;
    bool mLargest;

    const FunctionsGLX &mGLX;
    glx::Context mContext;
    glx::FBConfig mFBConfig;
    glx::Pbuffer mPbuffer;
};

}

#endif // LIBANGLE_RENDERER_GL_GLX_PBUFFERSURFACEGLX_H_
