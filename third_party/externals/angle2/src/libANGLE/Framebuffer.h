//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Framebuffer.h: Defines the gl::Framebuffer class. Implements GL framebuffer
// objects and related functionality. [OpenGL ES 2.0.24] section 4.4 page 105.

#ifndef LIBANGLE_FRAMEBUFFER_H_
#define LIBANGLE_FRAMEBUFFER_H_

#include <vector>

#include "common/angleutils.h"
#include "libANGLE/Constants.h"
#include "libANGLE/Error.h"
#include "libANGLE/FramebufferAttachment.h"
#include "libANGLE/RefCountObject.h"

namespace rx
{
class ImplFactory;
class FramebufferImpl;
class RenderbufferImpl;
class SurfaceImpl;
}

namespace egl
{
class Surface;
}

namespace gl
{
class Context;
class Renderbuffer;
class State;
class Texture;
class TextureCapsMap;
struct Caps;
struct Data;
struct Extensions;
struct ImageIndex;
struct Rectangle;

class Framebuffer
{
  public:

    class Data final : angle::NonCopyable
    {
      public:
        explicit Data();
        explicit Data(const Caps &caps);
        ~Data();

        const FramebufferAttachment *getReadAttachment() const;
        const FramebufferAttachment *getFirstColorAttachment() const;
        const FramebufferAttachment *getDepthOrStencilAttachment() const;
        const FramebufferAttachment *getColorAttachment(size_t colorAttachment) const;
        const FramebufferAttachment *getDepthAttachment() const;
        const FramebufferAttachment *getStencilAttachment() const;
        const FramebufferAttachment *getDepthStencilAttachment() const;

        const std::vector<GLenum> &getDrawBufferStates() const { return mDrawBufferStates; }
        const std::vector<FramebufferAttachment> &getColorAttachments() const { return mColorAttachments; }

      private:
        friend class Framebuffer;

        std::vector<FramebufferAttachment> mColorAttachments;
        FramebufferAttachment mDepthAttachment;
        FramebufferAttachment mStencilAttachment;

        std::vector<GLenum> mDrawBufferStates;
        GLenum mReadBufferState;
    };

    Framebuffer(const Caps &caps, rx::ImplFactory *factory, GLuint id);
    Framebuffer(rx::SurfaceImpl *surface);
    virtual ~Framebuffer();

    const rx::FramebufferImpl *getImplementation() const { return mImpl; }
    rx::FramebufferImpl *getImplementation() { return mImpl; }

    GLuint id() const { return mId; }

    void setAttachment(GLenum type,
                       GLenum binding,
                       const ImageIndex &textureIndex,
                       FramebufferAttachmentObject *resource);
    void resetAttachment(GLenum binding);

    void detachTexture(GLuint texture);
    void detachRenderbuffer(GLuint renderbuffer);

    const FramebufferAttachment *getColorbuffer(size_t colorAttachment) const;
    const FramebufferAttachment *getDepthbuffer() const;
    const FramebufferAttachment *getStencilbuffer() const;
    const FramebufferAttachment *getDepthStencilBuffer() const;
    const FramebufferAttachment *getDepthOrStencilbuffer() const;
    const FramebufferAttachment *getReadColorbuffer() const;
    GLenum getReadColorbufferType() const;
    const FramebufferAttachment *getFirstColorbuffer() const;

    const FramebufferAttachment *getAttachment(GLenum attachment) const;

    GLenum getDrawBufferState(unsigned int colorAttachment) const;
    void setDrawBuffers(size_t count, const GLenum *buffers);

    GLenum getReadBufferState() const;
    void setReadBuffer(GLenum buffer);

    bool isEnabledColorAttachment(size_t colorAttachment) const;
    bool hasEnabledColorAttachment() const;
    size_t getNumColorBuffers() const;
    bool hasStencil() const;
    int getSamples(const gl::Data &data) const;
    bool usingExtendedDrawBuffers() const;

    GLenum checkStatus(const gl::Data &data) const;
    bool hasValidDepthStencil() const;

    Error discard(size_t count, const GLenum *attachments);
    Error invalidate(size_t count, const GLenum *attachments);
    Error invalidateSub(size_t count, const GLenum *attachments, const gl::Rectangle &area);

    Error clear(Context *context, GLbitfield mask);
    Error clearBufferfv(Context *context, GLenum buffer, GLint drawbuffer, const GLfloat *values);
    Error clearBufferuiv(Context *context, GLenum buffer, GLint drawbuffer, const GLuint *values);
    Error clearBufferiv(Context *context, GLenum buffer, GLint drawbuffer, const GLint *values);
    Error clearBufferfi(Context *context,
                        GLenum buffer,
                        GLint drawbuffer,
                        GLfloat depth,
                        GLint stencil);

    GLenum getImplementationColorReadFormat() const;
    GLenum getImplementationColorReadType() const;
    Error readPixels(Context *context,
                     const gl::Rectangle &area,
                     GLenum format,
                     GLenum type,
                     GLvoid *pixels) const;

    Error blit(const gl::State &state, const gl::Rectangle &sourceArea, const gl::Rectangle &destArea,
               GLbitfield mask, GLenum filter, const gl::Framebuffer *sourceFramebuffer);

  protected:
    void detachResourceById(GLenum resourceType, GLuint resourceId);

    Data mData;
    rx::FramebufferImpl *mImpl;
    GLuint mId;
};

}

#endif   // LIBANGLE_FRAMEBUFFER_H_
