//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Framebuffer11.h: Defines the Framebuffer11 class.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_FRAMBUFFER11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_FRAMBUFFER11_H_

#include "libANGLE/renderer/d3d/FramebufferD3D.h"

namespace rx
{
class Renderer11;

class Framebuffer11 : public FramebufferD3D
{
  public:
    Framebuffer11(const gl::Framebuffer::Data &data, Renderer11 *renderer);
    virtual ~Framebuffer11();

    gl::Error discard(size_t count, const GLenum *attachments) override;
    gl::Error invalidate(size_t count, const GLenum *attachments) override;
    gl::Error invalidateSub(size_t count, const GLenum *attachments, const gl::Rectangle &area) override;

    // Invalidate the cached swizzles of all bound texture attachments.
    gl::Error invalidateSwizzles() const;

  private:
    gl::Error clear(const gl::State &state, const ClearParameters &clearParams) override;

    gl::Error readPixels(const gl::Rectangle &area, GLenum format, GLenum type, size_t outputPitch,
                         const gl::PixelPackState &pack, uint8_t *pixels) const override;

    gl::Error blit(const gl::Rectangle &sourceArea, const gl::Rectangle &destArea, const gl::Rectangle *scissor,
                   bool blitRenderTarget, bool blitDepth, bool blitStencil, GLenum filter,
                   const gl::Framebuffer *sourceFramebuffer) override;

    gl::Error invalidateBase(size_t count, const GLenum *attachments, bool useEXTBehavior) const;

    GLenum getRenderTargetImplementationFormat(RenderTargetD3D *renderTarget) const override;

    Renderer11 *const mRenderer;
};

}

#endif // LIBANGLE_RENDERER_D3D_D3D11_FRAMBUFFER11_H_
