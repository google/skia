//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RenderbufferD3d.cpp: Implements the RenderbufferD3D class, a specialization of RenderbufferImpl


#include "libANGLE/renderer/d3d/RenderbufferD3D.h"

#include "libANGLE/Image.h"
#include "libANGLE/renderer/d3d/EGLImageD3D.h"
#include "libANGLE/renderer/d3d/RendererD3D.h"
#include "libANGLE/renderer/d3d/RenderTargetD3D.h"

namespace rx
{
RenderbufferD3D::RenderbufferD3D(RendererD3D *renderer)
    : mRenderer(renderer), mRenderTarget(nullptr), mImage(nullptr)
{
}

RenderbufferD3D::~RenderbufferD3D()
{
    SafeDelete(mRenderTarget);
    mImage = nullptr;
}

gl::Error RenderbufferD3D::setStorage(GLenum internalformat, size_t width, size_t height)
{
    return setStorageMultisample(0, internalformat, width, height);
}

gl::Error RenderbufferD3D::setStorageMultisample(size_t samples, GLenum internalformat, size_t width, size_t height)
{
    // If the renderbuffer parameters are queried, the calling function
    // will expect one of the valid renderbuffer formats for use in
    // glRenderbufferStorage, but we should create depth and stencil buffers
    // as DEPTH24_STENCIL8
    GLenum creationFormat = internalformat;
    if (internalformat == GL_DEPTH_COMPONENT16 || internalformat == GL_STENCIL_INDEX8)
    {
        creationFormat = GL_DEPTH24_STENCIL8_OES;
    }

    // ANGLE_framebuffer_multisample states GL_OUT_OF_MEMORY is generated on a failure to create
    // the specified storage.
    // Because ES 3.0 already knows the exact number of supported samples, it would already have been
    // validated and generated GL_INVALID_VALUE.
    const gl::TextureCaps &formatCaps = mRenderer->getRendererTextureCaps().get(creationFormat);
    if (samples > formatCaps.getMaxSamples())
    {
        return gl::Error(GL_OUT_OF_MEMORY, "Renderbuffer format does not support %u samples, %u is the maximum.",
                         samples, formatCaps.getMaxSamples());
    }

    RenderTargetD3D *newRT = NULL;
    gl::Error error =
        mRenderer->createRenderTarget(static_cast<int>(width), static_cast<int>(height),
                                      creationFormat, static_cast<GLsizei>(samples), &newRT);
    if (error.isError())
    {
        return error;
    }

    SafeDelete(mRenderTarget);
    mImage        = nullptr;
    mRenderTarget = newRT;

    return gl::Error(GL_NO_ERROR);
}

gl::Error RenderbufferD3D::setStorageEGLImageTarget(egl::Image *image)
{
    mImage = GetImplAs<EGLImageD3D>(image);
    SafeDelete(mRenderTarget);

    return gl::Error(GL_NO_ERROR);
}

gl::Error RenderbufferD3D::getRenderTarget(RenderTargetD3D **outRenderTarget)
{
    if (mImage)
    {
        return mImage->getRenderTarget(outRenderTarget);
    }
    else
    {
        *outRenderTarget = mRenderTarget;
        return gl::Error(GL_NO_ERROR);
    }
}

gl::Error RenderbufferD3D::getAttachmentRenderTarget(const gl::FramebufferAttachment::Target &target,
                                                     FramebufferAttachmentRenderTarget **rtOut)
{
    return getRenderTarget(reinterpret_cast<RenderTargetD3D **>(rtOut));
}

}
