//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Framebuffer9.h: Defines the Framebuffer9 class.

#ifndef LIBANGLE_RENDERER_D3D_D3D9_FRAMBUFFER9_H_
#define LIBANGLE_RENDERER_D3D_D3D9_FRAMBUFFER9_H_

#include "libANGLE/renderer/d3d/FramebufferD3D.h"

namespace rx
{
class Renderer9;

class Framebuffer9 : public FramebufferD3D
{
  public:
    Framebuffer9(const gl::Framebuffer::Data &data, Renderer9 *renderer);
    virtual ~Framebuffer9();

    gl::Error discard(size_t count, const GLenum *attachments) override;
    gl::Error invalidate(size_t count, const GLenum *attachments) override;
    gl::Error invalidateSub(size_t count, const GLenum *attachments, const gl::Rectangle &area) override;

  private:
    gl::Error clear(const gl::State &state, const ClearParameters &clearParams) override;

    gl::Error readPixels(const gl::Rectangle &area, GLenum format, GLenum type, size_t outputPitch,
                         const gl::PixelPackState &pack, uint8_t *pixels) const override;

    gl::Error blit(const gl::Rectangle &sourceArea, const gl::Rectangle &destArea, const gl::Rectangle *scissor,
                   bool blitRenderTarget, bool blitDepth, bool blitStencil, GLenum filter,
                   const gl::Framebuffer *sourceFramebuffer) override;

    GLenum getRenderTargetImplementationFormat(RenderTargetD3D *renderTarget) const override;

    Renderer9 *const mRenderer;
};

}

#endif // LIBANGLE_RENDERER_D3D_D3D9_FRAMBUFFER9_H_
