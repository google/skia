//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

using namespace angle;

class DrawBuffersTest : public ANGLETest
{
  protected:
    DrawBuffersTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
        mMaxDrawBuffers = 0;
    }

    virtual void SetUp()
    {
        ANGLETest::SetUp();

        // This test seems to fail on an nVidia machine when the window is hidden
        SetWindowVisible(true);

        glGenFramebuffers(1, &mFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

        glGenTextures(4, mTextures);

        for (size_t texIndex = 0; texIndex < ArraySize(mTextures); texIndex++)
        {
            glBindTexture(GL_TEXTURE_2D, mTextures[texIndex]);
            glTexStorage2DEXT(GL_TEXTURE_2D, 1, GL_RGBA8, getWindowWidth(), getWindowHeight());
        }

        GLfloat data[] =
        {
            -1.0f, 1.0f,
            -1.0f, -2.0f,
            2.0f, 1.0f
        };

        glGenBuffers(1, &mBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6, data, GL_STATIC_DRAW);

        glGetIntegerv(GL_MAX_DRAW_BUFFERS, &mMaxDrawBuffers);

        ASSERT_GL_NO_ERROR();
    }

    virtual void TearDown()
    {
        glDeleteFramebuffers(1, &mFBO);
        glDeleteTextures(4, mTextures);
        glDeleteBuffers(1, &mBuffer);

        ANGLETest::TearDown();
    }

    void setupMRTProgramESSL3(bool bufferEnabled[8], GLuint *programOut)
    {
        const std::string vertexShaderSource =
            "#version 300 es\n"
            "in vec4 position;\n"
            "void main() {\n"
            "    gl_Position = position;\n"
            "}\n";

        std::stringstream strstr;

        strstr << "#version 300 es\n"
                  "precision highp float;\n";

        for (unsigned int index = 0; index < 8; index++)
        {
            if (bufferEnabled[index])
            {
                strstr << "layout(location = " << index << ") "
                          "out vec4 value" << index << ";\n";
            }
        }

        strstr << "void main()\n"
                  "{\n";

        for (unsigned int index = 0; index < 8; index++)
        {
            if (bufferEnabled[index])
            {
                unsigned int r = (index + 1) & 1;
                unsigned int g = (index + 1) & 2;
                unsigned int b = (index + 1) & 4;

                strstr << "    value" << index << " = vec4("
                       << r << ".0, " << g << ".0, "
                       << b << ".0, 1.0);\n";
            }
        }

        strstr << "}\n";

        *programOut = CompileProgram(vertexShaderSource, strstr.str());
        if (*programOut == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        glUseProgram(*programOut);

        GLint location = glGetAttribLocation(*programOut, "position");
        ASSERT_NE(location, -1);
        glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
        glVertexAttribPointer(location, 2, GL_FLOAT, GL_FALSE, 8, NULL);
        glEnableVertexAttribArray(location);
    }

    void setupMRTProgramESSL1(bool bufferEnabled[8], GLuint *programOut)
    {
        const std::string vertexShaderSource =
            "attribute vec4 position;\n"
            "void main() {\n"
            "    gl_Position = position;\n"
            "}\n";

        std::stringstream strstr;

        strstr << "#extension GL_EXT_draw_buffers : enable\n"
                  "precision highp float;\n"
                  "void main()\n"
                  "{\n";

        for (unsigned int index = 0; index < 8; index++)
        {
            if (bufferEnabled[index])
            {
                unsigned int r = (index + 1) & 1;
                unsigned int g = (index + 1) & 2;
                unsigned int b = (index + 1) & 4;

                strstr << "    gl_FragData[" << index << "] = vec4("
                    << r << ".0, " << g << ".0, "
                    << b << ".0, 1.0);\n";
            }
        }

        strstr << "}\n";

        *programOut = CompileProgram(vertexShaderSource, strstr.str());
        if (*programOut == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        glUseProgram(*programOut);

        GLint location = glGetAttribLocation(*programOut, "position");
        ASSERT_NE(location, -1);
        glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
        glVertexAttribPointer(location, 2, GL_FLOAT, GL_FALSE, 8, NULL);
        glEnableVertexAttribArray(location);
    }

    void setupMRTProgram(bool bufferEnabled[8], GLuint *programOut)
    {
        if (getClientVersion() == 3)
        {
            setupMRTProgramESSL3(bufferEnabled, programOut);
        }
        else
        {
            ASSERT_EQ(getClientVersion(), 2);
            setupMRTProgramESSL1(bufferEnabled, programOut);
        }
    }

    void verifyAttachment(unsigned int index, GLuint textureName)
    {
        for (GLint colorAttachment = 0; colorAttachment < mMaxDrawBuffers; colorAttachment++)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + colorAttachment, GL_TEXTURE_2D, 0, 0);
        }

        glBindTexture(GL_TEXTURE_2D, textureName);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureName, 0);

        unsigned int r = (((index + 1) & 1) > 0) ? 255 : 0;
        unsigned int g = (((index + 1) & 2) > 0) ? 255 : 0;
        unsigned int b = (((index + 1) & 4) > 0) ? 255 : 0;

        EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, r, g, b, 255);
    }

    GLuint mFBO;
    GLuint mTextures[4];
    GLuint mBuffer;
    GLint mMaxDrawBuffers;
};

// Verify that GL_MAX_DRAW_BUFFERS returns the expected values for D3D11
TEST_P(DrawBuffersTest, VerifyD3DLimits)
{
    EGLPlatformParameters platform = GetParam().eglParameters;
    if (platform.renderer == EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE)
    {
        if (platform.majorVersion == 9 && platform.minorVersion == 3)
        {
            // D3D11 Feature Level 9_3 supports 4 draw buffers
            ASSERT_EQ(mMaxDrawBuffers, 4);
        }
        else
        {
            // D3D11 Feature Level 10_0+ supports 8 draw buffers
            ASSERT_EQ(mMaxDrawBuffers, 8);
        }
    }
    else
    {
        std::cout << "Test skipped for non-D3D11 renderers." << std::endl;
        return;
    }
}

TEST_P(DrawBuffersTest, Gaps)
{
    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mTextures[0], 0);

    bool flags[8] = { false, true };

    GLuint program;
    setupMRTProgram(flags, &program);

    const GLenum bufs[] =
    {
        GL_NONE,
        GL_COLOR_ATTACHMENT1
    };
    glUseProgram(program);
    glDrawBuffersEXT(2, bufs);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    verifyAttachment(1, mTextures[0]);

    glDeleteProgram(program);
}

TEST_P(DrawBuffersTest, FirstAndLast)
{
    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextures[0], 0);

    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, mTextures[1], 0);

    bool flags[8] = { true, false, false, true };

    GLuint program;
    setupMRTProgram(flags, &program);

    const GLenum bufs[] =
    {
        GL_COLOR_ATTACHMENT0,
        GL_NONE,
        GL_NONE,
        GL_COLOR_ATTACHMENT3
    };

    glUseProgram(program);
    glDrawBuffersEXT(4, bufs);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    verifyAttachment(0, mTextures[0]);
    verifyAttachment(3, mTextures[1]);

    EXPECT_GL_NO_ERROR();

    glDeleteProgram(program);
}

TEST_P(DrawBuffersTest, FirstHalfNULL)
{
    bool flags[8] = { false };
    GLenum bufs[8] = { GL_NONE };

    GLuint halfMaxDrawBuffers = static_cast<GLuint>(mMaxDrawBuffers) / 2;

    for (GLuint texIndex = 0; texIndex < halfMaxDrawBuffers; texIndex++)
    {
        glBindTexture(GL_TEXTURE_2D, mTextures[texIndex]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + halfMaxDrawBuffers + texIndex, GL_TEXTURE_2D, mTextures[texIndex], 0);
        flags[texIndex + halfMaxDrawBuffers] = true;
        bufs[texIndex + halfMaxDrawBuffers] = GL_COLOR_ATTACHMENT0 + halfMaxDrawBuffers + texIndex;
    }

    GLuint program;
    setupMRTProgram(flags, &program);

    glUseProgram(program);
    glDrawBuffersEXT(mMaxDrawBuffers, bufs);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    for (GLuint texIndex = 0; texIndex < halfMaxDrawBuffers; texIndex++)
    {
        verifyAttachment(texIndex + halfMaxDrawBuffers, mTextures[texIndex]);
    }

    EXPECT_GL_NO_ERROR();

    glDeleteProgram(program);
}

TEST_P(DrawBuffersTest, UnwrittenOutputVariablesShouldNotCrash)
{
    // Bind two render targets but use a shader which writes only to the first one.
    glBindTexture(GL_TEXTURE_2D, mTextures[0]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextures[0], 0);

    glBindTexture(GL_TEXTURE_2D, mTextures[1]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mTextures[1], 0);

    bool flags[8] = { true, false };

    GLuint program;
    setupMRTProgram(flags, &program);

    const GLenum bufs[] =
    {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_NONE,
        GL_NONE,
    };

    glUseProgram(program);
    glDrawBuffersEXT(4, bufs);

    // This call should not crash when we dynamically generate the HLSL code.
    glDrawArrays(GL_TRIANGLES, 0, 3);

    verifyAttachment(0, mTextures[0]);

    EXPECT_GL_NO_ERROR();

    glDeleteProgram(program);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
ANGLE_INSTANTIATE_TEST(DrawBuffersTest, ES2_D3D11(), ES3_D3D11(), ES2_D3D11_FL9_3());
