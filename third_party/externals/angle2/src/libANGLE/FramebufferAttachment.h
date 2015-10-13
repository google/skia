//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FramebufferAttachment.h: Defines the wrapper class gl::FramebufferAttachment, as well as the
// objects and related functionality. [OpenGL ES 2.0.24] section 4.4.3 page 108.

#ifndef LIBANGLE_FRAMEBUFFERATTACHMENT_H_
#define LIBANGLE_FRAMEBUFFERATTACHMENT_H_

#include "angle_gl.h"
#include "common/angleutils.h"
#include "libANGLE/Error.h"
#include "libANGLE/ImageIndex.h"

namespace egl
{
class Surface;
}

namespace rx
{
// An implementation-specific object associated with an attachment.

class FramebufferAttachmentRenderTarget : angle::NonCopyable
{
  public:
    FramebufferAttachmentRenderTarget() {}
    virtual ~FramebufferAttachmentRenderTarget() {}
};

class FramebufferAttachmentObjectImpl;
}

namespace gl
{
class FramebufferAttachmentObject;
class Renderbuffer;
class Texture;

// FramebufferAttachment implements a GL framebuffer attachment.
// Attachments are "light" containers, which store pointers to ref-counted GL objects.
// We support GL texture (2D/3D/Cube/2D array) and renderbuffer object attachments.
// Note: Our old naming scheme used the term "Renderbuffer" for both GL renderbuffers and for
// framebuffer attachments, which confused their usage.

class FramebufferAttachment final
{
  public:
    FramebufferAttachment();

    FramebufferAttachment(GLenum type,
                          GLenum binding,
                          const ImageIndex &textureIndex,
                          FramebufferAttachmentObject *resource);

    FramebufferAttachment(const FramebufferAttachment &other);
    FramebufferAttachment &operator=(const FramebufferAttachment &other);

    ~FramebufferAttachment();

    // A framebuffer attachment points to one of three types of resources: Renderbuffers,
    // Textures and egl::Surface. The "Target" struct indicates which part of the
    // object an attachment references. For the three types:
    //   - a Renderbuffer has a unique renderable target, and needs no target index
    //   - a Texture has targets for every image and uses an ImageIndex
    //   - a Surface has targets for Color and Depth/Stencil, and uses the attachment binding
    class Target
    {
      public:
        Target();
        Target(GLenum binding, const ImageIndex &imageIndex);
        Target(const Target &other);
        Target &operator=(const Target &other);

        GLenum binding() const { return mBinding; }
        const ImageIndex &textureIndex() const { return mTextureIndex; }

      private:
        GLenum mBinding;
        ImageIndex mTextureIndex;
    };

    void detach();
    void attach(GLenum type,
                GLenum binding,
                const ImageIndex &textureIndex,
                FramebufferAttachmentObject *resource);

    // Helper methods
    GLuint getRedSize() const;
    GLuint getGreenSize() const;
    GLuint getBlueSize() const;
    GLuint getAlphaSize() const;
    GLuint getDepthSize() const;
    GLuint getStencilSize() const;
    GLenum getComponentType() const;
    GLenum getColorEncoding() const;

    bool isTextureWithId(GLuint textureId) const { return mType == GL_TEXTURE && id() == textureId; }
    bool isRenderbufferWithId(GLuint renderbufferId) const { return mType == GL_RENDERBUFFER && id() == renderbufferId; }

    GLenum getBinding() const { return mTarget.binding(); }
    GLuint id() const;

    // These methods are only legal to call on Texture attachments
    const ImageIndex &getTextureImageIndex() const;
    GLenum cubeMapFace() const;
    GLint mipLevel() const;
    GLint layer() const;

    GLsizei getWidth() const;
    GLsizei getHeight() const;
    GLenum getInternalFormat() const;
    GLsizei getSamples() const;
    GLenum type() const { return mType; }
    bool isAttached() const { return mType != GL_NONE; }

    Renderbuffer *getRenderbuffer() const;
    Texture *getTexture() const;
    const egl::Surface *getSurface() const;

    // "T" must be static_castable from FramebufferAttachmentRenderTarget
    template <typename T>
    gl::Error getRenderTarget(T **rtOut) const
    {
        // Cast through the pointer-to-pointer type
        rx::FramebufferAttachmentRenderTarget *rtPtr = nullptr;
        gl::Error error = getRenderTarget(&rtPtr);
        *rtOut = static_cast<T*>(rtPtr);
        return error;
    }

  private:
    gl::Error getRenderTarget(rx::FramebufferAttachmentRenderTarget **rtOut) const;

    GLenum mType;
    Target mTarget;
    FramebufferAttachmentObject *mResource;
};

// A base class for objects that FBO Attachments may point to.
class FramebufferAttachmentObject
{
  public:
    FramebufferAttachmentObject() {}
    virtual ~FramebufferAttachmentObject() {}

    virtual GLsizei getAttachmentWidth(const FramebufferAttachment::Target &target) const = 0;
    virtual GLsizei getAttachmentHeight(const FramebufferAttachment::Target &target) const = 0;
    virtual GLenum getAttachmentInternalFormat(const FramebufferAttachment::Target &target) const = 0;
    virtual GLsizei getAttachmentSamples(const FramebufferAttachment::Target &target) const = 0;

    virtual void onAttach() = 0;
    virtual void onDetach() = 0;
    virtual GLuint getId() const = 0;

    Error getAttachmentRenderTarget(const FramebufferAttachment::Target &target,
                                    rx::FramebufferAttachmentRenderTarget **rtOut) const;

  protected:
    virtual rx::FramebufferAttachmentObjectImpl *getAttachmentImpl() const = 0;
};

inline GLsizei FramebufferAttachment::getWidth() const
{
    return mResource->getAttachmentWidth(mTarget);
}

inline GLsizei FramebufferAttachment::getHeight() const
{
    return mResource->getAttachmentHeight(mTarget);
}

inline GLenum FramebufferAttachment::getInternalFormat() const
{
    return mResource->getAttachmentInternalFormat(mTarget);
}

inline GLsizei FramebufferAttachment::getSamples() const
{
    return mResource->getAttachmentSamples(mTarget);
}

inline gl::Error FramebufferAttachment::getRenderTarget(rx::FramebufferAttachmentRenderTarget **rtOut) const
{
    return mResource->getAttachmentRenderTarget(mTarget, rtOut);
}

} // namespace gl

namespace rx
{

class FramebufferAttachmentObjectImpl : angle::NonCopyable
{
  public:
    FramebufferAttachmentObjectImpl() {}
    virtual ~FramebufferAttachmentObjectImpl() {}

    virtual gl::Error getAttachmentRenderTarget(const gl::FramebufferAttachment::Target &target,
                                                FramebufferAttachmentRenderTarget **rtOut) = 0;
};

} // namespace rx

namespace gl
{

inline Error FramebufferAttachmentObject::getAttachmentRenderTarget(
    const FramebufferAttachment::Target &target,
    rx::FramebufferAttachmentRenderTarget **rtOut) const
{
    return getAttachmentImpl()->getAttachmentRenderTarget(target, rtOut);
}

}

#endif // LIBANGLE_FRAMEBUFFERATTACHMENT_H_
