//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SurfaceWGL.cpp: WGL implementation of egl::Surface

#include "libANGLE/renderer/gl/wgl/PbufferSurfaceWGL.h"

#include "common/debug.h"
#include "libANGLE/renderer/gl/RendererGL.h"
#include "libANGLE/renderer/gl/wgl/FunctionsWGL.h"
#include "libANGLE/renderer/gl/wgl/wgl_utils.h"

namespace rx
{

PbufferSurfaceWGL::PbufferSurfaceWGL(RendererGL *renderer,
                                     EGLint width,
                                     EGLint height,
                                     EGLenum textureFormat,
                                     EGLenum textureTarget,
                                     bool largest,
                                     int pixelFormat,
                                     HDC deviceContext,
                                     HGLRC wglContext,
                                     const FunctionsWGL *functions)
    : SurfaceGL(renderer),
      mWidth(width),
      mHeight(height),
      mLargest(largest),
      mTextureFormat(textureFormat),
      mTextureTarget(textureTarget),
      mPixelFormat(pixelFormat),
      mShareWGLContext(wglContext),
      mParentDeviceContext(deviceContext),
      mPbuffer(nullptr),
      mPbufferDeviceContext(nullptr),
      mFunctionsWGL(functions)
{
}

PbufferSurfaceWGL::~PbufferSurfaceWGL()
{
    mFunctionsWGL->releasePbufferDCARB(mPbuffer, mPbufferDeviceContext);
    mPbufferDeviceContext = nullptr;

    mFunctionsWGL->destroyPbufferARB(mPbuffer);
    mPbuffer = nullptr;
}

static int GetWGLTextureType(EGLenum eglTextureType)
{
    switch (eglTextureType)
    {
      case EGL_NO_TEXTURE:    return WGL_NO_TEXTURE_ARB;
      case EGL_TEXTURE_RGB:   return WGL_TEXTURE_RGB_ARB;
      case EGL_TEXTURE_RGBA:  return WGL_TEXTURE_RGBA_ARB;
      default: UNREACHABLE(); return 0;
    }
}

static int GetWGLTextureTarget(EGLenum eglTextureTarget)
{
    switch (eglTextureTarget)
    {
      case EGL_NO_TEXTURE:    return WGL_NO_TEXTURE_ARB;
      case EGL_TEXTURE_2D:    return WGL_TEXTURE_2D_ARB;
      default: UNREACHABLE(); return 0;
    }
}

egl::Error PbufferSurfaceWGL::initialize()
{
    const int pbufferCreationAttributes[] =
    {
        WGL_PBUFFER_LARGEST_ARB, mLargest ? 1 : 0,
        WGL_TEXTURE_FORMAT_ARB, GetWGLTextureType(mTextureFormat),
        WGL_TEXTURE_TARGET_ARB, GetWGLTextureTarget(mTextureTarget),
        0, 0,
    };

    mPbuffer = mFunctionsWGL->createPbufferARB(mParentDeviceContext, mPixelFormat, mWidth, mHeight,
                                               pbufferCreationAttributes);
    if (mPbuffer == nullptr)
    {
        DWORD error = GetLastError();
        return egl::Error(EGL_BAD_ALLOC, "Failed to create a native WGL pbuffer, error: 0x%08x.", HRESULT_CODE(error));
    }

    // The returned pbuffer may not be as large as requested, update the size members.
    if (mFunctionsWGL->queryPbufferARB(mPbuffer, WGL_PBUFFER_WIDTH_ARB, &mWidth) != TRUE ||
        mFunctionsWGL->queryPbufferARB(mPbuffer, WGL_PBUFFER_HEIGHT_ARB, &mHeight) != TRUE)
    {
        DWORD error = GetLastError();
        return egl::Error(EGL_BAD_ALLOC, "Failed to query the WGL pbuffer's dimensions, error: 0x%08x.", HRESULT_CODE(error));
    }

    mPbufferDeviceContext = mFunctionsWGL->getPbufferDCARB(mPbuffer);
    if (mPbufferDeviceContext == nullptr)
    {
        mFunctionsWGL->destroyPbufferARB(mPbuffer);
        mPbuffer = nullptr;

        DWORD error = GetLastError();
        return egl::Error(EGL_BAD_ALLOC, "Failed to get the WGL pbuffer handle, error: 0x%08x.", HRESULT_CODE(error));
    }

    return egl::Error(EGL_SUCCESS);
}

egl::Error PbufferSurfaceWGL::makeCurrent()
{
    if (!mFunctionsWGL->makeCurrent(mPbufferDeviceContext, mShareWGLContext))
    {
        // TODO: What error type here?
        return egl::Error(EGL_CONTEXT_LOST, "Failed to make the WGL context current.");
    }

    return egl::Error(EGL_SUCCESS);
}

egl::Error PbufferSurfaceWGL::swap()
{
    return egl::Error(EGL_SUCCESS);
}

egl::Error PbufferSurfaceWGL::postSubBuffer(EGLint x, EGLint y, EGLint width, EGLint height)
{
    return egl::Error(EGL_SUCCESS);
}

egl::Error PbufferSurfaceWGL::querySurfacePointerANGLE(EGLint attribute, void **value)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

static int GetWGLBufferBindTarget(EGLint buffer)
{
    switch (buffer)
    {
      case EGL_BACK_BUFFER:   return WGL_BACK_LEFT_ARB;
      default: UNREACHABLE(); return 0;
    }
}

egl::Error PbufferSurfaceWGL::bindTexImage(EGLint buffer)
{
    if (!mFunctionsWGL->bindTexImageARB(mPbuffer, GetWGLBufferBindTarget(buffer)))
    {
        DWORD error = GetLastError();
        return egl::Error(EGL_BAD_SURFACE, "Failed to bind native wgl pbuffer, error: 0x%08x.", HRESULT_CODE(error));
    }

    return egl::Error(EGL_SUCCESS);
}

egl::Error PbufferSurfaceWGL::releaseTexImage(EGLint buffer)
{
    if (!mFunctionsWGL->releaseTexImageARB(mPbuffer, GetWGLBufferBindTarget(buffer)))
    {
        DWORD error = GetLastError();
        return egl::Error(EGL_BAD_SURFACE, "Failed to unbind native wgl pbuffer, error: 0x%08x.", HRESULT_CODE(error));
    }

    return egl::Error(EGL_SUCCESS);
}

void PbufferSurfaceWGL::setSwapInterval(EGLint interval)
{
}

EGLint PbufferSurfaceWGL::getWidth() const
{
    return mWidth;
}

EGLint PbufferSurfaceWGL::getHeight() const
{
    return mHeight;
}

EGLint PbufferSurfaceWGL::isPostSubBufferSupported() const
{
    return EGL_FALSE;
}

EGLint PbufferSurfaceWGL::getSwapBehavior() const
{
    return EGL_BUFFER_PRESERVED;
}

}
