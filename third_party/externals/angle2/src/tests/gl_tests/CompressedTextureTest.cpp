//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

#include "media/pixel.inl"

using namespace angle;

class CompressedTextureTest : public ANGLETest
{
  protected:
    CompressedTextureTest()
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

        ASSERT_GL_NO_ERROR();
    }

    virtual void TearDown()
    {
        glDeleteProgram(mTextureProgram);

        ANGLETest::TearDown();
    }

    GLuint mTextureProgram;
    GLint mTextureUniformLocation;
};

TEST_P(CompressedTextureTest, CompressedTexImage)
{
    if (!extensionEnabled("GL_EXT_texture_compression_dxt1"))
    {
        std::cout << "Test skipped because GL_EXT_texture_compression_dxt1 is not available." << std::endl;
        return;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_0_width, pixel_0_height, 0, pixel_0_size, pixel_0_data);
    glCompressedTexImage2D(GL_TEXTURE_2D, 1, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_1_width, pixel_1_height, 0, pixel_1_size, pixel_1_data);
    glCompressedTexImage2D(GL_TEXTURE_2D, 2, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_2_width, pixel_2_height, 0, pixel_2_size, pixel_2_data);
    glCompressedTexImage2D(GL_TEXTURE_2D, 3, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_3_width, pixel_3_height, 0, pixel_3_size, pixel_3_data);
    glCompressedTexImage2D(GL_TEXTURE_2D, 4, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_4_width, pixel_4_height, 0, pixel_4_size, pixel_4_data);
    glCompressedTexImage2D(GL_TEXTURE_2D, 5, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_5_width, pixel_5_height, 0, pixel_5_size, pixel_5_data);
    glCompressedTexImage2D(GL_TEXTURE_2D, 6, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_6_width, pixel_6_height, 0, pixel_6_size, pixel_6_data);
    glCompressedTexImage2D(GL_TEXTURE_2D, 7, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_7_width, pixel_7_height, 0, pixel_7_size, pixel_7_data);
    glCompressedTexImage2D(GL_TEXTURE_2D, 8, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_8_width, pixel_8_height, 0, pixel_8_size, pixel_8_data);
    glCompressedTexImage2D(GL_TEXTURE_2D, 9, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_9_width, pixel_9_height, 0, pixel_9_size, pixel_9_data);

    EXPECT_GL_NO_ERROR();

    glUseProgram(mTextureProgram);
    glUniform1i(mTextureUniformLocation, 0);

    drawQuad(mTextureProgram, "position", 0.5f);

    EXPECT_GL_NO_ERROR();

    glDeleteTextures(1, &texture);

    EXPECT_GL_NO_ERROR();
}

TEST_P(CompressedTextureTest, CompressedTexStorage)
{
    if (!extensionEnabled("GL_EXT_texture_compression_dxt1"))
    {
        std::cout << "Test skipped due to missing GL_EXT_texture_compression_dxt1" << std::endl;
        return;
    }

    if (getClientVersion() < 3 && (!extensionEnabled("GL_EXT_texture_storage") || !extensionEnabled("GL_OES_rgb8_rgba8")))
    {
        std::cout << "Test skipped due to missing ES3 or GL_EXT_texture_storage or GL_OES_rgb8_rgba8" << std::endl;
        return;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (getClientVersion() < 3)
    {
        glTexStorage2DEXT(GL_TEXTURE_2D, pixel_levels, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_0_width, pixel_0_height);
    }
    else
    {
        glTexStorage2D(GL_TEXTURE_2D, pixel_levels, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_0_width, pixel_0_height);
    }
    EXPECT_GL_NO_ERROR();

    glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pixel_0_width, pixel_0_height, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_0_size, pixel_0_data);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 1, 0, 0, pixel_1_width, pixel_1_height, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_1_size, pixel_1_data);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 2, 0, 0, pixel_2_width, pixel_2_height, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_2_size, pixel_2_data);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 3, 0, 0, pixel_3_width, pixel_3_height, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_3_size, pixel_3_data);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 4, 0, 0, pixel_4_width, pixel_4_height, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_4_size, pixel_4_data);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 5, 0, 0, pixel_5_width, pixel_5_height, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_5_size, pixel_5_data);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 6, 0, 0, pixel_6_width, pixel_6_height, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_6_size, pixel_6_data);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 7, 0, 0, pixel_7_width, pixel_7_height, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_7_size, pixel_7_data);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 8, 0, 0, pixel_8_width, pixel_8_height, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_8_size, pixel_8_data);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 9, 0, 0, pixel_9_width, pixel_9_height, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_9_size, pixel_9_data);

    EXPECT_GL_NO_ERROR();

    glUseProgram(mTextureProgram);
    glUniform1i(mTextureUniformLocation, 0);

    drawQuad(mTextureProgram, "position", 0.5f);

    EXPECT_GL_NO_ERROR();

    glDeleteTextures(1, &texture);

    EXPECT_GL_NO_ERROR();
}

class CompressedTextureTestES3 : public CompressedTextureTest { };

class CompressedTextureTestD3D11 : public CompressedTextureTest { };

TEST_P(CompressedTextureTestES3, PBOCompressedTexImage)
{
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, pixel_0_size, NULL, GL_STREAM_DRAW);
    EXPECT_GL_NO_ERROR();

    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_0_size, pixel_0_data);
    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_0_width, pixel_0_height, 0, pixel_0_size, NULL);
    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_1_size, pixel_1_data);
    glCompressedTexImage2D(GL_TEXTURE_2D, 1, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_1_width, pixel_1_height, 0, pixel_1_size, NULL);
    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_2_size, pixel_2_data);
    glCompressedTexImage2D(GL_TEXTURE_2D, 2, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_2_width, pixel_2_height, 0, pixel_2_size, NULL);
    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_3_size, pixel_3_data);
    glCompressedTexImage2D(GL_TEXTURE_2D, 3, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_3_width, pixel_3_height, 0, pixel_3_size, NULL);
    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_4_size, pixel_4_data);
    glCompressedTexImage2D(GL_TEXTURE_2D, 4, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_4_width, pixel_4_height, 0, pixel_4_size, NULL);
    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_5_size, pixel_5_data);
    glCompressedTexImage2D(GL_TEXTURE_2D, 5, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_5_width, pixel_5_height, 0, pixel_5_size, NULL);
    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_6_size, pixel_6_data);
    glCompressedTexImage2D(GL_TEXTURE_2D, 6, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_6_width, pixel_6_height, 0, pixel_6_size, NULL);
    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_7_size, pixel_7_data);
    glCompressedTexImage2D(GL_TEXTURE_2D, 7, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_7_width, pixel_7_height, 0, pixel_7_size, NULL);
    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_8_size, pixel_8_data);
    glCompressedTexImage2D(GL_TEXTURE_2D, 8, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_8_width, pixel_8_height, 0, pixel_8_size, NULL);
    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_9_size, pixel_9_data);
    glCompressedTexImage2D(GL_TEXTURE_2D, 9, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_9_width, pixel_9_height, 0, pixel_9_size, NULL);

    EXPECT_GL_NO_ERROR();

    glUseProgram(mTextureProgram);
    glUniform1i(mTextureUniformLocation, 0);

    drawQuad(mTextureProgram, "position", 0.5f);

    EXPECT_GL_NO_ERROR();

    glDeleteTextures(1, &buffer);
    glDeleteTextures(1, &texture);

    EXPECT_GL_NO_ERROR();
}


TEST_P(CompressedTextureTestD3D11, PBOCompressedTexStorage)
{
    if (getClientVersion() < 3 && !extensionEnabled("GL_EXT_texture_compression_dxt1"))
    {
        return;
    }

    if (getClientVersion() < 3 && (!extensionEnabled("GL_EXT_texture_storage") || !extensionEnabled("GL_OES_rgb8_rgba8")))
    {
        return;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (getClientVersion() < 3)
    {
        glTexStorage2DEXT(GL_TEXTURE_2D, pixel_levels, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_0_width, pixel_0_height);
    }
    else
    {
        glTexStorage2D(GL_TEXTURE_2D, pixel_levels, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_0_width, pixel_0_height);
    }
    EXPECT_GL_NO_ERROR();

    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, pixel_0_size, NULL, GL_STREAM_DRAW);
    EXPECT_GL_NO_ERROR();

    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_0_size, pixel_0_data);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pixel_0_width, pixel_0_height, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_0_size, NULL);
    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_1_size, pixel_1_data);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 1, 0, 0, pixel_1_width, pixel_1_height, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_1_size, NULL);
    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_2_size, pixel_2_data);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 2, 0, 0, pixel_2_width, pixel_2_height, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_2_size, NULL);
    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_3_size, pixel_3_data);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 3, 0, 0, pixel_3_width, pixel_3_height, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_3_size, NULL);
    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_4_size, pixel_4_data);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 4, 0, 0, pixel_4_width, pixel_4_height, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_4_size, NULL);
    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_5_size, pixel_5_data);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 5, 0, 0, pixel_5_width, pixel_5_height, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_5_size, NULL);
    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_6_size, pixel_6_data);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 6, 0, 0, pixel_6_width, pixel_6_height, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_6_size, NULL);
    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_7_size, pixel_7_data);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 7, 0, 0, pixel_7_width, pixel_7_height, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_7_size, NULL);
    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_8_size, pixel_8_data);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 8, 0, 0, pixel_8_width, pixel_8_height, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_8_size, NULL);
    glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, pixel_9_size, pixel_9_data);
    glCompressedTexSubImage2D(GL_TEXTURE_2D, 9, 0, 0, pixel_9_width, pixel_9_height, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, pixel_9_size, NULL);

    EXPECT_GL_NO_ERROR();

    glUseProgram(mTextureProgram);
    glUniform1i(mTextureUniformLocation, 0);

    drawQuad(mTextureProgram, "position", 0.5f);

    EXPECT_GL_NO_ERROR();

    glDeleteTextures(1, &texture);

    EXPECT_GL_NO_ERROR();
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
ANGLE_INSTANTIATE_TEST(
    CompressedTextureTest,
    ES2_D3D9(), ES2_D3D11(), ES2_D3D11_FL9_3(), ES2_OPENGL(), ES3_OPENGL());

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
ANGLE_INSTANTIATE_TEST(CompressedTextureTestES3, ES3_D3D11());

ANGLE_INSTANTIATE_TEST(CompressedTextureTestD3D11, ES2_D3D11(), ES3_D3D11(), ES2_D3D11_FL9_3());
