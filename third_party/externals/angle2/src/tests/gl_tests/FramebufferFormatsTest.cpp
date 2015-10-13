//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

using namespace angle;

class FramebufferFormatsTest : public ANGLETest
{
  protected:
    FramebufferFormatsTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void checkBitCount(GLuint fbo, GLenum channel, GLint minBits)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        GLint bits = 0;
        glGetIntegerv(channel, &bits);

        if (minBits == 0)
        {
            EXPECT_EQ(minBits, bits);
        }
        else
        {
            EXPECT_GE(bits, minBits);
        }
    }

    void testBitCounts(GLuint fbo, GLint minRedBits, GLint minGreenBits, GLint minBlueBits,
                       GLint minAlphaBits, GLint minDepthBits, GLint minStencilBits)
    {
        checkBitCount(fbo, GL_RED_BITS, minRedBits);
        checkBitCount(fbo, GL_GREEN_BITS, minGreenBits);
        checkBitCount(fbo, GL_BLUE_BITS, minBlueBits);
        checkBitCount(fbo, GL_ALPHA_BITS, minAlphaBits);
        checkBitCount(fbo, GL_DEPTH_BITS, minDepthBits);
        checkBitCount(fbo, GL_STENCIL_BITS, minStencilBits);
    }

    void testTextureFormat(GLenum internalFormat, GLint minRedBits, GLint minGreenBits, GLint minBlueBits,
                           GLint minAlphaBits)
    {
        GLuint tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexStorage2DEXT(GL_TEXTURE_2D, 1, internalFormat, 1, 1);

        GLuint fbo = 0;
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

        testBitCounts(fbo, minRedBits, minGreenBits, minBlueBits, minAlphaBits, 0, 0);

        glDeleteTextures(1, &tex);
        glDeleteFramebuffers(1, &fbo);
    }

    void testRenderbufferMultisampleFormat(int minESVersion, GLenum attachmentType, GLenum internalFormat)
    {
        // TODO(geofflang): Figure out why this is broken on Intel OpenGL
        if (isIntel() && getPlatformRenderer() == EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE)
        {
            std::cout << "Test skipped on Intel OpenGL." << std::endl;
            return;
        }

        int clientVersion = getClientVersion();
        if (clientVersion < minESVersion)
        {
            return;
        }

        // Check that multisample is supported with at least two samples (minimum required is 1)
        bool supports2Samples = false;

        if (clientVersion == 2)
        {
            if (extensionEnabled("ANGLE_framebuffer_multisample"))
            {
                int maxSamples;
                glGetIntegerv(GL_MAX_SAMPLES_ANGLE, &maxSamples);
                supports2Samples = maxSamples >= 2;
            }
        }
        else
        {
            assert(clientVersion >= 3);
            int maxSamples;
            glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
            supports2Samples = maxSamples >= 2;
        }

        if (!supports2Samples)
        {
            return;
        }

        GLuint framebufferID;
        glGenFramebuffers(1, &framebufferID);
        glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);

        GLuint renderbufferID;
        glGenRenderbuffers(1, &renderbufferID);
        glBindRenderbuffer(GL_RENDERBUFFER, renderbufferID);

        EXPECT_GL_NO_ERROR();
        glRenderbufferStorageMultisampleANGLE(GL_RENDERBUFFER, 2, internalFormat, 128, 128);
        EXPECT_GL_NO_ERROR();
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachmentType, GL_RENDERBUFFER, renderbufferID);
        EXPECT_GL_NO_ERROR();

        glDeleteRenderbuffers(1, &renderbufferID);
        glDeleteFramebuffers(1, &framebufferID);
    }

    virtual void SetUp()
    {
        ANGLETest::SetUp();
    }

    virtual void TearDown()
    {
        ANGLETest::TearDown();
    }
};

TEST_P(FramebufferFormatsTest, RGBA4)
{
    testTextureFormat(GL_RGBA4, 4, 4, 4, 4);
}

TEST_P(FramebufferFormatsTest, RGB565)
{
    testTextureFormat(GL_RGB565, 5, 6, 5, 0);
}

TEST_P(FramebufferFormatsTest, RGB8)
{
    if (getClientVersion() < 3 && !extensionEnabled("GL_OES_rgb8_rgba8"))
    {
        std::cout << "Test skipped due to missing ES3 or GL_OES_rgb8_rgba8." << std::endl;
        return;
    }

    testTextureFormat(GL_RGB8_OES, 8, 8, 8, 0);
}

TEST_P(FramebufferFormatsTest, BGRA8)
{
    if (!extensionEnabled("GL_EXT_texture_format_BGRA8888"))
    {
        std::cout << "Test skipped due to missing GL_EXT_texture_format_BGRA8888." << std::endl;
        return;
    }

    testTextureFormat(GL_BGRA8_EXT, 8, 8, 8, 8);
}

TEST_P(FramebufferFormatsTest, RGBA8)
{
    if (getClientVersion() < 3 && !extensionEnabled("GL_OES_rgb8_rgba8"))
    {
        std::cout << "Test skipped due to missing ES3 or GL_OES_rgb8_rgba8." << std::endl;
        return;
    }

    testTextureFormat(GL_RGBA8_OES, 8, 8, 8, 8);
}

TEST_P(FramebufferFormatsTest, RenderbufferMultisample_DEPTH16)
{
    testRenderbufferMultisampleFormat(2, GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT16);
}

TEST_P(FramebufferFormatsTest, RenderbufferMultisample_DEPTH24)
{
    testRenderbufferMultisampleFormat(3, GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT24);
}

TEST_P(FramebufferFormatsTest, RenderbufferMultisample_DEPTH32F)
{
    if (getClientVersion() < 3)
    {
        std::cout << "Test skipped due to missing ES3." << std::endl;
        return;
    }

    testRenderbufferMultisampleFormat(3, GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT32F);
}

TEST_P(FramebufferFormatsTest, RenderbufferMultisample_DEPTH24_STENCIL8)
{
    testRenderbufferMultisampleFormat(3, GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH24_STENCIL8);
}

TEST_P(FramebufferFormatsTest, RenderbufferMultisample_DEPTH32F_STENCIL8)
{
    if (getClientVersion() < 3)
    {
        std::cout << "Test skipped due to missing ES3." << std::endl;
        return;
    }

    testRenderbufferMultisampleFormat(3, GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH32F_STENCIL8);
}

TEST_P(FramebufferFormatsTest, RenderbufferMultisample_STENCIL_INDEX8)
{
    // TODO(geofflang): Figure out how to support GLSTENCIL_INDEX8 on desktop GL
    if (GetParam().getRenderer() == EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE)
    {
        std::cout << "Test skipped on Desktop OpenGL." << std::endl;
        return;
    }

    testRenderbufferMultisampleFormat(2, GL_STENCIL_ATTACHMENT, GL_STENCIL_INDEX8);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
ANGLE_INSTANTIATE_TEST(FramebufferFormatsTest, ES2_D3D9(), ES2_D3D11(), ES3_D3D11(), ES2_OPENGL(), ES3_OPENGL());
