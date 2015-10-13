//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Image.h: Implements the rx::Image class, an abstract base class for the
// renderer-specific classes which will define the interface to the underlying
// surfaces or resources.

#include "libANGLE/renderer/d3d/ImageD3D.h"

#include "libANGLE/Framebuffer.h"
#include "libANGLE/FramebufferAttachment.h"
#include "libANGLE/renderer/d3d/FramebufferD3D.h"
#include "libANGLE/renderer/d3d/RenderTargetD3D.h"

namespace rx
{

ImageD3D::ImageD3D()
    : mWidth(0),
      mHeight(0),
      mDepth(0),
      mInternalFormat(GL_NONE),
      mRenderable(false),
      mTarget(GL_NONE),
      mDirty(false)
{
}

gl::Error ImageD3D::copy(const gl::Offset &destOffset, const gl::Rectangle &sourceArea, const gl::Framebuffer *source)
{
    const gl::FramebufferAttachment *srcAttachment = source->getReadColorbuffer();
    ASSERT(srcAttachment);

    RenderTargetD3D *renderTarget = NULL;
    gl::Error error = srcAttachment->getRenderTarget(&renderTarget);
    if (error.isError())
    {
        return error;
    }

    ASSERT(renderTarget);
    return copy(destOffset, sourceArea, renderTarget);
}

}
