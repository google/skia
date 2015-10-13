//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FramebufferGL.h: Defines the class interface for FramebufferGL.

#ifndef LIBANGLE_RENDERER_GL_FRAMEBUFFERGL_H_
#define LIBANGLE_RENDERER_GL_FRAMEBUFFERGL_H_

#include "libANGLE/renderer/FramebufferImpl.h"

namespace rx
{

class FunctionsGL;
class StateManagerGL;
struct WorkaroundsGL;

class FramebufferGL : public FramebufferImpl
{
  public:
    FramebufferGL(const gl::Framebuffer::Data &data,
                  const FunctionsGL *functions,
                  StateManagerGL *stateManager,
                  const WorkaroundsGL &workarounds,
                  bool isDefault);
    // Constructor called when we need to create a FramebufferGL from an
    // existing framebuffer name, for example for the default framebuffer
    // on the Mac EGL CGL backend.
    FramebufferGL(GLuint id,
                  const gl::Framebuffer::Data &data,
                  const FunctionsGL *functions,
                  const WorkaroundsGL &workarounds,
                  StateManagerGL *stateManager);
    ~FramebufferGL() override;

    void onUpdateColorAttachment(size_t index) override;
    void onUpdateDepthAttachment() override;
    void onUpdateStencilAttachment() override;
    void onUpdateDepthStencilAttachment() override;

    void setDrawBuffers(size_t count, const GLenum *buffers) override;
    void setReadBuffer(GLenum buffer) override;

    gl::Error discard(size_t count, const GLenum *attachments) override;
    gl::Error invalidate(size_t count, const GLenum *attachments) override;
    gl::Error invalidateSub(size_t count, const GLenum *attachments, const gl::Rectangle &area) override;

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

    void syncDrawState() const;

    GLuint getFramebufferID() const;

  private:
    void syncClearState(GLbitfield mask);
    void syncClearBufferState(GLenum buffer, GLint drawBuffer);

    const FunctionsGL *mFunctions;
    StateManagerGL *mStateManager;
    const WorkaroundsGL &mWorkarounds;

    GLuint mFramebufferID;
    bool mIsDefault;
};

}

#endif // LIBANGLE_RENDERER_GL_FRAMEBUFFERGL_H_
