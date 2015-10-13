//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

using namespace angle;

class DiscardFramebufferEXTTest : public ANGLETest
{
protected:
    DiscardFramebufferEXTTest()
    {
        setWindowWidth(256);
        setWindowHeight(256);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
        setConfigStencilBits(8);
    }
};

TEST_P(DiscardFramebufferEXTTest, ExtensionEnabled)
{
    EGLPlatformParameters platform = GetParam().eglParameters;

    if (platform.renderer == EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE)
    {
        // EXPECT_TRUE(extensionEnabled("EXT_discard_framebuffer"));

        // EXT_discard_framebuffer is disabled in D3D11 ANGLE due to Chromium BUG:497445
        // Enabling this extension (even as a no-op) causes WebGL video failures in Chromium
        // Once this bug is fixed, we can reenable the extension.
        EXPECT_FALSE(extensionEnabled("EXT_discard_framebuffer"));
    }
    else
    {
        // Other platforms don't currently implement this extension
        EXPECT_FALSE(extensionEnabled("EXT_discard_framebuffer"));
    }
}

TEST_P(DiscardFramebufferEXTTest, DefaultFramebuffer)
{
    if (!extensionEnabled("EXT_discard_framebuffer"))
    {
        std::cout << "Test skipped because EXT_discard_framebuffer is not available." << std::endl;
        return;
    }

    // These should succeed on the default framebuffer
    const GLenum discards1[] = { GL_COLOR_EXT };
    glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, discards1);
    EXPECT_GL_NO_ERROR();

    const GLenum discards2[] = { GL_DEPTH_EXT };
    glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, discards2);
    EXPECT_GL_NO_ERROR();

    const GLenum discards3[] = { GL_STENCIL_EXT };
    glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, discards3);
    EXPECT_GL_NO_ERROR();

    const GLenum discards4[] = { GL_STENCIL_EXT, GL_COLOR_EXT, GL_DEPTH_EXT };
    glDiscardFramebufferEXT(GL_FRAMEBUFFER, 3, discards4);
    EXPECT_GL_NO_ERROR();

    // These should fail on the default framebuffer
    const GLenum discards5[] = { GL_COLOR_ATTACHMENT0 };
    glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, discards5);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    const GLenum discards6[] = { GL_DEPTH_ATTACHMENT };
    glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, discards6);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    const GLenum discards7[] = { GL_STENCIL_ATTACHMENT };
    glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, discards7);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
}

TEST_P(DiscardFramebufferEXTTest, NonDefaultFramebuffer)
{
    if (!extensionEnabled("EXT_discard_framebuffer"))
    {
        std::cout << "Test skipped because EXT_discard_framebuffer is not available." << std::endl;
        return;
    }

    GLuint tex2D;
    GLuint framebuffer;

    // Create a basic off-screen framebuffer
    // Don't create a depth/stencil texture, to ensure that also works correctly
    glGenTextures(1, &tex2D);
    glGenFramebuffers(1, &framebuffer);
    glBindTexture(GL_TEXTURE_2D, tex2D);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, getWindowWidth(), getWindowHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex2D, 0);
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    // These should fail on the non-default framebuffer
    const GLenum discards1[] = { GL_COLOR_EXT };
    glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, discards1);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    const GLenum discards2[] = { GL_DEPTH_EXT };
    glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, discards2);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    const GLenum discards3[] = { GL_STENCIL_EXT };
    glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, discards3);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    const GLenum discards4[] = { GL_STENCIL_EXT, GL_COLOR_EXT, GL_DEPTH_EXT };
    glDiscardFramebufferEXT(GL_FRAMEBUFFER, 3, discards4);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    // These should succeed on the non-default framebuffer
    const GLenum discards5[] = { GL_COLOR_ATTACHMENT0 };
    glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, discards5);
    EXPECT_GL_NO_ERROR();

    const GLenum discards6[] = { GL_DEPTH_ATTACHMENT };
    glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, discards6);
    EXPECT_GL_NO_ERROR();

    const GLenum discards7[] = { GL_STENCIL_ATTACHMENT };
    glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, discards7);
    EXPECT_GL_NO_ERROR();
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
ANGLE_INSTANTIATE_TEST(DiscardFramebufferEXTTest, ES2_D3D9(), ES2_D3D11(), ES2_D3D11_FL9_3(), ES2_OPENGL(), ES3_OPENGL());
