//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

using namespace angle;

class ReadPixelsTest : public ANGLETest
{
  protected:
    ReadPixelsTest()
    {
        setWindowWidth(32);
        setWindowHeight(32);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    virtual void SetUp()
    {
        ANGLETest::SetUp();

        glGenBuffers(1, &mPBO);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBO);
        glBufferData(GL_PIXEL_PACK_BUFFER, 4 * getWindowWidth() * getWindowHeight(), NULL, GL_STATIC_DRAW);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

        const char *vertexShaderSrc = SHADER_SOURCE
        (
            attribute vec4 aTest;
            attribute vec2 aPosition;
            varying vec4 vTest;

            void main()
            {
                vTest = aTest;
                gl_Position = vec4(aPosition, 0.0, 1.0);
                gl_PointSize = 1.0;
            }
        );

        const char *fragmentShaderSrc = SHADER_SOURCE
        (
            precision mediump float;
            varying vec4 vTest;

            void main()
            {
                gl_FragColor = vTest;
            }
        );

        mProgram = CompileProgram(vertexShaderSrc, fragmentShaderSrc);

        glGenTextures(1, &mTexture);
        glBindTexture(GL_TEXTURE_2D, mTexture);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 4, 1);

        glGenFramebuffers(1, &mFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexture, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glGenBuffers(1, &mPositionVBO);
        glBindBuffer(GL_ARRAY_BUFFER, mPositionVBO);
        glBufferData(GL_ARRAY_BUFFER, 128, NULL, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        ASSERT_GL_NO_ERROR();
    }

    virtual void TearDown()
    {
        ANGLETest::TearDown();

        glDeleteBuffers(1, &mPBO);
        glDeleteProgram(mProgram);
        glDeleteTextures(1, &mTexture);
        glDeleteFramebuffers(1, &mFBO);
    }

    GLuint mPBO;
    GLuint mProgram;
    GLuint mTexture;
    GLuint mFBO;
    GLuint mPositionVBO;
};

TEST_P(ReadPixelsTest, OutOfBounds)
{
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();

    GLsizei pixelsWidth = 32;
    GLsizei pixelsHeight = 32;
    GLint offset = 16;
    std::vector<GLubyte> pixels((pixelsWidth + offset) * (pixelsHeight + offset) * 4);

    glReadPixels(-offset, -offset, pixelsWidth + offset, pixelsHeight + offset, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);
    EXPECT_GL_NO_ERROR();

    for (int y = pixelsHeight / 2; y < pixelsHeight; y++)
    {
        for (int x = pixelsWidth / 2; x < pixelsWidth; x++)
        {
            const GLubyte* pixel = &pixels[0] + ((y * (pixelsWidth + offset) + x) * 4);
            unsigned int r = static_cast<unsigned int>(pixel[0]);
            unsigned int g = static_cast<unsigned int>(pixel[1]);
            unsigned int b = static_cast<unsigned int>(pixel[2]);
            unsigned int a = static_cast<unsigned int>(pixel[3]);

            // Expect that all pixels which fell within the framebuffer are red
            EXPECT_EQ(255u, r);
            EXPECT_EQ(0u,   g);
            EXPECT_EQ(0u,   b);
            EXPECT_EQ(255u, a);
        }
    }
}

TEST_P(ReadPixelsTest, PBOWithOtherTarget)
{
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();

    glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBO);
    glReadPixels(0, 0, 16, 16, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, mPBO);

    GLvoid *mappedPtr = glMapBufferRange(GL_ARRAY_BUFFER, 0, 32, GL_MAP_READ_BIT);
    unsigned char *dataPtr = static_cast<unsigned char *>(mappedPtr);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(255, dataPtr[0]);
    EXPECT_EQ(0,   dataPtr[1]);
    EXPECT_EQ(0,   dataPtr[2]);
    EXPECT_EQ(255, dataPtr[3]);

    glUnmapBuffer(GL_ARRAY_BUFFER);
    EXPECT_GL_NO_ERROR();
}

TEST_P(ReadPixelsTest, PBOWithExistingData)
{
    // Clear backbuffer to red
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();

    // Read 16x16 region from red backbuffer to PBO
    glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBO);
    glReadPixels(0, 0, 16, 16, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    // Clear backbuffer to green
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();

    // Read 16x16 region from green backbuffer to PBO at offset 16
    glReadPixels(0, 0, 16, 16, GL_RGBA, GL_UNSIGNED_BYTE, reinterpret_cast<GLvoid*>(16));
    GLvoid * mappedPtr = glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, 32, GL_MAP_READ_BIT);
    unsigned char *dataPtr = static_cast<unsigned char *>(mappedPtr);
    EXPECT_GL_NO_ERROR();

    // Test pixel 0 is red (existing data)
    EXPECT_EQ(255, dataPtr[0]);
    EXPECT_EQ(0, dataPtr[1]);
    EXPECT_EQ(0, dataPtr[2]);
    EXPECT_EQ(255, dataPtr[3]);

    // Test pixel 16 is green (new data)
    EXPECT_EQ(0, dataPtr[16 * 4 + 0]);
    EXPECT_EQ(255, dataPtr[16 * 4 + 1]);
    EXPECT_EQ(0, dataPtr[16 * 4 + 2]);
    EXPECT_EQ(255, dataPtr[16 * 4 + 3]);

    glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    EXPECT_GL_NO_ERROR();
}

TEST_P(ReadPixelsTest, PBOAndSubData)
{
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();

    glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBO);
    glReadPixels(0, 0, 16, 16, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    unsigned char data[4] = { 1, 2, 3, 4 };

    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, mPBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 4, data);

    GLvoid *mappedPtr = glMapBufferRange(GL_ARRAY_BUFFER, 0, 32, GL_MAP_READ_BIT);
    unsigned char *dataPtr = static_cast<unsigned char *>(mappedPtr);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(1, dataPtr[0]);
    EXPECT_EQ(2, dataPtr[1]);
    EXPECT_EQ(3, dataPtr[2]);
    EXPECT_EQ(4, dataPtr[3]);

    glUnmapBuffer(GL_ARRAY_BUFFER);
    EXPECT_GL_NO_ERROR();
}

TEST_P(ReadPixelsTest, PBOAndSubDataOffset)
{
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();

    glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBO);
    glReadPixels(0, 0, 16, 16, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    unsigned char data[4] = { 1, 2, 3, 4 };

    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, mPBO);
    glBufferSubData(GL_ARRAY_BUFFER, 16, 4, data);

    GLvoid *mappedPtr = glMapBufferRange(GL_ARRAY_BUFFER, 0, 32, GL_MAP_READ_BIT);
    unsigned char *dataPtr = static_cast<unsigned char *>(mappedPtr);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(255, dataPtr[0]);
    EXPECT_EQ(0, dataPtr[1]);
    EXPECT_EQ(0, dataPtr[2]);
    EXPECT_EQ(255, dataPtr[3]);

    EXPECT_EQ(1, dataPtr[16]);
    EXPECT_EQ(2, dataPtr[17]);
    EXPECT_EQ(3, dataPtr[18]);
    EXPECT_EQ(4, dataPtr[19]);

    glUnmapBuffer(GL_ARRAY_BUFFER);
    EXPECT_GL_NO_ERROR();
}

TEST_P(ReadPixelsTest, DrawWithPBO)
{
    unsigned char data[4] = { 1, 2, 3, 4 };

    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
    EXPECT_GL_NO_ERROR();

    // glReadBuffer(GL_COLOR_ATTACHMENT0); // FIXME: currently UNIMPLEMENTED
    glBindFramebuffer(GL_READ_FRAMEBUFFER, mFBO);
    EXPECT_GL_NO_ERROR();

    glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBO);
    glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    EXPECT_GL_NO_ERROR();

    float positionData[] = { 0.5f, 0.5f };

    glUseProgram(mProgram);
    glViewport(0, 0, 1, 1);
    glBindBuffer(GL_ARRAY_BUFFER, mPositionVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 1 * 2 * 4, positionData);
    EXPECT_GL_NO_ERROR();

    GLint positionLocation = glGetAttribLocation(mProgram, "aPosition");
    EXPECT_NE(-1, positionLocation);

    GLint testLocation = glGetAttribLocation(mProgram, "aTest");
    EXPECT_NE(-1, testLocation);

    glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionLocation);
    EXPECT_GL_NO_ERROR();

    glBindBuffer(GL_ARRAY_BUFFER, mPBO);
    glVertexAttribPointer(testLocation, 4, GL_UNSIGNED_BYTE, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(testLocation);
    EXPECT_GL_NO_ERROR();

    glDrawArrays(GL_POINTS, 0, 1);
    EXPECT_GL_NO_ERROR();

    memset(data, 0, 4);
    glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(1, data[0]);
    EXPECT_EQ(2, data[1]);
    EXPECT_EQ(3, data[2]);
    EXPECT_EQ(4, data[3]);
}

TEST_P(ReadPixelsTest, MultisampledPBO)
{
    if (getClientVersion() < 3 && !extensionEnabled("GL_ANGLE_framebuffer_multisample"))
    {
        std::cout << "Test skipped because ES3 or GL_ANGLE_framebuffer_multisample is not available." << std::endl;
        return;
    }

    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);

    if (extensionEnabled("GL_ANGLE_framebuffer_multisample"))
    {
        glRenderbufferStorageMultisampleANGLE(GL_RENDERBUFFER, 2, GL_RGBA8, 4, 4);
    }
    else
    {
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, 2, GL_RGBA8, 4, 4);
    }

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);

    ASSERT_GL_NO_ERROR();

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBO);

    EXPECT_GL_NO_ERROR();

    glReadPixels(0, 0, 1, 1, GL_RGBA8, GL_UNSIGNED_BYTE, NULL);

    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glDeleteRenderbuffers(1, &rbo);
    glDeleteFramebuffers(1, &fbo);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
ANGLE_INSTANTIATE_TEST(ReadPixelsTest, ES3_D3D11());
