//
// Copyright (c) 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SwapChainD3D.h: Defines a back-end specific class that hides the details of the
// implementation-specific swapchain.

#ifndef LIBANGLE_RENDERER_D3D_SWAPCHAIND3D_H_
#define LIBANGLE_RENDERER_D3D_SWAPCHAIND3D_H_

#include <GLES2/gl2.h>
#include <EGL/egl.h>

#include "common/angleutils.h"
#include "common/platform.h"

// TODO: move out of D3D11
#include "libANGLE/renderer/d3d/d3d11/NativeWindow.h"

#if !defined(ANGLE_FORCE_VSYNC_OFF)
#define ANGLE_FORCE_VSYNC_OFF 0
#endif

namespace rx
{
class RenderTargetD3D;

class SwapChainD3D : angle::NonCopyable
{
  public:
    SwapChainD3D(rx::NativeWindow nativeWindow, HANDLE shareHandle, GLenum backBufferFormat, GLenum depthBufferFormat)
        : mNativeWindow(nativeWindow), mOffscreenRenderTargetFormat(backBufferFormat), mDepthBufferFormat(depthBufferFormat), mShareHandle(shareHandle)
    {
    }

    virtual ~SwapChainD3D() {};

    virtual EGLint resize(EGLint backbufferWidth, EGLint backbufferSize) = 0;
    virtual EGLint reset(EGLint backbufferWidth, EGLint backbufferHeight, EGLint swapInterval) = 0;
    virtual EGLint swapRect(EGLint x, EGLint y, EGLint width, EGLint height) = 0;
    virtual void recreate() = 0;

    virtual RenderTargetD3D *getColorRenderTarget() = 0;
    virtual RenderTargetD3D *getDepthStencilRenderTarget() = 0;

    GLenum GetRenderTargetInternalFormat() const { return mOffscreenRenderTargetFormat; }
    GLenum GetDepthBufferInternalFormat() const { return mDepthBufferFormat; }

    HANDLE getShareHandle() { return mShareHandle; }
    virtual void *getKeyedMutex() = 0;

  protected:
    rx::NativeWindow mNativeWindow;  // Handler for the Window that the surface is created for.
    const GLenum mOffscreenRenderTargetFormat;
    const GLenum mDepthBufferFormat;

    HANDLE mShareHandle;
};

}
#endif // LIBANGLE_RENDERER_D3D_SWAPCHAIND3D_H_
