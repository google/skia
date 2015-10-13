//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include <gtest/gtest.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "test_utils/angle_test_configs.h"

using namespace angle;

class EGLQueryContextTest : public testing::TestWithParam<PlatformParameters>
{
  public:
    void SetUp() override
    {
        int clientVersion = GetParam().majorVersion;

        PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(eglGetProcAddress("eglGetPlatformDisplayEXT"));
        EXPECT_TRUE(eglGetPlatformDisplayEXT != NULL);

        EGLint dispattrs[] =
        {
            EGL_PLATFORM_ANGLE_TYPE_ANGLE, GetParam().getRenderer(),
            EGL_NONE
        };
        mDisplay = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, EGL_DEFAULT_DISPLAY, dispattrs);
        EXPECT_TRUE(mDisplay != EGL_NO_DISPLAY);
        EXPECT_TRUE(eglInitialize(mDisplay, NULL, NULL) != EGL_FALSE);

        EGLint ncfg;
        EGLint cfgattrs[] =
        {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_RENDERABLE_TYPE, clientVersion == 3 ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_ES2_BIT,
            EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
            EGL_NONE
        };
        EXPECT_TRUE(eglChooseConfig(mDisplay, cfgattrs, &mConfig, 1, &ncfg) != EGL_FALSE);
        EXPECT_TRUE(ncfg == 1);

        EGLint ctxattrs[] =
        {
            EGL_CONTEXT_CLIENT_VERSION, clientVersion,
            EGL_NONE
        };
        mContext = eglCreateContext(mDisplay, mConfig, NULL, ctxattrs);
        EXPECT_TRUE(mContext != EGL_NO_CONTEXT);

        EGLint surfattrs[] =
        {
            EGL_WIDTH, 16,
            EGL_HEIGHT, 16,
            EGL_NONE
        };
        mSurface = eglCreatePbufferSurface(mDisplay, mConfig, surfattrs);
        EXPECT_TRUE(mSurface != EGL_NO_SURFACE);
    }

    void TearDown() override
    {
        eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroyContext(mDisplay, mContext);
        eglDestroySurface(mDisplay, mSurface);
        eglTerminate(mDisplay);
    }

    EGLDisplay mDisplay;
    EGLConfig mConfig;
    EGLContext mContext;
    EGLSurface mSurface;
};

TEST_P(EGLQueryContextTest, GetConfigID)
{
    EGLint configId, contextConfigId;
    EXPECT_TRUE(eglGetConfigAttrib(mDisplay, mConfig, EGL_CONFIG_ID, &configId) != EGL_FALSE);
    EXPECT_TRUE(eglQueryContext(mDisplay, mContext, EGL_CONFIG_ID, &contextConfigId) != EGL_FALSE);
    EXPECT_TRUE(configId == contextConfigId);
}

TEST_P(EGLQueryContextTest, GetClientType)
{
    EGLint clientType;
    EXPECT_TRUE(eglQueryContext(mDisplay, mContext, EGL_CONTEXT_CLIENT_TYPE, &clientType) != EGL_FALSE);
    EXPECT_TRUE(clientType == EGL_OPENGL_ES_API);
}

TEST_P(EGLQueryContextTest, GetClientVersion)
{
    EGLint clientVersion;
    EXPECT_TRUE(eglQueryContext(mDisplay, mContext, EGL_CONTEXT_CLIENT_VERSION, &clientVersion) != EGL_FALSE);
    EXPECT_TRUE(clientVersion == GetParam().majorVersion);
}

TEST_P(EGLQueryContextTest, GetRenderBufferNoSurface)
{
    EGLint renderBuffer;
    EXPECT_TRUE(eglQueryContext(mDisplay, mContext, EGL_RENDER_BUFFER, &renderBuffer) != EGL_FALSE);
    EXPECT_TRUE(renderBuffer == EGL_NONE);
}

TEST_P(EGLQueryContextTest, GetRenderBufferBoundSurface)
{
    EGLint renderBuffer, contextRenderBuffer;
    EXPECT_TRUE(eglQuerySurface(mDisplay, mSurface, EGL_RENDER_BUFFER, &renderBuffer) != EGL_FALSE);
    EXPECT_TRUE(eglMakeCurrent(mDisplay, mSurface, mSurface, mContext) != EGL_FALSE);
    EXPECT_TRUE(eglQueryContext(mDisplay, mContext, EGL_RENDER_BUFFER, &contextRenderBuffer) != EGL_FALSE);
    EXPECT_TRUE(renderBuffer == contextRenderBuffer);
}

TEST_P(EGLQueryContextTest, BadDisplay)
{
    EGLint val;
    EXPECT_TRUE(eglQueryContext(EGL_NO_DISPLAY, mContext, EGL_CONTEXT_CLIENT_TYPE, &val) == EGL_FALSE);
    EXPECT_TRUE(eglGetError() == EGL_BAD_DISPLAY);
}

TEST_P(EGLQueryContextTest, NotInitialized)
{
    EGLint val;
    TearDown();
    EXPECT_TRUE(eglQueryContext(mDisplay, mContext, EGL_CONTEXT_CLIENT_TYPE, &val) == EGL_FALSE);
    EXPECT_TRUE(eglGetError() == EGL_NOT_INITIALIZED);

    mDisplay = EGL_NO_DISPLAY;
    mSurface = EGL_NO_SURFACE;
    mContext = EGL_NO_CONTEXT;
}

TEST_P(EGLQueryContextTest, BadContext)
{
    EGLint val;
    EXPECT_TRUE(eglQueryContext(mDisplay, EGL_NO_CONTEXT, EGL_CONTEXT_CLIENT_TYPE, &val) == EGL_FALSE);
    EXPECT_TRUE(eglGetError() == EGL_BAD_CONTEXT);
}

TEST_P(EGLQueryContextTest, BadAttribute)
{
    EGLint val;
    EXPECT_TRUE(eglQueryContext(mDisplay, mContext, EGL_HEIGHT, &val) == EGL_FALSE);
    EXPECT_TRUE(eglGetError() == EGL_BAD_ATTRIBUTE);
}

ANGLE_INSTANTIATE_TEST(EGLQueryContextTest, ES2_D3D9(), ES2_D3D11(), ES2_D3D11_FL9_3(), ES2_OPENGL(),
                       ES3_D3D11(), ES3_OPENGL());
