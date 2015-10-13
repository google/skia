//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// EGLContextCompatibilityTest.cpp:
//   This test will try to use all combinations of context configs and
//   surface configs. If the configs are compatible, it checks that simple
//   rendering works, otherwise it checks an error is generated one MakeCurrent.
#include <gtest/gtest.h>

#include <vector>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "OSWindow.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/angle_test_configs.h"

using namespace angle;

namespace
{

const EGLint contextAttribs[] =
{
    EGL_CONTEXT_CLIENT_VERSION, 2,
    EGL_NONE
};

}

class EGLContextCompatibilityTest : public ANGLETest
{
  public:
    EGLContextCompatibilityTest()
      : mDisplay(0)
    {
    }

    void SetUp() override
    {
        PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(eglGetProcAddress("eglGetPlatformDisplayEXT"));
        ASSERT_TRUE(eglGetPlatformDisplayEXT != nullptr);

        EGLint dispattrs[] =
        {
            EGL_PLATFORM_ANGLE_TYPE_ANGLE, GetParam().getRenderer(),
            EGL_NONE
        };
        mDisplay = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, EGL_DEFAULT_DISPLAY, dispattrs);
        ASSERT_TRUE(mDisplay != EGL_NO_DISPLAY);

        ASSERT_TRUE(eglInitialize(mDisplay, nullptr, nullptr) == EGL_TRUE);

        int nConfigs = 0;
        ASSERT_TRUE(eglGetConfigs(mDisplay, nullptr, 0, &nConfigs) == EGL_TRUE);
        ASSERT_TRUE(nConfigs != 0);

        int nReturnedConfigs = 0;
        mConfigs.resize(nConfigs);
        ASSERT_TRUE(eglGetConfigs(mDisplay, mConfigs.data(), nConfigs, &nReturnedConfigs) == EGL_TRUE);
        ASSERT_TRUE(nConfigs == nReturnedConfigs);
    }

    void TearDown() override
    {
        eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglTerminate(mDisplay);
    };

  protected:
    bool areConfigsCompatible(EGLConfig c1, EGLConfig c2, EGLint surfaceBit)
    {
        EGLint colorBufferType1, colorBufferType2;
        EGLint red1, red2, green1, green2, blue1, blue2, alpha1, alpha2;
        EGLint depth1, depth2, stencil1, stencil2;
        EGLint surfaceType1, surfaceType2;

        eglGetConfigAttrib(mDisplay, c1, EGL_COLOR_BUFFER_TYPE, &colorBufferType1);
        eglGetConfigAttrib(mDisplay, c2, EGL_COLOR_BUFFER_TYPE, &colorBufferType2);

        eglGetConfigAttrib(mDisplay, c1, EGL_RED_SIZE, &red1);
        eglGetConfigAttrib(mDisplay, c2, EGL_RED_SIZE, &red2);
        eglGetConfigAttrib(mDisplay, c1, EGL_GREEN_SIZE, &green1);
        eglGetConfigAttrib(mDisplay, c2, EGL_GREEN_SIZE, &green2);
        eglGetConfigAttrib(mDisplay, c1, EGL_BLUE_SIZE, &blue1);
        eglGetConfigAttrib(mDisplay, c2, EGL_BLUE_SIZE, &blue2);
        eglGetConfigAttrib(mDisplay, c1, EGL_ALPHA_SIZE, &alpha1);
        eglGetConfigAttrib(mDisplay, c2, EGL_ALPHA_SIZE, &alpha2);

        eglGetConfigAttrib(mDisplay, c1, EGL_DEPTH_SIZE, &depth1);
        eglGetConfigAttrib(mDisplay, c2, EGL_DEPTH_SIZE, &depth2);
        eglGetConfigAttrib(mDisplay, c1, EGL_STENCIL_SIZE, &stencil1);
        eglGetConfigAttrib(mDisplay, c2, EGL_STENCIL_SIZE, &stencil2);

        eglGetConfigAttrib(mDisplay, c1, EGL_SURFACE_TYPE, &surfaceType1);
        eglGetConfigAttrib(mDisplay, c2, EGL_SURFACE_TYPE, &surfaceType2);

        return colorBufferType1 == colorBufferType2 &&
               red1 == red2 && green1 == green2 && blue1 == blue2 && alpha1 == alpha2 &&
               depth1 == depth2 && stencil1 == stencil2 &&
               (surfaceType1 & surfaceBit) != 0 &&
               (surfaceType2 & surfaceBit) != 0;
    }

    void testWindowCompatibility(EGLConfig windowConfig, EGLConfig contextConfig, bool compatible) const
    {
        OSWindow *osWindow = CreateOSWindow();
        ASSERT_TRUE(osWindow != nullptr);
        osWindow->initialize("EGLContextCompatibilityTest", 500, 500);
        osWindow->setVisible(true);

        EGLContext context = eglCreateContext(mDisplay, contextConfig, EGL_NO_CONTEXT, contextAttribs);
        ASSERT_TRUE(context != EGL_NO_CONTEXT);

        EGLSurface window = eglCreateWindowSurface(mDisplay, windowConfig, osWindow->getNativeWindow(), nullptr);
        ASSERT_EGL_SUCCESS();

        if (compatible)
        {
            testClearSurface(window, context);
        }
        else
        {
            testMakeCurrentFails(window, context);
        }

        eglDestroySurface(mDisplay, window);
        ASSERT_EGL_SUCCESS();

        eglDestroyContext(mDisplay, context);
        ASSERT_EGL_SUCCESS();

        SafeDelete(osWindow);
    }

    void testPbufferCompatibility(EGLConfig pbufferConfig, EGLConfig contextConfig, bool compatible) const
    {
        EGLContext context = eglCreateContext(mDisplay, contextConfig, EGL_NO_CONTEXT, contextAttribs);
        ASSERT_TRUE(context != EGL_NO_CONTEXT);

        const EGLint pBufferAttribs[] =
        {
            EGL_WIDTH, 500,
            EGL_HEIGHT, 500,
            EGL_NONE,
        };
        EGLSurface pbuffer = eglCreatePbufferSurface(mDisplay, pbufferConfig, pBufferAttribs);
        ASSERT_TRUE(pbuffer != EGL_NO_SURFACE);

        if (compatible)
        {
            testClearSurface(pbuffer, context);
        }
        else
        {
            testMakeCurrentFails(pbuffer, context);
        }

        eglDestroySurface(mDisplay, pbuffer);
        ASSERT_EGL_SUCCESS();

        eglDestroyContext(mDisplay, context);
        ASSERT_EGL_SUCCESS();
    }

    std::vector<EGLConfig> mConfigs;
    EGLDisplay mDisplay;

  private:
    void testClearSurface(EGLSurface surface, EGLContext context) const
    {
        eglMakeCurrent(mDisplay, surface, surface, context);
        ASSERT_EGL_SUCCESS();

        glViewport(0, 0, 500, 500);
        glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ASSERT_GL_NO_ERROR();
        EXPECT_PIXEL_EQ(250, 250, 0, 0, 255, 255);

        eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        ASSERT_EGL_SUCCESS();
    }

    void testMakeCurrentFails(EGLSurface surface, EGLContext context) const
    {
        eglMakeCurrent(mDisplay, surface, surface, context);
        EXPECT_EGL_ERROR(EGL_BAD_MATCH);
    }
};

// The test is split in several subtest so that simple cases
// are tested separately. Also each surface types are not tested
// together.

// Basic test checking contexts and windows created with the
// same config can render.
TEST_P(EGLContextCompatibilityTest, WindowSameConfig)
{
    for (size_t i = 0; i < mConfigs.size(); i++)
    {
        EGLConfig config = mConfigs[i];

        EGLint surfaceType;
        eglGetConfigAttrib(mDisplay, config, EGL_SURFACE_TYPE, &surfaceType);
        ASSERT_EGL_SUCCESS();

        if ((surfaceType & EGL_WINDOW_BIT) != 0)
        {
            testWindowCompatibility(config, config, true);
        }
    }
}

// Basic test checking contexts and pbuffers created with the
// same config can render.
TEST_P(EGLContextCompatibilityTest, PbufferSameConfig)
{
    for (size_t i = 0; i < mConfigs.size(); i++)
    {
        EGLConfig config = mConfigs[i];

        EGLint surfaceType;
        eglGetConfigAttrib(mDisplay, config, EGL_SURFACE_TYPE, &surfaceType);
        ASSERT_EGL_SUCCESS();

        if ((surfaceType & EGL_PBUFFER_BIT) != 0)
        {
            testPbufferCompatibility(config, config, true);
        }
    }
}

// Check that a context rendering to a window with a different
// config works or errors according to the EGL compatibility rules
TEST_P(EGLContextCompatibilityTest, WindowDifferentConfig)
{
    for (size_t i = 0; i < mConfigs.size(); i++)
    {
        EGLConfig config1 = mConfigs[i];
        EGLint surfaceType;
        eglGetConfigAttrib(mDisplay, config1, EGL_SURFACE_TYPE, &surfaceType);
        ASSERT_EGL_SUCCESS();

        if ((surfaceType & EGL_WINDOW_BIT) == 0)
        {
            continue;
        }

        for (size_t j = 0; j < mConfigs.size(); j++)
        {
            EGLConfig config2 = mConfigs[j];
            testWindowCompatibility(config1, config2,
                                    areConfigsCompatible(config1, config2, EGL_WINDOW_BIT));
        }
    }
}

// Check that a context rendering to a pbuffer with a different
// config works or errors according to the EGL compatibility rules
TEST_P(EGLContextCompatibilityTest, PbufferDifferentConfig)
{
    for (size_t i = 0; i < mConfigs.size(); i++)
    {
        EGLConfig config1 = mConfigs[i];
        EGLint surfaceType;
        eglGetConfigAttrib(mDisplay, config1, EGL_SURFACE_TYPE, &surfaceType);
        ASSERT_EGL_SUCCESS();

        if ((surfaceType & EGL_PBUFFER_BIT) == 0)
        {
            continue;
        }

        for (size_t j = 0; j < mConfigs.size(); j++)
        {
            EGLConfig config2 = mConfigs[j];
            testPbufferCompatibility(config1, config2, areConfigsCompatible(config1, config2, EGL_PBUFFER_BIT));
        }
    }
}

ANGLE_INSTANTIATE_TEST(EGLContextCompatibilityTest, ES2_D3D9(), ES2_D3D11(), ES2_OPENGL());
