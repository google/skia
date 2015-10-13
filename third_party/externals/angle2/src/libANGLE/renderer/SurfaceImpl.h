//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SurfaceImpl.h: Implementation methods of egl::Surface

#ifndef LIBANGLE_RENDERER_SURFACEIMPL_H_
#define LIBANGLE_RENDERER_SURFACEIMPL_H_

#include "common/angleutils.h"
#include "libANGLE/Error.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/FramebufferAttachment.h"

namespace egl
{
class Display;
struct Config;
}

namespace rx
{

class FramebufferImpl;

class SurfaceImpl : public FramebufferAttachmentObjectImpl
{
  public:
    SurfaceImpl();
    virtual ~SurfaceImpl();

    virtual egl::Error initialize() = 0;
    virtual FramebufferImpl *createDefaultFramebuffer(const gl::Framebuffer::Data &data) = 0;
    virtual egl::Error swap() = 0;
    virtual egl::Error postSubBuffer(EGLint x, EGLint y, EGLint width, EGLint height) = 0;
    virtual egl::Error querySurfacePointerANGLE(EGLint attribute, void **value) = 0;
    virtual egl::Error bindTexImage(EGLint buffer) = 0;
    virtual egl::Error releaseTexImage(EGLint buffer) = 0;
    virtual void setSwapInterval(EGLint interval) = 0;

    // width and height can change with client window resizing
    virtual EGLint getWidth() const = 0;
    virtual EGLint getHeight() const = 0;

    virtual EGLint isPostSubBufferSupported() const = 0;
    virtual EGLint getSwapBehavior() const = 0;
};

}

#endif // LIBANGLE_RENDERER_SURFACEIMPL_H_

