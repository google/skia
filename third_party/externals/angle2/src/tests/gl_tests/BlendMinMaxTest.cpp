//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

using namespace angle;

class BlendMinMaxTest : public ANGLETest
{
  protected:
    BlendMinMaxTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);

        mProgram = 0;
        mColorLocation = -1;
    }

    struct Color
    {
        float values[4];
    };

    static GLubyte getExpected(bool blendMin, float curColor, GLubyte prevColor)
    {
        GLubyte curAsUbyte = static_cast<GLubyte>((curColor * std::numeric_limits<GLubyte>::max()) + 0.5f);
        return blendMin ? std::min<GLubyte>(curAsUbyte, prevColor) : std::max<GLubyte>(curAsUbyte, prevColor);
    }

    void runTest(GLenum colorFormat)
    {
        if (getClientVersion() < 3 && !extensionEnabled("GL_EXT_blend_minmax"))
        {
            std::cout << "Test skipped because ES3 or GL_EXT_blend_minmax is not available." << std::endl;
            return;
        }

        SetUpFramebuffer(colorFormat);

        const size_t colorCount = 1024;
        Color colors[colorCount];
        for (size_t i = 0; i < colorCount; i++)
        {
            for (size_t j = 0; j < 4; j++)
            {
                colors[i].values[j] = (rand() % 255) / 255.0f;
            }
        }

        GLubyte prevColor[4];
        for (size_t i = 0; i < colorCount; i++)
        {
            const Color &color = colors[i];
            glUseProgram(mProgram);
            glUniform4f(mColorLocation, color.values[0], color.values[1], color.values[2], color.values[3]);

            bool blendMin = (rand() % 2 == 0);
            glBlendEquation(blendMin ? GL_MIN : GL_MAX);

            drawQuad(mProgram, "aPosition", 0.5f);

            if (i > 0)
            {
                EXPECT_PIXEL_EQ(0, 0,
                                getExpected(blendMin, color.values[0], prevColor[0]),
                                getExpected(blendMin, color.values[1], prevColor[1]),
                                getExpected(blendMin, color.values[2], prevColor[2]),
                                getExpected(blendMin, color.values[3], prevColor[3]));
            }

            glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, prevColor);
        }
    }

    virtual void SetUp()
    {
        ANGLETest::SetUp();

        const std::string testVertexShaderSource = SHADER_SOURCE
        (
            attribute highp vec4 aPosition;

            void main(void)
            {
                gl_Position = aPosition;
            }
        );

        const std::string testFragmentShaderSource = SHADER_SOURCE
        (
            uniform highp vec4 color;
            void main(void)
            {
                gl_FragColor = color;
            }
        );

        mProgram = CompileProgram(testVertexShaderSource, testFragmentShaderSource);
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
    }

    void SetUpFramebuffer(GLenum colorFormat)
    {
        glGenFramebuffers(1, &mFramebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFramebuffer);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, mFramebuffer);

        glGenRenderbuffers(1, &mColorRenderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, mColorRenderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, colorFormat, getWindowWidth(), getWindowHeight());
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, mColorRenderbuffer);

        ASSERT_GL_NO_ERROR();
    }

    virtual void TearDown()
    {
        glDeleteProgram(mProgram);
        glDeleteFramebuffers(1, &mFramebuffer);
        glDeleteRenderbuffers(1, &mColorRenderbuffer);

        ANGLETest::TearDown();
    }

    GLuint mProgram;
    GLint mColorLocation;

    GLuint mFramebuffer;
    GLuint mColorRenderbuffer;
};

TEST_P(BlendMinMaxTest, RGBA8)
{
    runTest(GL_RGBA8);
}

TEST_P(BlendMinMaxTest, RGBA32f)
{
    if (getClientVersion() < 3 && !extensionEnabled("GL_OES_texture_float"))
    {
        std::cout << "Test skipped because ES3 or GL_OES_texture_float is not available." << std::endl;
        return;
    }

    runTest(GL_RGBA32F);
}

TEST_P(BlendMinMaxTest, RGBA16F)
{
    if (getClientVersion() < 3 && !extensionEnabled("GL_OES_texture_half_float"))
    {
        std::cout << "Test skipped because ES3 or GL_OES_texture_half_float is not available." << std::endl;
        return;
    }

    // TODO(jmadill): figure out why this fails
    if (isIntel() && GetParam() == ES2_D3D11())
    {
        std::cout << "Test skipped on Intel due to failures." << std::endl;
        return;
    }

    runTest(GL_RGBA16F);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
ANGLE_INSTANTIATE_TEST(BlendMinMaxTest, ES2_D3D9(), ES2_D3D11(), ES2_OPENGL());
