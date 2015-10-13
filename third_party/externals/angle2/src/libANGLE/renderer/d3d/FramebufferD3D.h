//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FramebufferD3D.h: Defines the DefaultAttachmentD3D and FramebufferD3D classes.

#ifndef LIBANGLE_RENDERER_D3D_FRAMBUFFERD3D_H_
#define LIBANGLE_RENDERER_D3D_FRAMBUFFERD3D_H_

#include <vector>
#include <cstdint>

#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/FramebufferImpl.h"

namespace gl
{
class FramebufferAttachment;
struct PixelPackState;

typedef std::vector<const FramebufferAttachment *> AttachmentList;

}

namespace rx
{
class RenderTargetD3D;
struct WorkaroundsD3D;

struct ClearParameters
{
    bool clearColor[gl::IMPLEMENTATION_MAX_DRAW_BUFFERS];
    gl::ColorF colorFClearValue;
    gl::ColorI colorIClearValue;
    gl::ColorUI colorUIClearValue;
    GLenum colorClearType;
    bool colorMaskRed;
    bool colorMaskGreen;
    bool colorMaskBlue;
    bool colorMaskAlpha;

    bool clearDepth;
    float depthClearValue;

    bool clearStencil;
    GLint stencilClearValue;
    GLuint stencilWriteMask;

    bool scissorEnabled;
    gl::Rectangle scissor;
};

class FramebufferD3D : public FramebufferImpl
{
  public:
    FramebufferD3D(const gl::Framebuffer::Data &data);
    virtual ~FramebufferD3D();

    void onUpdateColorAttachment(size_t index) override;
    void onUpdateDepthAttachment() override;
    void onUpdateStencilAttachment() override;
    void onUpdateDepthStencilAttachment() override;

    void setDrawBuffers(size_t count, const GLenum *buffers) override;
    void setReadBuffer(GLenum buffer) override;

    gl::Error clear(const gl::Data &data, GLbitfield mask) override;
    gl::Error clearBufferfv(const gl::State &state, GLenum buffer, GLint drawbuffer, const GLfloat *values) override;
    gl::Error clearBufferuiv(const gl::State &state, GLenum buffer, GLint drawbuffer, const GLuint *values) override;
    gl::Error clearBufferiv(const gl::State &state, GLenum buffer, GLint drawbuffer, const GLint *values) override;
    gl::Error clearBufferfi(const gl::State &state, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil) override;

    GLenum getImplementationColorReadFormat() const override;
    GLenum getImplementationColorReadType() const override;
    gl::Error readPixels(const gl::State &state, const gl::Rectangle &area, GLenum format, GLenum type, GLvoid *pixels) const override;

    gl::Error blit(const gl::State &state, const gl::Rectangle &sourceArea, const gl::Rectangle &destArea,
                   GLbitfield mask, GLenum filter, const gl::Framebuffer *sourceFramebuffer) override;

    GLenum checkStatus() const override;

    const gl::AttachmentList &getColorAttachmentsForRender(const WorkaroundsD3D &workarounds) const;

  protected:
    // Cache variable
    mutable gl::AttachmentList mColorAttachmentsForRender;
    mutable bool mInvalidateColorAttachmentCache;

  private:
    virtual gl::Error clear(const gl::State &state, const ClearParameters &clearParams) = 0;

    virtual gl::Error readPixels(const gl::Rectangle &area, GLenum format, GLenum type, size_t outputPitch,
                                 const gl::PixelPackState &pack, uint8_t *pixels) const = 0;

    virtual gl::Error blit(const gl::Rectangle &sourceArea, const gl::Rectangle &destArea, const gl::Rectangle *scissor,
                           bool blitRenderTarget, bool blitDepth, bool blitStencil, GLenum filter,
                           const gl::Framebuffer *sourceFramebuffer) = 0;

    virtual GLenum getRenderTargetImplementationFormat(RenderTargetD3D *renderTarget) const = 0;
};

}

#endif // LIBANGLE_RENDERER_D3D_FRAMBUFFERD3D_H_
