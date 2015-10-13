//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// EGLImageD3D.cpp: Implements the rx::EGLImageD3D class, the D3D implementation of EGL images

#include "libANGLE/renderer/d3d/EGLImageD3D.h"

#include "common/debug.h"
#include "common/utilities.h"
#include "libANGLE/AttributeMap.h"
#include "libANGLE/Texture.h"
#include "libANGLE/renderer/d3d/RenderbufferD3D.h"
#include "libANGLE/renderer/d3d/RendererD3D.h"
#include "libANGLE/renderer/d3d/RenderTargetD3D.h"
#include "libANGLE/renderer/d3d/TextureD3D.h"
#include "libANGLE/renderer/d3d/TextureStorage.h"

#include <EGL/eglext.h>

namespace rx
{
static gl::ImageIndex GetImageIndex(GLenum target, size_t mip, size_t layer)
{
    if (target == GL_TEXTURE_3D)
    {
        return gl::ImageIndex::Make3D(static_cast<GLint>(mip), static_cast<GLint>(layer));
    }
    else
    {
        ASSERT(layer == 0);
        return gl::ImageIndex::MakeGeneric(target, static_cast<GLint>(mip));
    }
}

EGLImageD3D::EGLImageD3D(RendererD3D *renderer,
                         EGLenum target,
                         egl::ImageSibling *buffer,
                         const egl::AttributeMap &attribs)
    : mRenderer(renderer), mBuffer(buffer), mAttachmentBuffer(nullptr), mRenderTarget(nullptr)
{
    ASSERT(renderer != nullptr);
    ASSERT(buffer != nullptr);

    if (egl::IsTextureTarget(target))
    {
        mAttachmentBuffer = GetImplAs<TextureD3D>(GetAs<gl::Texture>(buffer));
        mAttachmentTarget = gl::FramebufferAttachment::Target(
            GL_NONE, GetImageIndex(egl_gl::EGLImageTargetToGLTextureTarget(target),
                                   attribs.get(EGL_GL_TEXTURE_LEVEL_KHR, 0),
                                   attribs.get(EGL_GL_TEXTURE_ZOFFSET_KHR, 0)));
    }
    else if (egl::IsRenderbufferTarget(target))
    {
        mAttachmentBuffer = GetImplAs<RenderbufferD3D>(GetAs<gl::Renderbuffer>(buffer));
        mAttachmentTarget =
            gl::FramebufferAttachment::Target(GL_NONE, gl::ImageIndex::MakeInvalid());
    }
    else
    {
        UNREACHABLE();
    }
}

EGLImageD3D::~EGLImageD3D()
{
    SafeDelete(mRenderTarget);
}

egl::Error EGLImageD3D::initialize()
{
    return egl::Error(EGL_SUCCESS);
}

gl::Error EGLImageD3D::orphan(egl::ImageSibling *sibling)
{
    if (sibling == mBuffer)
    {
        gl::Error error = copyToLocalRendertarget();
        if (error.isError())
        {
            return error;
        }
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error EGLImageD3D::getRenderTarget(RenderTargetD3D **outRT) const
{
    if (mAttachmentBuffer)
    {
        FramebufferAttachmentRenderTarget *rt = nullptr;
        gl::Error error = mAttachmentBuffer->getAttachmentRenderTarget(mAttachmentTarget, &rt);
        if (error.isError())
        {
            return error;
        }

        *outRT = static_cast<RenderTargetD3D *>(rt);
        return gl::Error(GL_NO_ERROR);
    }
    else
    {
        ASSERT(mRenderTarget);
        *outRT = mRenderTarget;
        return gl::Error(GL_NO_ERROR);
    }
}

gl::Error EGLImageD3D::copyToLocalRendertarget()
{
    ASSERT(mBuffer != nullptr);
    ASSERT(mAttachmentBuffer != nullptr);
    ASSERT(mRenderTarget == nullptr);

    RenderTargetD3D *curRenderTarget = nullptr;
    gl::Error error = getRenderTarget(&curRenderTarget);
    if (error.isError())
    {
        return error;
    }

    // Clear the source image buffers
    mBuffer           = nullptr;
    mAttachmentBuffer = nullptr;

    return mRenderer->createRenderTargetCopy(curRenderTarget, &mRenderTarget);
}
}
