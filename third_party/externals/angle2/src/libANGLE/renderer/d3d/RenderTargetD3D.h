//
// Copyright (c) 2012-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RenderTargetD3D.h: Defines an abstract wrapper class to manage IDirect3DSurface9
// and ID3D11View objects belonging to renderbuffers and renderable textures.

#ifndef LIBANGLE_RENDERER_D3D_RENDERTARGETD3D_H_
#define LIBANGLE_RENDERER_D3D_RENDERTARGETD3D_H_

#include "common/angleutils.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/FramebufferAttachment.h"

namespace rx
{

class RenderTargetD3D : public FramebufferAttachmentRenderTarget
{
  public:
    RenderTargetD3D();
    virtual ~RenderTargetD3D();

    virtual GLsizei getWidth() const = 0;
    virtual GLsizei getHeight() const = 0;
    virtual GLsizei getDepth() const = 0;
    virtual GLenum getInternalFormat() const = 0;
    virtual GLsizei getSamples() const = 0;
    gl::Extents getExtents() const { return gl::Extents(getWidth(), getHeight(), getDepth()); }

    virtual unsigned int getSerial() const;
    static unsigned int issueSerials(unsigned int count);

  private:
    const unsigned int mSerial;
    static unsigned int mCurrentSerial;
};

}

#endif // LIBANGLE_RENDERER_D3D_RENDERTARGETD3D_H_
