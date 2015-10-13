//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// PBufferSurfaceCGL.h: an implementation of egl::Surface for PBuffers for the CLG backend,
//                      currently implemented using renderbuffers

#ifndef LIBANGLE_RENDERER_GL_CGL_PBUFFERSURFACECGL_H_
#define LIBANGLE_RENDERER_GL_CGL_PBUFFERSURFACECGL_H_

#include "libANGLE/renderer/gl/SurfaceGL.h"

namespace rx
{

class FunctionsGL;
class StateManagerGL;
struct WorkaroundsGL;

class PbufferSurfaceCGL : public SurfaceGL
{
  public:
    PbufferSurfaceCGL(RendererGL *renderer,
                      EGLint width,
                      EGLint height,
                      const FunctionsGL *functions);
    ~PbufferSurfaceCGL() override;

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

    FramebufferImpl *createDefaultFramebuffer(const gl::Framebuffer::Data &data) override;

  private:
    unsigned mWidth;
    unsigned mHeight;

    const FunctionsGL *mFunctions;
    StateManagerGL *mStateManager;
    const WorkaroundsGL &mWorkarounds;

    GLuint mFramebuffer;
    GLuint mColorRenderbuffer;
    GLuint mDSRenderbuffer;
};

}

#endif // LIBANGLE_RENDERER_GL_CGL_PBUFFERSURFACECGL_H_
