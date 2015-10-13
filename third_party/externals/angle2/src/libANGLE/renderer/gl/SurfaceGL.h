//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SurfaceGL.h: Defines the class interface for SurfaceGL.

#ifndef LIBANGLE_RENDERER_GL_SURFACEGL_H_
#define LIBANGLE_RENDERER_GL_SURFACEGL_H_

#include "libANGLE/renderer/SurfaceImpl.h"

namespace rx
{

class RendererGL;

class SurfaceGL : public SurfaceImpl
{
  public:
    SurfaceGL(RendererGL *renderer);
    ~SurfaceGL() override;

    gl::Error getAttachmentRenderTarget(const gl::FramebufferAttachment::Target &target,
                                        FramebufferAttachmentRenderTarget **rtOut) override
    {
        return gl::Error(GL_OUT_OF_MEMORY, "Not supported on OpenGL");
    }

    FramebufferImpl *createDefaultFramebuffer(const gl::Framebuffer::Data &data) override;

    virtual egl::Error makeCurrent() = 0;

  private:
    RendererGL *mRenderer;
};

}

#endif // LIBANGLE_RENDERER_GL_SURFACEGL_H_
