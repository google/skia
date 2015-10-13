//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include <string.h>
#include <cassert>
#include <vector>

#include "EGLWindow.h"
#include "OSWindow.h"
#include "common/debug.h"

EGLPlatformParameters::EGLPlatformParameters()
    : renderer(EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE),
      majorVersion(EGL_DONT_CARE),
      minorVersion(EGL_DONT_CARE),
      deviceType(EGL_DONT_CARE)
{
}

EGLPlatformParameters::EGLPlatformParameters(EGLint renderer)
    : renderer(renderer),
      majorVersion(EGL_DONT_CARE),
      minorVersion(EGL_DONT_CARE),
      deviceType(EGL_DONT_CARE)
{
    if (renderer == EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE ||
        renderer == EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE)
    {
        deviceType = EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE;
    }
}

EGLPlatformParameters::EGLPlatformParameters(EGLint renderer, EGLint majorVersion, EGLint minorVersion, EGLint useWarp)
    : renderer(renderer),
      majorVersion(majorVersion),
      minorVersion(minorVersion),
      deviceType(useWarp)
{
}

bool operator<(const EGLPlatformParameters &a, const EGLPlatformParameters &b)
{
    if (a.renderer != b.renderer)
    {
        return a.renderer < b.renderer;
    }

    if (a.majorVersion != b.majorVersion)
    {
        return a.majorVersion < b.majorVersion;
    }

    if (a.minorVersion != b.minorVersion)
    {
        return a.minorVersion < b.minorVersion;
    }

    return a.deviceType < b.deviceType;
}

bool operator==(const EGLPlatformParameters &a, const EGLPlatformParameters &b)
{
    return (a.renderer == b.renderer) &&
           (a.majorVersion == b.majorVersion) &&
           (a.minorVersion == b.minorVersion) &&
           (a.deviceType == b.deviceType);
}

EGLWindow::EGLWindow(EGLint glesMajorVersion,
                     EGLint glesMinorVersion,
                     const EGLPlatformParameters &platform)
    : mDisplay(EGL_NO_DISPLAY),
      mSurface(EGL_NO_SURFACE),
      mContext(EGL_NO_CONTEXT),
      mClientMajorVersion(glesMajorVersion),
      mClientMinorVersion(glesMinorVersion),
      mPlatform(platform),
      mRedBits(-1),
      mGreenBits(-1),
      mBlueBits(-1),
      mAlphaBits(-1),
      mDepthBits(-1),
      mStencilBits(-1),
      mMultisample(false),
      mSwapInterval(-1)
{
}

EGLWindow::~EGLWindow()
{
    destroyGL();
}

void EGLWindow::swap()
{
    eglSwapBuffers(mDisplay, mSurface);
}

EGLConfig EGLWindow::getConfig() const
{
    return mConfig;
}

EGLDisplay EGLWindow::getDisplay() const
{
    return mDisplay;
}

EGLSurface EGLWindow::getSurface() const
{
    return mSurface;
}

EGLContext EGLWindow::getContext() const
{
    return mContext;
}

bool EGLWindow::initializeGL(OSWindow *osWindow)
{
    PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(eglGetProcAddress("eglGetPlatformDisplayEXT"));
    if (!eglGetPlatformDisplayEXT)
    {
        return false;
    }

    std::vector<EGLint> displayAttributes;
    displayAttributes.push_back(EGL_PLATFORM_ANGLE_TYPE_ANGLE);
    displayAttributes.push_back(mPlatform.renderer);
    displayAttributes.push_back(EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE);
    displayAttributes.push_back(mPlatform.majorVersion);
    displayAttributes.push_back(EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE);
    displayAttributes.push_back(mPlatform.minorVersion);

    if (mPlatform.deviceType != EGL_DONT_CARE)
    {
        displayAttributes.push_back(EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE);
        displayAttributes.push_back(mPlatform.deviceType);
    }
    displayAttributes.push_back(EGL_NONE);

    mDisplay = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, osWindow->getNativeDisplay(), &displayAttributes[0]);
    if (mDisplay == EGL_NO_DISPLAY)
    {
        destroyGL();
        return false;
    }

    EGLint majorVersion, minorVersion;
    if (eglInitialize(mDisplay, &majorVersion, &minorVersion) == EGL_FALSE)
    {
        destroyGL();
        return false;
    }

    const char *displayExtensions = eglQueryString(mDisplay, EGL_EXTENSIONS);

    // EGL_KHR_create_context is required to request a non-ES2 context.
    bool hasKHRCreateContext = strstr(displayExtensions, "EGL_KHR_create_context") != nullptr;
    if (majorVersion != 2 && minorVersion != 0 && !hasKHRCreateContext)
    {
        destroyGL();
        return false;
    }

    eglBindAPI(EGL_OPENGL_ES_API);
    if (eglGetError() != EGL_SUCCESS)
    {
        destroyGL();
        return false;
    }

    const EGLint configAttributes[] =
    {
        EGL_RED_SIZE,       (mRedBits >= 0)     ? mRedBits     : EGL_DONT_CARE,
        EGL_GREEN_SIZE,     (mGreenBits >= 0)   ? mGreenBits   : EGL_DONT_CARE,
        EGL_BLUE_SIZE,      (mBlueBits >= 0)    ? mBlueBits    : EGL_DONT_CARE,
        EGL_ALPHA_SIZE,     (mAlphaBits >= 0)   ? mAlphaBits   : EGL_DONT_CARE,
        EGL_DEPTH_SIZE,     (mDepthBits >= 0)   ? mDepthBits   : EGL_DONT_CARE,
        EGL_STENCIL_SIZE,   (mStencilBits >= 0) ? mStencilBits : EGL_DONT_CARE,
        EGL_SAMPLE_BUFFERS, mMultisample ? 1 : 0,
        EGL_NONE
    };

    EGLint configCount;
    if (!eglChooseConfig(mDisplay, configAttributes, &mConfig, 1, &configCount) || (configCount != 1))
    {
        destroyGL();
        return false;
    }

    eglGetConfigAttrib(mDisplay, mConfig, EGL_RED_SIZE, &mRedBits);
    eglGetConfigAttrib(mDisplay, mConfig, EGL_GREEN_SIZE, &mGreenBits);
    eglGetConfigAttrib(mDisplay, mConfig, EGL_BLUE_SIZE, &mBlueBits);
    eglGetConfigAttrib(mDisplay, mConfig, EGL_ALPHA_SIZE, &mAlphaBits);
    eglGetConfigAttrib(mDisplay, mConfig, EGL_DEPTH_SIZE, &mDepthBits);
    eglGetConfigAttrib(mDisplay, mConfig, EGL_STENCIL_SIZE, &mStencilBits);

    std::vector<EGLint> surfaceAttributes;
    if (strstr(displayExtensions, "EGL_NV_post_sub_buffer") != nullptr)
    {
        surfaceAttributes.push_back(EGL_POST_SUB_BUFFER_SUPPORTED_NV);
        surfaceAttributes.push_back(EGL_TRUE);
    }

    surfaceAttributes.push_back(EGL_NONE);

    mSurface = eglCreateWindowSurface(mDisplay, mConfig, osWindow->getNativeWindow(), &surfaceAttributes[0]);
    if (eglGetError() != EGL_SUCCESS)
    {
        destroyGL();
        return false;
    }
    ASSERT(mSurface != EGL_NO_SURFACE);

    std::vector<EGLint> contextAttributes;
    if (hasKHRCreateContext)
    {
        contextAttributes.push_back(EGL_CONTEXT_MAJOR_VERSION_KHR);
        contextAttributes.push_back(mClientMajorVersion);

        contextAttributes.push_back(EGL_CONTEXT_MINOR_VERSION_KHR);
        contextAttributes.push_back(mClientMinorVersion);
    }
    contextAttributes.push_back(EGL_NONE);

    mContext = eglCreateContext(mDisplay, mConfig, nullptr, &contextAttributes[0]);
    if (eglGetError() != EGL_SUCCESS)
    {
        destroyGL();
        return false;
    }

    eglMakeCurrent(mDisplay, mSurface, mSurface, mContext);
    if (eglGetError() != EGL_SUCCESS)
    {
        destroyGL();
        return false;
    }

    if (mSwapInterval != -1)
    {
        eglSwapInterval(mDisplay, mSwapInterval);
    }

    return true;
}

void EGLWindow::destroyGL()
{
    if (mSurface != EGL_NO_SURFACE)
    {
        assert(mDisplay != EGL_NO_DISPLAY);
        eglDestroySurface(mDisplay, mSurface);
        mSurface = EGL_NO_SURFACE;
    }

    if (mContext != EGL_NO_CONTEXT)
    {
        assert(mDisplay != EGL_NO_DISPLAY);
        eglDestroyContext(mDisplay, mContext);
        mContext = EGL_NO_CONTEXT;
    }

    if (mDisplay != EGL_NO_DISPLAY)
    {
        eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglTerminate(mDisplay);
        mDisplay = EGL_NO_DISPLAY;
    }
}

bool EGLWindow::isGLInitialized() const
{
    return mSurface != EGL_NO_SURFACE &&
           mContext != EGL_NO_CONTEXT &&
           mDisplay != EGL_NO_DISPLAY;
}

// Find an EGLConfig that is an exact match for the specified attributes. EGL_FALSE is returned if
// the EGLConfig is found.  This indicates that the EGLConfig is not supported.
EGLBoolean EGLWindow::FindEGLConfig(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *config)
{
    EGLint numConfigs = 0;
    eglGetConfigs(dpy, nullptr, 0, &numConfigs);
    std::vector<EGLConfig> allConfigs(numConfigs);
    eglGetConfigs(dpy, allConfigs.data(), static_cast<EGLint>(allConfigs.size()), &numConfigs);

    for (size_t i = 0; i < allConfigs.size(); i++)
    {
        bool matchFound = true;
        for (const EGLint *curAttrib = attrib_list; curAttrib[0] != EGL_NONE; curAttrib += 2)
        {
            EGLint actualValue = EGL_DONT_CARE;
            eglGetConfigAttrib(dpy, allConfigs[i], curAttrib[0], &actualValue);
            if (curAttrib[1] != actualValue)
            {
                matchFound = false;
                break;
            }
        }

        if (matchFound)
        {
            *config = allConfigs[i];
            return EGL_TRUE;
        }
    }

    return EGL_FALSE;
}
