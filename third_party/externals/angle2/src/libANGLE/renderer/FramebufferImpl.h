//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FramebufferImpl.h: Defines the abstract rx::FramebufferImpl class.

#ifndef LIBANGLE_RENDERER_FRAMEBUFFERIMPL_H_
#define LIBANGLE_RENDERER_FRAMEBUFFERIMPL_H_

#include "angle_gl.h"
#include "common/angleutils.h"
#include "libANGLE/Error.h"
#include "libANGLE/Framebuffer.h"

namespace gl
{
class State;
class Framebuffer;
class FramebufferAttachment;
struct Rectangle;
}

namespace rx
{

class FramebufferImpl : angle::NonCopyable
{
  public:
    explicit FramebufferImpl(const gl::Framebuffer::Data &data) : mData(data) { }
    virtual ~FramebufferImpl() { }

    virtual void onUpdateColorAttachment(size_t index) = 0;
    virtual void onUpdateDepthAttachment() = 0;
    virtual void onUpdateStencilAttachment() = 0;
    virtual void onUpdateDepthStencilAttachment() = 0;

    virtual void setDrawBuffers(size_t count, const GLenum *buffers) = 0;
    virtual void setReadBuffer(GLenum buffer) = 0;

    virtual gl::Error discard(size_t count, const GLenum *attachments) = 0;
    virtual gl::Error invalidate(size_t count, const GLenum *attachments) = 0;
    virtual gl::Error invalidateSub(size_t count, const GLenum *attachments, const gl::Rectangle &area) = 0;

    virtual gl::Error clear(const gl::Data &data, GLbitfield mask) = 0;
    virtual gl::Error clearBufferfv(const gl::State &state, GLenum buffer, GLint drawbuffer, const GLfloat *values) = 0;
    virtual gl::Error clearBufferuiv(const gl::State &state, GLenum buffer, GLint drawbuffer, const GLuint *values) = 0;
    virtual gl::Error clearBufferiv(const gl::State &state, GLenum buffer, GLint drawbuffer, const GLint *values) = 0;
    virtual gl::Error clearBufferfi(const gl::State &state, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil) = 0;

    virtual GLenum getImplementationColorReadFormat() const = 0;
    virtual GLenum getImplementationColorReadType() const = 0;
    virtual gl::Error readPixels(const gl::State &state, const gl::Rectangle &area, GLenum format, GLenum type, GLvoid *pixels) const = 0;

    virtual gl::Error blit(const gl::State &state, const gl::Rectangle &sourceArea, const gl::Rectangle &destArea,
                           GLbitfield mask, GLenum filter, const gl::Framebuffer *sourceFramebuffer) = 0;

    virtual GLenum checkStatus() const = 0;

    const gl::Framebuffer::Data &getData() const { return mData; }

  protected:
    const gl::Framebuffer::Data &mData;
};

}

#endif // LIBANGLE_RENDERER_FRAMEBUFFERIMPL_H_
