//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// WindowSurfaceCGL.h: CGL implementation of egl::Surface for windows

#ifndef LIBANGLE_RENDERER_GL_CGL_WINDOWSURFACECGL_H_
#define LIBANGLE_RENDERER_GL_CGL_WINDOWSURFACECGL_H_

#include "libANGLE/renderer/gl/SurfaceGL.h"

@class CALayer;
struct __IOSurface;
typedef __IOSurface *IOSurfaceRef;

namespace rx
{

class DisplayCGL;
class FramebufferGL;
class FunctionsGL;
class StateManagerGL;
struct WorkaroundsGL;

class DisplayLink;

class WindowSurfaceCGL : public SurfaceGL
{
  public:
    WindowSurfaceCGL(RendererGL *renderer, CALayer *layer, const FunctionsGL *functions);
    ~WindowSurfaceCGL() override;

    egl::Error initialize() override;
    egl::Error makeCurrent() override;

    egl::Error swap() override;
    egl::Error postSubBuffer(EGLint x, EGLint y, EGLint width, EGLint height) override;
    egl::Error querySurfacePointerANGLE(EGLint attribute, void **value) override;
    egl::Error bindTexImage(EGLint buffer) override;
    egl::Error releaseTexImage(EGLint buffer) override;
    void setSwapInterval(EGLint interval) override;

    EGLint getWidth() const override;
    EGLint getHeight() const override;

    EGLint isPostSubBufferSupported() const override;
    EGLint getSwapBehavior() const override;

    FramebufferImpl *createDefaultFramebuffer(const gl::Framebuffer::Data &data) override;

  private:
    struct Surface
    {
        IOSurfaceRef ioSurface;
        GLuint texture;
        uint64_t lastPresentNanos;
    };

    void freeSurfaceData(Surface *surface);
    egl::Error initializeSurfaceData(Surface *surface, int width, int height);

    CALayer *mLayer;
    const FunctionsGL *mFunctions;
    StateManagerGL *mStateManager;
    const WorkaroundsGL &mWorkarounds;
    DisplayLink *mDisplayLink;

    // CGL doesn't have a default framebuffer, we instead render to an IOSurface
    // that will be set as the content of the CALayer which is our native window.
    // We use two IOSurfaces to do double buffering.
    Surface mSurfaces[2];
    int mCurrentSurface;
    GLuint mFramebuffer;
    GLuint mDSRenderbuffer;
};

}

#endif // LIBANGLE_RENDERER_GL_CGL_WINDOWSURFACECGL_H_
