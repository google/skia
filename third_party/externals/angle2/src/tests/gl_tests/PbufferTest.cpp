//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

using namespace angle;

class PbufferTest : public ANGLETest
{
  protected:
    PbufferTest()
    {
        setWindowWidth(512);
        setWindowHeight(512);
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
            precision highp float;
            attribute vec4 position;
            varying vec2 texcoord;

            void main()
            {
                gl_Position = position;
                texcoord = (position.xy * 0.5) + 0.5;
                texcoord.y = 1.0 - texcoord.y;
            }
        );

        const std::string textureFSSource = SHADER_SOURCE
        (
            precision highp float;
            uniform sampler2D tex;
            varying vec2 texcoord;

            void main()
            {
                gl_FragColor = texture2D(tex, texcoord);
            }
        );

        mTextureProgram = CompileProgram(vsSource, textureFSSource);
        if (mTextureProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        mTextureUniformLocation = glGetUniformLocation(mTextureProgram, "tex");

        EGLWindow *window = getEGLWindow();

        EGLint surfaceType = 0;
        eglGetConfigAttrib(window->getDisplay(), window->getConfig(), EGL_SURFACE_TYPE, &surfaceType);
        mSupportsPbuffers = (surfaceType & EGL_PBUFFER_BIT) != 0;

        EGLint bindToTextureRGBA = 0;
        eglGetConfigAttrib(window->getDisplay(), window->getConfig(), EGL_BIND_TO_TEXTURE_RGBA, &bindToTextureRGBA);
        mSupportsBindTexImage = (bindToTextureRGBA == EGL_TRUE);

        const EGLint pBufferAttributes[] =
        {
            EGL_WIDTH, static_cast<EGLint>(mPbufferSize),
            EGL_HEIGHT, static_cast<EGLint>(mPbufferSize),
            EGL_TEXTURE_FORMAT, mSupportsBindTexImage ? EGL_TEXTURE_RGBA : EGL_NO_TEXTURE,
            EGL_TEXTURE_TARGET, mSupportsBindTexImage ? EGL_TEXTURE_2D : EGL_NO_TEXTURE,
            EGL_NONE, EGL_NONE,
        };

        mPbuffer = eglCreatePbufferSurface(window->getDisplay(), window->getConfig(), pBufferAttributes);
        if (mSupportsPbuffers)
        {
            ASSERT_NE(mPbuffer, EGL_NO_SURFACE);
            ASSERT_EGL_SUCCESS();
        }
        else
        {
            ASSERT_EQ(mPbuffer, EGL_NO_SURFACE);
            ASSERT_EGL_ERROR(EGL_BAD_MATCH);
        }

        ASSERT_GL_NO_ERROR();
    }

    virtual void TearDown()
    {
        glDeleteProgram(mTextureProgram);

        EGLWindow *window = getEGLWindow();
        eglDestroySurface(window->getDisplay(), mPbuffer);

        ANGLETest::TearDown();
    }

    GLuint mTextureProgram;
    GLint mTextureUniformLocation;

    const size_t mPbufferSize = 32;
    EGLSurface mPbuffer;
    bool mSupportsPbuffers;
    bool mSupportsBindTexImage;
};

// Test clearing a Pbuffer and checking the color is correct
TEST_P(PbufferTest, Clearing)
{
    if (!mSupportsPbuffers)
    {
        std::cout << "Test skipped because Pbuffers are not supported." << std::endl;
        return;
    }

    EGLWindow *window = getEGLWindow();

    // Clear the window surface to blue and verify
    eglMakeCurrent(window->getDisplay(), window->getSurface(), window->getSurface(), window->getContext());
    ASSERT_EGL_SUCCESS();

    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 0, 0, 255, 255);

    // Apply the Pbuffer and clear it to purple and verify
    eglMakeCurrent(window->getDisplay(), mPbuffer, mPbuffer, window->getContext());
    ASSERT_EGL_SUCCESS();

    glViewport(0, 0, static_cast<GLsizei>(mPbufferSize), static_cast<GLsizei>(mPbufferSize));
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(static_cast<GLint>(mPbufferSize) / 2, static_cast<GLint>(mPbufferSize) / 2, 255,
                    0, 255, 255);

    // Rebind the window surface and verify that it is still blue
    eglMakeCurrent(window->getDisplay(), window->getSurface(), window->getSurface(), window->getContext());
    ASSERT_EGL_SUCCESS();
    EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 0, 0, 255, 255);
}

// Bind the Pbuffer to a texture and verify it renders correctly
TEST_P(PbufferTest, BindTexImage)
{
    if (!mSupportsPbuffers)
    {
        std::cout << "Test skipped because Pbuffers are not supported." << std::endl;
        return;
    }

    if (!mSupportsBindTexImage)
    {
        std::cout << "Test skipped because Pbuffer does not support binding to RGBA textures." << std::endl;
        return;
    }

    EGLWindow *window = getEGLWindow();

    // Apply the Pbuffer and clear it to purple
    eglMakeCurrent(window->getDisplay(), mPbuffer, mPbuffer, window->getContext());
    ASSERT_EGL_SUCCESS();

    glViewport(0, 0, static_cast<GLsizei>(mPbufferSize), static_cast<GLsizei>(mPbufferSize));
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_EQ(static_cast<GLint>(mPbufferSize) / 2, static_cast<GLint>(mPbufferSize) / 2, 255,
                    0, 255, 255);

    // Apply the window surface
    eglMakeCurrent(window->getDisplay(), window->getSurface(), window->getSurface(), window->getContext());

    // Create a texture and bind the Pbuffer to it
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    EXPECT_GL_NO_ERROR();

    eglBindTexImage(window->getDisplay(), mPbuffer, EGL_BACK_BUFFER);
    glViewport(0, 0, getWindowWidth(), getWindowHeight());
    ASSERT_EGL_SUCCESS();

    // Draw a quad and verify that it is purple
    glUseProgram(mTextureProgram);
    glUniform1i(mTextureUniformLocation, 0);

    drawQuad(mTextureProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();

    // Unbind the texture
    eglReleaseTexImage(window->getDisplay(), mPbuffer, EGL_BACK_BUFFER);
    ASSERT_EGL_SUCCESS();

    // Verify that purple was drawn
    EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 255, 0, 255, 255);

    glDeleteTextures(1, &texture);
}

// Verify that when eglBind/ReleaseTexImage are called, the texture images are freed and their
// size information is correctly updated.
TEST_P(PbufferTest, TextureSizeReset)
{
    if (!mSupportsPbuffers)
    {
        std::cout << "Test skipped because Pbuffers are not supported." << std::endl;
        return;
    }

    if (!mSupportsBindTexImage)
    {
        std::cout << "Test skipped because Pbuffer does not support binding to RGBA textures." << std::endl;
        return;
    }

    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    EXPECT_GL_NO_ERROR();

    glUseProgram(mTextureProgram);
    glUniform1i(mTextureUniformLocation, 0);

    // Fill the texture with white pixels
    std::vector<GLubyte> whitePixels(mPbufferSize * mPbufferSize * 4, 255);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<GLsizei>(mPbufferSize),
                 static_cast<GLsizei>(mPbufferSize), 0, GL_RGBA, GL_UNSIGNED_BYTE, &whitePixels[0]);
    EXPECT_GL_NO_ERROR();

    // Draw the white texture and verify that the pixels are correct
    drawQuad(mTextureProgram, "position", 0.5f);
    EXPECT_PIXEL_EQ(0, 0, 255, 255, 255, 255);

    // Bind the EGL surface and draw with it, results are undefined since nothing has
    // been written to it
    EGLWindow *window = getEGLWindow();
    eglBindTexImage(window->getDisplay(), mPbuffer, EGL_BACK_BUFFER);
    drawQuad(mTextureProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();

    // Clear the back buffer to a unique color (green)
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_EQ(0, 0, 0, 255, 0, 255);

    // Unbind the EGL surface and try to draw with the texture again, the texture's size should
    // now be zero and incomplete so the back buffer should be black
    eglReleaseTexImage(window->getDisplay(), mPbuffer, EGL_BACK_BUFFER);
    drawQuad(mTextureProgram, "position", 0.5f);
    EXPECT_PIXEL_EQ(0, 0, 0, 0, 0, 255);
}

// Bind a Pbuffer, redefine the texture, and verify it renders correctly
TEST_P(PbufferTest, BindTexImageAndRedefineTexture)
{
    if (!mSupportsPbuffers)
    {
        std::cout << "Test skipped because Pbuffers are not supported." << std::endl;
        return;
    }

    if (!mSupportsBindTexImage)
    {
        std::cout << "Test skipped because Pbuffer does not support binding to RGBA textures." << std::endl;
        return;
    }

    EGLWindow *window = getEGLWindow();

    // Apply the Pbuffer and clear it to purple
    eglMakeCurrent(window->getDisplay(), mPbuffer, mPbuffer, window->getContext());
    ASSERT_EGL_SUCCESS();

    glViewport(0, 0, static_cast<GLsizei>(mPbufferSize), static_cast<GLsizei>(mPbufferSize));
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_EQ(static_cast<GLint>(mPbufferSize) / 2, static_cast<GLint>(mPbufferSize) / 2, 255,
                    0, 255, 255);

    // Apply the window surface
    eglMakeCurrent(window->getDisplay(), window->getSurface(), window->getSurface(), window->getContext());

    // Create a texture and bind the Pbuffer to it
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    EXPECT_GL_NO_ERROR();

    eglBindTexImage(window->getDisplay(), mPbuffer, EGL_BACK_BUFFER);
    glViewport(0, 0, getWindowWidth(), getWindowHeight());
    ASSERT_EGL_SUCCESS();

    // Redefine the texture
    unsigned int pixelValue = 0xFFFF00FF;
    std::vector<unsigned int> pixelData(getWindowWidth() * getWindowHeight(), pixelValue);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getWindowWidth(), getWindowHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, &pixelData[0]);

    // Draw a quad and verify that it is magenta
    glUseProgram(mTextureProgram);
    glUniform1i(mTextureUniformLocation, 0);

    drawQuad(mTextureProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();

    // Verify that magenta was drawn
    EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 255, 0, 255, 255);

    glDeleteTextures(1, &texture);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
ANGLE_INSTANTIATE_TEST(PbufferTest, ES2_D3D9(), ES2_D3D11(), ES2_OPENGL(), ES2_D3D11_WARP(), ES2_D3D11_REFERENCE());
