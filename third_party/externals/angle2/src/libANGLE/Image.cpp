//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Image.cpp: Implements the egl::Image class representing the EGLimage object.

#include "libANGLE/Image.h"

#include "common/debug.h"
#include "common/utilities.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/Texture.h"
#include "libANGLE/Renderbuffer.h"
#include "libANGLE/renderer/ImageImpl.h"

namespace egl
{
ImageSibling::ImageSibling(GLuint id) : RefCountObject(id), mSourcesOf(), mTargetOf()
{
}

ImageSibling::~ImageSibling()
{
    // EGL images should hold a ref to their targets and siblings, a Texture should not be deletable
    // while it is attached to an EGL image.
    ASSERT(mSourcesOf.empty());
    orphanImages();
}

void ImageSibling::setTargetImage(egl::Image *imageTarget)
{
    ASSERT(imageTarget != nullptr);
    mTargetOf.set(imageTarget);
    imageTarget->addTargetSibling(this);
}

gl::Error ImageSibling::orphanImages()
{
    if (mTargetOf.get() != nullptr)
    {
        // Can't be a target and have sources.
        ASSERT(mSourcesOf.empty());

        gl::Error error = mTargetOf->orphanSibling(this);
        if (error.isError())
        {
            return error;
        }

        mTargetOf.set(nullptr);
    }
    else
    {
        for (auto &sourceImage : mSourcesOf)
        {
            gl::Error error = sourceImage->orphanSibling(this);
            if (error.isError())
            {
                return error;
            }
        }
        mSourcesOf.clear();
    }

    return gl::Error(GL_NO_ERROR);
}

void ImageSibling::addImageSource(egl::Image *imageSource)
{
    ASSERT(imageSource != nullptr);
    mSourcesOf.insert(imageSource);
}

void ImageSibling::removeImageSource(egl::Image *imageSource)
{
    ASSERT(mSourcesOf.find(imageSource) != mSourcesOf.end());
    mSourcesOf.erase(imageSource);
}

Image::Image(rx::ImageImpl *impl, EGLenum target, ImageSibling *buffer, const AttributeMap &attribs)
    : RefCountObject(0),
      mImplementation(impl),
      mInternalFormat(GL_NONE),
      mWidth(0),
      mHeight(0),
      mSamples(0),
      mSource(),
      mTargets()
{
    ASSERT(mImplementation != nullptr);
    ASSERT(buffer != nullptr);

    mSource.set(buffer);
    mSource->addImageSource(this);

    if (IsTextureTarget(target))
    {
        gl::Texture *texture = rx::GetAs<gl::Texture>(mSource.get());
        GLenum textureTarget = egl_gl::EGLImageTargetToGLTextureTarget(target);
        size_t level         = attribs.get(EGL_GL_TEXTURE_LEVEL_KHR, 0);
        mInternalFormat      = texture->getInternalFormat(textureTarget, level);
        mWidth               = texture->getWidth(textureTarget, level);
        mHeight              = texture->getHeight(textureTarget, level);
        mSamples             = 0;
    }
    else if (IsRenderbufferTarget(target))
    {
        gl::Renderbuffer *renderbuffer = rx::GetAs<gl::Renderbuffer>(mSource.get());
        mInternalFormat                = renderbuffer->getInternalFormat();
        mWidth                         = renderbuffer->getWidth();
        mHeight                        = renderbuffer->getHeight();
        mSamples                       = renderbuffer->getSamples();
    }
    else
    {
        UNREACHABLE();
    }
}

Image::~Image()
{
    SafeDelete(mImplementation);

    // All targets should hold a ref to the egl image and it should not be deleted until there are
    // no siblings left.
    ASSERT(mTargets.empty());

    // Tell the source that it is no longer used by this image
    if (mSource.get() != nullptr)
    {
        mSource->removeImageSource(this);
        mSource.set(nullptr);
    }
}

void Image::addTargetSibling(ImageSibling *sibling)
{
    mTargets.insert(sibling);
}

gl::Error Image::orphanSibling(ImageSibling *sibling)
{
    // notify impl
    gl::Error error = mImplementation->orphan(sibling);

    if (mSource.get() == sibling)
    {
        // If the sibling is the source, it cannot be a target.
        ASSERT(mTargets.find(sibling) == mTargets.end());

        mSource.set(nullptr);
    }
    else
    {
        mTargets.erase(sibling);
    }

    return error;
}

GLenum Image::getInternalFormat() const
{
    return mInternalFormat;
}

size_t Image::getWidth() const
{
    return mWidth;
}

size_t Image::getHeight() const
{
    return mHeight;
}

size_t Image::getSamples() const
{
    return mSamples;
}

rx::ImageImpl *Image::getImplementation()
{
    return mImplementation;
}

const rx::ImageImpl *Image::getImplementation() const
{
    return mImplementation;
}
}
