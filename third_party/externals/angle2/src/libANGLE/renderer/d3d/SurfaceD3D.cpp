//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SurfaceD3D.cpp: D3D implementation of an EGL surface

#include "libANGLE/renderer/d3d/SurfaceD3D.h"

#include "libANGLE/Display.h"
#include "libANGLE/Surface.h"
#include "libANGLE/renderer/d3d/RendererD3D.h"
#include "libANGLE/renderer/d3d/RenderTargetD3D.h"
#include "libANGLE/renderer/d3d/SwapChainD3D.h"

#include <tchar.h>
#include <EGL/eglext.h>
#include <algorithm>

namespace rx
{

SurfaceD3D *SurfaceD3D::createOffscreen(RendererD3D *renderer, egl::Display *display, const egl::Config *config, EGLClientBuffer shareHandle,
                                        EGLint width, EGLint height)
{
    return new SurfaceD3D(renderer, display, config, width, height, EGL_TRUE, shareHandle, NULL);
}

SurfaceD3D *SurfaceD3D::createFromWindow(RendererD3D *renderer, egl::Display *display, const egl::Config *config, EGLNativeWindowType window,
                                         EGLint fixedSize, EGLint width, EGLint height)
{
    return new SurfaceD3D(renderer, display, config, width, height, fixedSize, static_cast<EGLClientBuffer>(0), window);
}

SurfaceD3D::SurfaceD3D(RendererD3D *renderer,
                       egl::Display *display,
                       const egl::Config *config,
                       EGLint width,
                       EGLint height,
                       EGLint fixedSize,
                       EGLClientBuffer shareHandle,
                       EGLNativeWindowType window)
    : SurfaceImpl(),
      mRenderer(renderer),
      mDisplay(display),
      mFixedSize(fixedSize == EGL_TRUE),
      mRenderTargetFormat(config->renderTargetFormat),
      mDepthStencilFormat(config->depthStencilFormat),
      mSwapChain(nullptr),
      mSwapIntervalDirty(true),
      mNativeWindow(window, config),
      mWidth(width),
      mHeight(height),
      mSwapInterval(1),
      mShareHandle(reinterpret_cast<HANDLE *>(shareHandle))
{
}

SurfaceD3D::~SurfaceD3D()
{
    releaseSwapChain();
}

void SurfaceD3D::releaseSwapChain()
{
    SafeDelete(mSwapChain);
}

egl::Error SurfaceD3D::initialize()
{
    if (mNativeWindow.getNativeWindow())
    {
        if (!mNativeWindow.initialize())
        {
            return egl::Error(EGL_BAD_SURFACE);
        }
    }

    egl::Error error = resetSwapChain();
    if (error.isError())
    {
        return error;
    }

    return egl::Error(EGL_SUCCESS);
}

FramebufferImpl *SurfaceD3D::createDefaultFramebuffer(const gl::Framebuffer::Data &data)
{
    return mRenderer->createFramebuffer(data);
}

egl::Error SurfaceD3D::bindTexImage(EGLint)
{
    return egl::Error(EGL_SUCCESS);
}

egl::Error SurfaceD3D::releaseTexImage(EGLint)
{
    return egl::Error(EGL_SUCCESS);
}

egl::Error SurfaceD3D::resetSwapChain()
{
    ASSERT(!mSwapChain);

    int width;
    int height;

    if (!mFixedSize)
    {
        RECT windowRect;
        if (!mNativeWindow.getClientRect(&windowRect))
        {
            ASSERT(false);

            return egl::Error(EGL_BAD_SURFACE, "Could not retrieve the window dimensions");
        }

        width = windowRect.right - windowRect.left;
        height = windowRect.bottom - windowRect.top;
    }
    else
    {
        // non-window surface - size is determined at creation
        width = mWidth;
        height = mHeight;
    }

    mSwapChain = mRenderer->createSwapChain(mNativeWindow, mShareHandle, mRenderTargetFormat, mDepthStencilFormat);
    if (!mSwapChain)
    {
        return egl::Error(EGL_BAD_ALLOC);
    }

    egl::Error error = resetSwapChain(width, height);
    if (error.isError())
    {
        SafeDelete(mSwapChain);
        return error;
    }

    return egl::Error(EGL_SUCCESS);
}

egl::Error SurfaceD3D::resizeSwapChain(int backbufferWidth, int backbufferHeight)
{
    ASSERT(backbufferWidth >= 0 && backbufferHeight >= 0);
    ASSERT(mSwapChain);

    EGLint status = mSwapChain->resize(std::max(1, backbufferWidth), std::max(1, backbufferHeight));

    if (status == EGL_CONTEXT_LOST)
    {
        mDisplay->notifyDeviceLost();
        return egl::Error(status);
    }
    else if (status != EGL_SUCCESS)
    {
        return egl::Error(status);
    }

    mWidth = backbufferWidth;
    mHeight = backbufferHeight;

    return egl::Error(EGL_SUCCESS);
}

egl::Error SurfaceD3D::resetSwapChain(int backbufferWidth, int backbufferHeight)
{
    ASSERT(backbufferWidth >= 0 && backbufferHeight >= 0);
    ASSERT(mSwapChain);

    EGLint status = mSwapChain->reset(std::max(1, backbufferWidth), std::max(1, backbufferHeight), mSwapInterval);

    if (status == EGL_CONTEXT_LOST)
    {
        mRenderer->notifyDeviceLost();
        return egl::Error(status);
    }
    else if (status != EGL_SUCCESS)
    {
        return egl::Error(status);
    }

    mWidth = backbufferWidth;
    mHeight = backbufferHeight;
    mSwapIntervalDirty = false;

    return egl::Error(EGL_SUCCESS);
}

egl::Error SurfaceD3D::swapRect(EGLint x, EGLint y, EGLint width, EGLint height)
{
    if (!mSwapChain)
    {
        return egl::Error(EGL_SUCCESS);
    }

    if (x + width > mWidth)
    {
        width = mWidth - x;
    }

    if (y + height > mHeight)
    {
        height = mHeight - y;
    }

    if (width != 0 && height != 0)
    {
        EGLint status = mSwapChain->swapRect(x, y, width, height);

        if (status == EGL_CONTEXT_LOST)
        {
            mRenderer->notifyDeviceLost();
            return egl::Error(status);
        }
        else if (status != EGL_SUCCESS)
        {
            return egl::Error(status);
        }
    }

    checkForOutOfDateSwapChain();

    return egl::Error(EGL_SUCCESS);
}

bool SurfaceD3D::checkForOutOfDateSwapChain()
{
    RECT client;
    int clientWidth = getWidth();
    int clientHeight = getHeight();
    bool sizeDirty = false;
    if (!mFixedSize && !mNativeWindow.isIconic())
    {
        // The window is automatically resized to 150x22 when it's minimized, but the swapchain shouldn't be resized
        // because that's not a useful size to render to.
        if (!mNativeWindow.getClientRect(&client))
        {
            ASSERT(false);
            return false;
        }

        // Grow the buffer now, if the window has grown. We need to grow now to avoid losing information.
        clientWidth = client.right - client.left;
        clientHeight = client.bottom - client.top;
        sizeDirty = clientWidth != getWidth() || clientHeight != getHeight();
    }

    bool wasDirty = (mSwapIntervalDirty || sizeDirty);

    if (mSwapIntervalDirty)
    {
        resetSwapChain(clientWidth, clientHeight);
    }
    else if (sizeDirty)
    {
        resizeSwapChain(clientWidth, clientHeight);
    }

    return wasDirty;
}

egl::Error SurfaceD3D::swap()
{
    return swapRect(0, 0, mWidth, mHeight);
}

egl::Error SurfaceD3D::postSubBuffer(EGLint x, EGLint y, EGLint width, EGLint height)
{
    return swapRect(x, y, width, height);
}

rx::SwapChainD3D *SurfaceD3D::getSwapChain() const
{
    return mSwapChain;
}

void SurfaceD3D::setSwapInterval(EGLint interval)
{
    if (mSwapInterval == interval)
    {
        return;
    }

    mSwapInterval = interval;
    mSwapIntervalDirty = true;
}

EGLint SurfaceD3D::getWidth() const
{
    return mWidth;
}

EGLint SurfaceD3D::getHeight() const
{
    return mHeight;
}

EGLint SurfaceD3D::isPostSubBufferSupported() const
{
    // post sub buffer is always possible on D3D surfaces
    return EGL_TRUE;
}

EGLint SurfaceD3D::getSwapBehavior() const
{
    return EGL_BUFFER_PRESERVED;
}

egl::Error SurfaceD3D::querySurfacePointerANGLE(EGLint attribute, void **value)
{
    if (attribute == EGL_D3D_TEXTURE_2D_SHARE_HANDLE_ANGLE)
    {
        *value = mSwapChain->getShareHandle();
    }
    else if (attribute == EGL_DXGI_KEYED_MUTEX_ANGLE)
    {
        *value = mSwapChain->getKeyedMutex();
    }
    else UNREACHABLE();

    return egl::Error(EGL_SUCCESS);
}

gl::Error SurfaceD3D::getAttachmentRenderTarget(const gl::FramebufferAttachment::Target &target,
                                                FramebufferAttachmentRenderTarget **rtOut)
{
    if (target.binding() == GL_BACK)
    {
        *rtOut = mSwapChain->getColorRenderTarget();
    }
    else
    {
        *rtOut = mSwapChain->getDepthStencilRenderTarget();
    }
    return gl::Error(GL_NO_ERROR);
}

}
