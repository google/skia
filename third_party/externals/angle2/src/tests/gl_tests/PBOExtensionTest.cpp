//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

using namespace angle;

class PBOExtensionTest : public ANGLETest
{
  protected:
    PBOExtensionTest()
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

        if (extensionEnabled("NV_pixel_buffer_object"))
        {
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

            glGenBuffers(1, &mPositionVBO);
            glBindBuffer(GL_ARRAY_BUFFER, mPositionVBO);
            glBufferData(GL_ARRAY_BUFFER, 128, NULL, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        ASSERT_GL_NO_ERROR();
    }

    virtual void TearDown()
    {
        ANGLETest::TearDown();

        glDeleteBuffers(1, &mPBO);
        glDeleteProgram(mProgram);
    }

    GLuint mPBO;
    GLuint mProgram;
    GLuint mPositionVBO;
};

TEST_P(PBOExtensionTest, PBOWithOtherTarget)
{
    if (extensionEnabled("NV_pixel_buffer_object"))
    {
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        EXPECT_GL_NO_ERROR();

        glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBO);
        glReadPixels(0, 0, 16, 16, GL_RGBA, GL_UNSIGNED_BYTE, 0);

        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, mPBO);

        GLvoid *mappedPtr = glMapBufferRangeEXT(GL_ARRAY_BUFFER, 0, 32, GL_MAP_READ_BIT);
        unsigned char *dataPtr = static_cast<unsigned char *>(mappedPtr);
        EXPECT_GL_NO_ERROR();

        EXPECT_EQ(255, dataPtr[0]);
        EXPECT_EQ(0,   dataPtr[1]);
        EXPECT_EQ(0,   dataPtr[2]);
        EXPECT_EQ(255, dataPtr[3]);

        glUnmapBufferOES(GL_ARRAY_BUFFER);
    }
    EXPECT_GL_NO_ERROR();
}

TEST_P(PBOExtensionTest, PBOWithExistingData)
{
    if (extensionEnabled("NV_pixel_buffer_object"))
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
        GLvoid * mappedPtr = glMapBufferRangeEXT(GL_PIXEL_PACK_BUFFER, 0, 32, GL_MAP_READ_BIT);
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

        glUnmapBufferOES(GL_PIXEL_PACK_BUFFER);
    }
    EXPECT_GL_NO_ERROR();
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
ANGLE_INSTANTIATE_TEST(PBOExtensionTest, ES2_D3D11(), ES3_D3D11());
