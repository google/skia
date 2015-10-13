//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// WindowSurfaceWGL.cpp: WGL implementation of egl::Surface for windows

#include "libANGLE/renderer/gl/wgl/WindowSurfaceWGL.h"

#include "common/debug.h"
#include "libANGLE/renderer/gl/RendererGL.h"
#include "libANGLE/renderer/gl/wgl/FunctionsWGL.h"
#include "libANGLE/renderer/gl/wgl/wgl_utils.h"

namespace rx
{

WindowSurfaceWGL::WindowSurfaceWGL(RendererGL *renderer,
                                   EGLNativeWindowType window,
                                   int pixelFormat,
                                   HGLRC wglContext,
                                   const FunctionsWGL *functions)
    : SurfaceGL(renderer),
      mPixelFormat(pixelFormat),
      mWGLContext(wglContext),
      mWindow(window),
      mDeviceContext(nullptr),
      mFunctionsWGL(functions),
      mSwapBehavior(0)
{
}

WindowSurfaceWGL::~WindowSurfaceWGL()
{
    ReleaseDC(mWindow, mDeviceContext);
    mDeviceContext = nullptr;
}

egl::Error WindowSurfaceWGL::initialize()
{
    mDeviceContext = GetDC(mWindow);
    if (!mDeviceContext)
    {
        return egl::Error(EGL_BAD_NATIVE_WINDOW, "Failed to get the device context from the native window, "
                                                 "error: 0x%X.", GetLastError());
    }

    // Require that the pixel format for this window has not been set yet or is equal to the Display's pixel format.
    int windowPixelFormat = GetPixelFormat(mDeviceContext);
    if (windowPixelFormat == 0)
    {
        PIXELFORMATDESCRIPTOR pixelFormatDescriptor = { 0 };
        if (!DescribePixelFormat(mDeviceContext, mPixelFormat, sizeof(pixelFormatDescriptor), &pixelFormatDescriptor))
        {
            return egl::Error(EGL_BAD_NATIVE_WINDOW, "Failed to DescribePixelFormat, error: 0x%X.", GetLastError());
        }

        if (!SetPixelFormat(mDeviceContext, mPixelFormat, &pixelFormatDescriptor))
        {
            return egl::Error(EGL_NOT_INITIALIZED, "Failed to set the pixel format on the device context, "
                                                   "error: 0x%X.", GetLastError());
        }
    }
    else if (windowPixelFormat != mPixelFormat)
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Pixel format of the NativeWindow and NativeDisplayType must match.");
    }

    // Check for the swap behavior of this pixel format
    switch (
        wgl::QueryWGLFormatAttrib(mDeviceContext, mPixelFormat, WGL_SWAP_METHOD_ARB, mFunctionsWGL))
    {
        case WGL_SWAP_COPY_ARB:
            mSwapBehavior = EGL_BUFFER_PRESERVED;
            break;

        case WGL_SWAP_EXCHANGE_ARB:
        case WGL_SWAP_UNDEFINED_ARB:
        default:
            mSwapBehavior = EGL_BUFFER_DESTROYED;
            break;
    }

    return egl::Error(EGL_SUCCESS);
}

egl::Error WindowSurfaceWGL::makeCurrent()
{
    if (!mFunctionsWGL->makeCurrent(mDeviceContext, mWGLContext))
    {
        // TODO: What error type here?
        return egl::Error(EGL_CONTEXT_LOST, "Failed to make the WGL context current.");
    }

    return egl::Error(EGL_SUCCESS);
}

egl::Error WindowSurfaceWGL::swap()
{
    if (!mFunctionsWGL->swapBuffers(mDeviceContext))
    {
        // TODO: What error type here?
        return egl::Error(EGL_CONTEXT_LOST, "Failed to swap buffers on the child window.");
    }

    return egl::Error(EGL_SUCCESS);
}

egl::Error WindowSurfaceWGL::postSubBuffer(EGLint x, EGLint y, EGLint width, EGLint height)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

egl::Error WindowSurfaceWGL::querySurfacePointerANGLE(EGLint attribute, void **value)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

egl::Error WindowSurfaceWGL::bindTexImage(EGLint buffer)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

egl::Error WindowSurfaceWGL::releaseTexImage(EGLint buffer)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

void WindowSurfaceWGL::setSwapInterval(EGLint interval)
{
    if (mFunctionsWGL->swapIntervalEXT)
    {
        mFunctionsWGL->swapIntervalEXT(interval);
    }
}

EGLint WindowSurfaceWGL::getWidth() const
{
    RECT rect;
    if (!GetClientRect(mWindow, &rect))
    {
        return 0;
    }
    return rect.right - rect.left;
}

EGLint WindowSurfaceWGL::getHeight() const
{
    RECT rect;
    if (!GetClientRect(mWindow, &rect))
    {
        return 0;
    }
    return rect.bottom - rect.top;
}

EGLint WindowSurfaceWGL::isPostSubBufferSupported() const
{
    // PostSubBuffer extension not exposed on WGL.
    UNIMPLEMENTED();
    return EGL_FALSE;
}

EGLint WindowSurfaceWGL::getSwapBehavior() const
{
    return mSwapBehavior;
}

}
