//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

using namespace angle;

class FramebufferRenderMipmapTest : public ANGLETest
{
  protected:
    FramebufferRenderMipmapTest()
    {
        setWindowWidth(256);
        setWindowHeight(256);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    virtual void SetUp()
    {
        ANGLETest::SetUp();

        const std::string vsSource = SHADER_SOURCE
        (
            attribute highp vec4 position;
            void main(void)
            {
                gl_Position = position;
            }
        );

        const std::string fsSource = SHADER_SOURCE
        (
            uniform highp vec4 color;
            void main(void)
            {
                gl_FragColor = color;
            }
        );

        mProgram = CompileProgram(vsSource, fsSource);
        if (mProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        mColorLocation = glGetUniformLocation(mProgram, "color");

        glUseProgram(mProgram);

        glClearColor(0, 0, 0, 0);
        glClearDepthf(0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);

        ASSERT_GL_NO_ERROR();
    }

    virtual void TearDown()
    {
        glDeleteProgram(mProgram);

        ANGLETest::TearDown();
    }

    GLuint mProgram;
    GLint mColorLocation;
};

// Validate that if we are in ES3 or GL_OES_fbo_render_mipmap exists, there are no validation errors
// when using a non-zero level in glFramebufferTexture2D.
TEST_P(FramebufferRenderMipmapTest, Validation)
{
    bool renderToMipmapSupported = extensionEnabled("GL_OES_fbo_render_mipmap") || getClientVersion() > 2;

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    const size_t levels = 5;
    for (size_t i = 0; i < levels; i++)
    {
        size_t size = 1 << ((levels - 1) - i);
        glTexImage2D(GL_TEXTURE_2D, static_cast<GLint>(i), GL_RGBA, static_cast<GLsizei>(size),
                     static_cast<GLsizei>(size), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    }

    EXPECT_GL_NO_ERROR();

    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    EXPECT_GL_NO_ERROR();

    for (size_t i = 0; i < levels; i++)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex,
                               static_cast<GLint>(i));

        if (i > 0 && !renderToMipmapSupported)
        {
            EXPECT_GL_ERROR(GL_INVALID_VALUE);
        }
        else
        {
            EXPECT_GL_NO_ERROR();
            EXPECT_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER), GLenum(GL_FRAMEBUFFER_COMPLETE));
        }
    }

    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &tex);
}

// Render to various levels of a texture and check that they have the correct color data via ReadPixels
TEST_P(FramebufferRenderMipmapTest, RenderToMipmap)
{
    // TODO(geofflang): Figure out why this is broken on Intel OpenGL
    if (isIntel() && getPlatformRenderer() == EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE)
    {
        std::cout << "Test skipped on Intel OpenGL." << std::endl;
        return;
    }

    bool renderToMipmapSupported = extensionEnabled("GL_OES_fbo_render_mipmap") || getClientVersion() > 2;
    if (!renderToMipmapSupported)
    {
        std::cout << "Test skipped because GL_OES_fbo_render_mipmap or ES3 is not available." << std::endl;
        return;
    }

    const GLfloat levelColors[] =
    {
        1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f, 1.0f,
    };
    const size_t testLevels = ArraySize(levelColors) / 4;

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    for (size_t i = 0; i < testLevels; i++)
    {
        size_t size = 1 << ((testLevels - 1) - i);
        glTexImage2D(GL_TEXTURE_2D, static_cast<GLint>(i), GL_RGBA, static_cast<GLsizei>(size),
                     static_cast<GLsizei>(size), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    }

    EXPECT_GL_NO_ERROR();

    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    EXPECT_GL_NO_ERROR();

    // Render to the levels of the texture with different colors
    for (size_t i = 0; i < testLevels; i++)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex,
                               static_cast<GLint>(i));
        EXPECT_GL_NO_ERROR();

        glUseProgram(mProgram);
        glUniform4fv(mColorLocation, 1, levelColors + (i * 4));

        drawQuad(mProgram, "position", 0.5f);
        EXPECT_GL_NO_ERROR();
    }

    // Test that the levels of the texture are correct
    for (size_t i = 0; i < testLevels; i++)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex,
                               static_cast<GLint>(i));
        EXPECT_GL_NO_ERROR();

        const GLfloat *color = levelColors + (i * 4);
        EXPECT_PIXEL_EQ(0, 0, color[0] * 255, color[1] * 255, color[2] * 255, color[3] * 255);
    }

    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &tex);

    EXPECT_GL_NO_ERROR();
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
ANGLE_INSTANTIATE_TEST(FramebufferRenderMipmapTest, ES2_D3D9(), ES2_D3D11(), ES3_D3D11(), ES2_OPENGL(), ES3_OPENGL());
