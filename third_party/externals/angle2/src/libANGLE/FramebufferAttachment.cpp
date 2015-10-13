//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FramebufferAttachment.cpp: the gl::FramebufferAttachment class and its derived classes
// objects and related functionality. [OpenGL ES 2.0.24] section 4.4.3 page 108.

#include "libANGLE/FramebufferAttachment.h"

#include "common/utilities.h"
#include "libANGLE/Config.h"
#include "libANGLE/Renderbuffer.h"
#include "libANGLE/Surface.h"
#include "libANGLE/Texture.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/FramebufferImpl.h"

namespace gl
{

////// FramebufferAttachment::Target Implementation //////

FramebufferAttachment::Target::Target()
    : mBinding(GL_NONE),
      mTextureIndex(ImageIndex::MakeInvalid())
{
}

FramebufferAttachment::Target::Target(GLenum binding, const ImageIndex &imageIndex)
    : mBinding(binding),
      mTextureIndex(imageIndex)
{
}

FramebufferAttachment::Target::Target(const Target &other)
    : mBinding(other.mBinding),
      mTextureIndex(other.mTextureIndex)
{
}

FramebufferAttachment::Target &FramebufferAttachment::Target::operator=(const Target &other)
{
    this->mBinding = other.mBinding;
    this->mTextureIndex = other.mTextureIndex;
    return *this;
}

////// FramebufferAttachment Implementation //////

FramebufferAttachment::FramebufferAttachment()
    : mType(GL_NONE), mResource(nullptr)
{
}

FramebufferAttachment::FramebufferAttachment(GLenum type,
                                             GLenum binding,
                                             const ImageIndex &textureIndex,
                                             FramebufferAttachmentObject *resource)
    : mResource(nullptr)
{
    attach(type, binding, textureIndex, resource);
}

FramebufferAttachment::FramebufferAttachment(const FramebufferAttachment &other)
    : mResource(nullptr)
{
    attach(other.mType, other.mTarget.binding(), other.mTarget.textureIndex(), other.mResource);
}

FramebufferAttachment &FramebufferAttachment::operator=(const FramebufferAttachment &other)
{
    attach(other.mType, other.mTarget.binding(), other.mTarget.textureIndex(), other.mResource);
    return *this;
}

FramebufferAttachment::~FramebufferAttachment()
{
    detach();
}

void FramebufferAttachment::detach()
{
    mType = GL_NONE;
    if (mResource != nullptr)
    {
        mResource->onDetach();
        mResource = nullptr;
    }

    // not technically necessary, could omit for performance
    mTarget = Target();
}

void FramebufferAttachment::attach(GLenum type,
                                   GLenum binding,
                                   const ImageIndex &textureIndex,
                                   FramebufferAttachmentObject *resource)
{
    mType = type;
    mTarget = Target(binding, textureIndex);

    if (resource)
    {
        resource->onAttach();
    }
    if (mResource != nullptr)
    {
        mResource->onDetach();
    }
    mResource = resource;
}

GLuint FramebufferAttachment::getRedSize() const
{
    return GetInternalFormatInfo(getInternalFormat()).redBits;
}

GLuint FramebufferAttachment::getGreenSize() const
{
    return GetInternalFormatInfo(getInternalFormat()).greenBits;
}

GLuint FramebufferAttachment::getBlueSize() const
{
    return GetInternalFormatInfo(getInternalFormat()).blueBits;
}

GLuint FramebufferAttachment::getAlphaSize() const
{
    return GetInternalFormatInfo(getInternalFormat()).alphaBits;
}

GLuint FramebufferAttachment::getDepthSize() const
{
    return GetInternalFormatInfo(getInternalFormat()).depthBits;
}

GLuint FramebufferAttachment::getStencilSize() const
{
    return GetInternalFormatInfo(getInternalFormat()).stencilBits;
}

GLenum FramebufferAttachment::getComponentType() const
{
    return GetInternalFormatInfo(getInternalFormat()).componentType;
}

GLenum FramebufferAttachment::getColorEncoding() const
{
    return GetInternalFormatInfo(getInternalFormat()).colorEncoding;
}

GLuint FramebufferAttachment::id() const
{
    return mResource->getId();
}

const ImageIndex &FramebufferAttachment::getTextureImageIndex() const
{
    ASSERT(type() == GL_TEXTURE);
    return mTarget.textureIndex();
}

GLenum FramebufferAttachment::cubeMapFace() const
{
    ASSERT(mType == GL_TEXTURE);

    const auto &index = mTarget.textureIndex();
    return IsCubeMapTextureTarget(index.type) ? index.type : GL_NONE;
}

GLint FramebufferAttachment::mipLevel() const
{
    ASSERT(type() == GL_TEXTURE);
    return mTarget.textureIndex().mipIndex;
}

GLint FramebufferAttachment::layer() const
{
    ASSERT(mType == GL_TEXTURE);

    const auto &index = mTarget.textureIndex();

    if (index.type == GL_TEXTURE_2D_ARRAY || index.type == GL_TEXTURE_3D)
    {
        return index.layerIndex;
    }
    return 0;
}

Texture *FramebufferAttachment::getTexture() const
{
    return rx::GetAs<Texture>(mResource);
}

Renderbuffer *FramebufferAttachment::getRenderbuffer() const
{
    return rx::GetAs<Renderbuffer>(mResource);
}

const egl::Surface *FramebufferAttachment::getSurface() const
{
    return rx::GetAs<egl::Surface>(mResource);
}

}
