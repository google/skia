//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

#include <vector>

using namespace angle;

namespace
{

class SwizzleTest : public ANGLETest
{
  protected:
    SwizzleTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);

        GLenum swizzles[] =
        {
            GL_RED,
            GL_GREEN,
            GL_BLUE,
            GL_ALPHA,
            GL_ZERO,
            GL_ONE,
        };

        for (int r = 0; r < 6; r++)
        {
            for (int g = 0; g < 6; g++)
            {
                for (int b = 0; b < 6; b++)
                {
                    for (int a = 0; a < 6; a++)
                    {
                        swizzlePermutation permutation;
                        permutation.swizzleRed = swizzles[r];
                        permutation.swizzleGreen = swizzles[g];
                        permutation.swizzleBlue = swizzles[b];
                        permutation.swizzleAlpha = swizzles[a];
                        mPermutations.push_back(permutation);
                    }
                }
            }
        }
    }

    void SetUp() override
    {
        ANGLETest::SetUp();

        const std::string vertexShaderSource = SHADER_SOURCE
        (
            precision highp float;
            attribute vec4 position;
            varying vec2 texcoord;

            void main()
            {
                gl_Position = position;
                texcoord = (position.xy * 0.5) + 0.5;
            }
        );

        const std::string fragmentShaderSource = SHADER_SOURCE
        (
            precision highp float;
            uniform sampler2D tex;
            varying vec2 texcoord;

            void main()
            {
                gl_FragColor = texture2D(tex, texcoord);
            }
        );

        mProgram = CompileProgram(vertexShaderSource, fragmentShaderSource);
        if (mProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        mTextureUniformLocation = glGetUniformLocation(mProgram, "tex");

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    }

    void TearDown() override
    {
        glDeleteProgram(mProgram);
        glDeleteTextures(1, &mTexture);

        ANGLETest::TearDown();
    }

    template <typename T>
    void init2DTexture(GLenum internalFormat, GLenum dataFormat, GLenum dataType, const T* data)
    {
        glGenTextures(1, &mTexture);
        glBindTexture(GL_TEXTURE_2D, mTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, 1, 1, 0, dataFormat, dataType, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    void init2DCompressedTexture(GLenum internalFormat, GLsizei width, GLsizei height, GLsizei dataSize, const GLubyte* data)
    {
        glGenTextures(1, &mTexture);
        glBindTexture(GL_TEXTURE_2D, mTexture);
        glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, width, height, 0, dataSize, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    GLubyte getExpectedValue(GLenum swizzle, GLubyte unswizzled[4])
    {
        switch (swizzle)
        {
          case GL_RED:   return unswizzled[0];
          case GL_GREEN: return unswizzled[1];
          case GL_BLUE:  return unswizzled[2];
          case GL_ALPHA: return unswizzled[3];
          case GL_ZERO:  return 0;
          case GL_ONE:   return 255;
          default:       return 0;
        }
    }

    void runTest2D()
    {
        glUseProgram(mProgram);
        glBindTexture(GL_TEXTURE_2D, mTexture);
        glUniform1i(mTextureUniformLocation, 0);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_GREEN);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_BLUE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ALPHA);

        glClear(GL_COLOR_BUFFER_BIT);
        drawQuad(mProgram, "position", 0.5f);

        GLubyte unswizzled[4];
        glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &unswizzled);

        for (size_t i = 0; i < mPermutations.size(); i++)
        {
            const swizzlePermutation& permutation = mPermutations[i];

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, permutation.swizzleRed);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, permutation.swizzleGreen);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, permutation.swizzleBlue);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, permutation.swizzleAlpha);

            glClear(GL_COLOR_BUFFER_BIT);
            drawQuad(mProgram, "position", 0.5f);

            EXPECT_PIXEL_EQ(0, 0,
                            getExpectedValue(permutation.swizzleRed, unswizzled),
                            getExpectedValue(permutation.swizzleGreen, unswizzled),
                            getExpectedValue(permutation.swizzleBlue, unswizzled),
                            getExpectedValue(permutation.swizzleAlpha, unswizzled));
        }
    }

    GLuint mProgram;
    GLint mTextureUniformLocation;

    GLuint mTexture;

    struct swizzlePermutation
    {
        GLenum swizzleRed;
        GLenum swizzleGreen;
        GLenum swizzleBlue;
        GLenum swizzleAlpha;
    };
    std::vector<swizzlePermutation> mPermutations;
};

TEST_P(SwizzleTest, RGBA8_2D)
{
    GLubyte data[] = { 1, 64, 128, 200 };
    init2DTexture(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, data);
    runTest2D();
}

TEST_P(SwizzleTest, RGB8_2D)
{
    GLubyte data[] = { 77, 66, 55 };
    init2DTexture(GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, data);
    runTest2D();
}

TEST_P(SwizzleTest, RG8_2D)
{
    GLubyte data[] = { 11, 99 };
    init2DTexture(GL_RG8, GL_RG, GL_UNSIGNED_BYTE, data);
    runTest2D();
}

TEST_P(SwizzleTest, R8_2D)
{
    GLubyte data[] = { 2 };
    init2DTexture(GL_R8, GL_RED, GL_UNSIGNED_BYTE, data);
    runTest2D();
}

TEST_P(SwizzleTest, RGBA32F_2D)
{
    GLfloat data[] = { 0.25f, 0.5f, 0.75f, 0.8f };
    init2DTexture(GL_RGBA32F, GL_RGBA, GL_FLOAT, data);
    runTest2D();
}

TEST_P(SwizzleTest, RGB32F_2D)
{
    GLfloat data[] = { 0.1f, 0.2f, 0.3f };
    init2DTexture(GL_RGB32F, GL_RGB, GL_FLOAT, data);
    runTest2D();
}

TEST_P(SwizzleTest, RG32F_2D)
{
    GLfloat data[] = { 0.9f, 0.1f  };
    init2DTexture(GL_RG32F, GL_RG, GL_FLOAT, data);
    runTest2D();
}

TEST_P(SwizzleTest, R32F_2D)
{
    GLfloat data[] = { 0.5f };
    init2DTexture(GL_R32F, GL_RED, GL_FLOAT, data);
    runTest2D();
}

TEST_P(SwizzleTest, D32F_2D)
{
    GLfloat data[] = { 0.5f };
    init2DTexture(GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, data);
    runTest2D();
}

TEST_P(SwizzleTest, D16_2D)
{
    GLushort data[] = { 0xFF };
    init2DTexture(GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, data);
    runTest2D();
}

TEST_P(SwizzleTest, D24_2D)
{
    GLuint data[] = { 0xFFFF };
    init2DTexture(GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, data);
    runTest2D();
}

TEST_P(SwizzleTest, L8_2D)
{
    GLubyte data[] = {0x77};
    init2DTexture(GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
    runTest2D();
}

TEST_P(SwizzleTest, A8_2D)
{
    GLubyte data[] = {0x55};
    init2DTexture(GL_ALPHA, GL_ALPHA, GL_UNSIGNED_BYTE, data);
    runTest2D();
}

TEST_P(SwizzleTest, LA8_2D)
{
    GLubyte data[] = {0x77, 0x66};
    init2DTexture(GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, data);
    runTest2D();
}

TEST_P(SwizzleTest, L32F_2D)
{
    GLfloat data[] = {0.7f};
    init2DTexture(GL_LUMINANCE, GL_LUMINANCE, GL_FLOAT, data);
    runTest2D();
}

TEST_P(SwizzleTest, A32F_2D)
{
    GLfloat data[] = {
        0.4f,
    };
    init2DTexture(GL_ALPHA, GL_ALPHA, GL_FLOAT, data);
    runTest2D();
}

TEST_P(SwizzleTest, LA32F_2D)
{
    GLfloat data[] = {
        0.5f, 0.6f,
    };
    init2DTexture(GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_FLOAT, data);
    runTest2D();
}

#include "media/pixel.inl"

TEST_P(SwizzleTest, CompressedDXT_2D)
{
    if (!extensionEnabled("GL_EXT_texture_compression_dxt1"))
    {
        std::cout << "Test skipped due to missing GL_EXT_texture_compression_dxt1." << std::endl;
        return;
    }

    init2DCompressedTexture(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_0_width, pixel_0_height, pixel_0_size, pixel_0_data);
    runTest2D();
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
ANGLE_INSTANTIATE_TEST(SwizzleTest, ES3_D3D11(), ES3_OPENGL(), ES3_OPENGL(3, 3));

} // namespace
