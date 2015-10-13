//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SurfaceD3D.h: D3D implementation of an EGL surface

#ifndef LIBANGLE_RENDERER_D3D_SURFACED3D_H_
#define LIBANGLE_RENDERER_D3D_SURFACED3D_H_

#include "libANGLE/renderer/SurfaceImpl.h"
#include "libANGLE/renderer/d3d/d3d11/NativeWindow.h"

namespace egl
{
class Surface;
}

namespace rx
{
class SwapChainD3D;
class RendererD3D;

class SurfaceD3D : public SurfaceImpl
{
  public:
    static SurfaceD3D *createFromWindow(RendererD3D *renderer, egl::Display *display, const egl::Config *config,
                                        EGLNativeWindowType window, EGLint fixedSize, EGLint width, EGLint height);
    static SurfaceD3D *createOffscreen(RendererD3D *renderer, egl::Display *display, const egl::Config *config,
                                       EGLClientBuffer shareHandle, EGLint width, EGLint height);
    ~SurfaceD3D() override;
    void releaseSwapChain();

    egl::Error initialize() override;
    FramebufferImpl *createDefaultFramebuffer(const gl::Framebuffer::Data &data) override;

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

    // D3D implementations
    SwapChainD3D *getSwapChain() const;

    egl::Error resetSwapChain();

    // Returns true if swapchain changed due to resize or interval update
    bool checkForOutOfDateSwapChain();

    gl::Error getAttachmentRenderTarget(const gl::FramebufferAttachment::Target &target,
                                        FramebufferAttachmentRenderTarget **rtOut) override;

  private:
    SurfaceD3D(RendererD3D *renderer, egl::Display *display, const egl::Config *config, EGLint width, EGLint height,
               EGLint fixedSize, EGLClientBuffer shareHandle, EGLNativeWindowType window);

    egl::Error swapRect(EGLint x, EGLint y, EGLint width, EGLint height);
    egl::Error resetSwapChain(int backbufferWidth, int backbufferHeight);
    egl::Error resizeSwapChain(int backbufferWidth, int backbufferHeight);

    RendererD3D *mRenderer;
    egl::Display *mDisplay;

    bool mFixedSize;

    GLenum mRenderTargetFormat;
    GLenum mDepthStencilFormat;

    SwapChainD3D *mSwapChain;
    bool mSwapIntervalDirty;

    NativeWindow mNativeWindow;   // Handler for the Window that the surface is created for.
    EGLint mWidth;
    EGLint mHeight;

    EGLint mSwapInterval;

    HANDLE mShareHandle;
};


}

#endif // LIBANGLE_RENDERER_D3D_SURFACED3D_H_
