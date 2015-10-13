//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

using namespace angle;

namespace
{

class TextureTest : public ANGLETest
{
  protected:
    TextureTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void SetUp() override
    {
        ANGLETest::SetUp();
        glGenTextures(1, &mTexture2D);
        glGenTextures(1, &mTextureCube);

        glBindTexture(GL_TEXTURE_2D, mTexture2D);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        EXPECT_GL_NO_ERROR();

        glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureCube);
        glTexStorage2DEXT(GL_TEXTURE_CUBE_MAP, 1, GL_RGBA8, 1, 1);
        EXPECT_GL_NO_ERROR();

        ASSERT_GL_NO_ERROR();

        const std::string vertexShaderSource = SHADER_SOURCE
        (
            precision highp float;
            attribute vec4 position;
            varying vec2 texcoord;

            uniform vec2 textureScale;

            void main()
            {
                gl_Position = vec4(position.xy * textureScale, 0.0, 1.0);
                texcoord = (position.xy * 0.5) + 0.5;
            }
        );

        const std::string fragmentShaderSource2D = SHADER_SOURCE
        (
            precision highp float;
            uniform sampler2D tex;
            varying vec2 texcoord;

            void main()
            {
                gl_FragColor = texture2D(tex, texcoord);
            }
        );

        const std::string fragmentShaderSourceCube = SHADER_SOURCE
        (
            precision highp float;
            uniform sampler2D tex2D;
            uniform samplerCube texCube;
            varying vec2 texcoord;

            void main()
            {
                gl_FragColor = texture2D(tex2D, texcoord);
                gl_FragColor += textureCube(texCube, vec3(texcoord, 0));
            }
        );

        m2DProgram = CompileProgram(vertexShaderSource, fragmentShaderSource2D);
        mCubeProgram = CompileProgram(vertexShaderSource, fragmentShaderSourceCube);
        ASSERT_NE(0u, m2DProgram);
        ASSERT_NE(0u, mCubeProgram);

        mTexture2DUniformLocation = glGetUniformLocation(m2DProgram, "tex");
        ASSERT_NE(-1, mTexture2DUniformLocation);

        mTextureScaleUniformLocation = glGetUniformLocation(m2DProgram, "textureScale");
        ASSERT_NE(-1, mTextureScaleUniformLocation);

        glUseProgram(m2DProgram);
        glUniform2f(mTextureScaleUniformLocation, 1.0f, 1.0f);
        glUseProgram(0);
        ASSERT_GL_NO_ERROR();
    }

    void TearDown() override
    {
        glDeleteTextures(1, &mTexture2D);
        glDeleteTextures(1, &mTextureCube);
        glDeleteProgram(m2DProgram);
        glDeleteProgram(mCubeProgram);

        ANGLETest::TearDown();
    }

    // Tests CopyTexSubImage with floating point textures of various formats.
    void testFloatCopySubImage(int sourceImageChannels, int destImageChannels)
    {
        // TODO(jmadill): Figure out why this is broken on Intel D3D11
        if (isIntel() && getPlatformRenderer() == EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE)
        {
            std::cout << "Test skipped on Intel D3D11." << std::endl;
            return;
        }

        if (getClientVersion() < 3)
        {
            if (!extensionEnabled("GL_OES_texture_float"))
            {
                std::cout << "Test skipped due to missing GL_OES_texture_float." << std::endl;
                return;
            }

            if ((sourceImageChannels < 3 || destImageChannels < 3) && !extensionEnabled("GL_EXT_texture_rg"))
            {
                std::cout << "Test skipped due to missing GL_EXT_texture_rg." << std::endl;
                return;
            }
        }

        GLfloat sourceImageData[4][16] =
        {
            { // R
                1.0f,
                0.0f,
                0.0f,
                1.0f
            },
            { // RG
                1.0f, 0.0f,
                0.0f, 1.0f,
                0.0f, 0.0f,
                1.0f, 1.0f
            },
            { // RGB
                1.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 1.0f,
                1.0f, 1.0f, 0.0f
            },
            { // RGBA
                1.0f, 0.0f, 0.0f, 1.0f,
                0.0f, 1.0f, 0.0f, 1.0f,
                0.0f, 0.0f, 1.0f, 1.0f,
                1.0f, 1.0f, 0.0f, 1.0f
            },
        };

        GLenum imageFormats[] =
        {
            GL_R32F,
            GL_RG32F,
            GL_RGB32F,
            GL_RGBA32F,
        };

        GLenum sourceUnsizedFormats[] =
        {
            GL_RED,
            GL_RG,
            GL_RGB,
            GL_RGBA,
        };

        GLuint textures[2];

        glGenTextures(2, textures);

        GLfloat *imageData = sourceImageData[sourceImageChannels - 1];
        GLenum sourceImageFormat = imageFormats[sourceImageChannels - 1];
        GLenum sourceUnsizedFormat = sourceUnsizedFormats[sourceImageChannels - 1];
        GLenum destImageFormat = imageFormats[destImageChannels - 1];

        glBindTexture(GL_TEXTURE_2D, textures[0]);
        glTexStorage2DEXT(GL_TEXTURE_2D, 1, sourceImageFormat, 2, 2);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2, 2, sourceUnsizedFormat, GL_FLOAT, imageData);

        if (sourceImageChannels < 3 && !extensionEnabled("GL_EXT_texture_rg"))
        {
            // This is not supported
            ASSERT_GL_ERROR(GL_INVALID_OPERATION);
        }
        else
        {
            ASSERT_GL_NO_ERROR();
        }

        GLuint fbo;
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[0], 0);

        glBindTexture(GL_TEXTURE_2D, textures[1]);
        glTexStorage2DEXT(GL_TEXTURE_2D, 1, destImageFormat, 2, 2);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, 2, 2);
        ASSERT_GL_NO_ERROR();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        drawQuad(m2DProgram, "position", 0.5f);

        int testImageChannels = std::min(sourceImageChannels, destImageChannels);

        EXPECT_PIXEL_EQ(0, 0, 255, 0, 0, 255);
        if (testImageChannels > 1)
        {
            EXPECT_PIXEL_EQ(getWindowHeight() - 1, 0, 0, 255, 0, 255);
            EXPECT_PIXEL_EQ(getWindowHeight() - 1, getWindowWidth() - 1, 255, 255, 0, 255);
            if (testImageChannels > 2)
            {
                EXPECT_PIXEL_EQ(0, getWindowWidth() - 1, 0, 0, 255, 255);
            }
        }

        glDeleteFramebuffers(1, &fbo);
        glDeleteTextures(2, textures);

        ASSERT_GL_NO_ERROR();
    }

    GLuint mTexture2D;
    GLuint mTextureCube;

    GLuint m2DProgram;
    GLuint mCubeProgram;
    GLint mTexture2DUniformLocation;
    GLint mTextureScaleUniformLocation;
};

class TextureTestES3 : public ANGLETest
{
  protected:
    TextureTestES3()
        : m2DArrayTexture(0),
          m2DArrayProgram(0),
          mTextureArrayLocation(-1)
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void SetUp() override
    {
        ANGLETest::SetUp();

        const std::string vertexShaderSource =
            "#version 300 es\n"
            "out vec2 texcoord;\n"
            "in vec4 position;\n"
            "void main() {\n"
            "   gl_Position = vec4(position.xy, 0.0, 1.0);\n"
            "   texcoord = (position.xy * 0.5) + 0.5;\n"
            "}";

        const std::string fragmentShaderSource2DArray =
            "#version 300 es\n"
            "precision highp float;\n"
            "uniform sampler2DArray tex2DArray;\n"
            "in vec2 texcoord;\n"
            "out vec4 fragColor;\n"
            "void main()\n"
            "{\n"
            "    fragColor = texture(tex2DArray, vec3(texcoord.x, texcoord.y, 0.0));\n"
            "}\n";

        m2DArrayProgram = CompileProgram(vertexShaderSource, fragmentShaderSource2DArray);
        ASSERT_NE(0u, m2DArrayProgram);

        mTextureArrayLocation = glGetUniformLocation(m2DArrayProgram, "tex2DArray");
        ASSERT_NE(-1, mTextureArrayLocation);

        glGenTextures(1, &m2DArrayTexture);
        ASSERT_GL_NO_ERROR();
    }

    void TearDown() override
    {
        glDeleteTextures(1, &m2DArrayTexture);
        glDeleteProgram(m2DArrayProgram);
    }

    GLuint m2DArrayTexture;
    GLuint m2DArrayProgram;
    GLint mTextureArrayLocation;
};

TEST_P(TextureTest, NegativeAPISubImage)
{
    glBindTexture(GL_TEXTURE_2D, mTexture2D);
    EXPECT_GL_ERROR(GL_NO_ERROR);

    const GLubyte *pixels[20] = { 0 };
    glTexSubImage2D(GL_TEXTURE_2D, 0, 1, 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
}

TEST_P(TextureTest, ZeroSizedUploads)
{
    glBindTexture(GL_TEXTURE_2D, mTexture2D);
    EXPECT_GL_ERROR(GL_NO_ERROR);

    // Use the texture first to make sure it's in video memory
    glUseProgram(m2DProgram);
    glUniform1i(mTexture2DUniformLocation, 0);
    drawQuad(m2DProgram, "position", 0.5f);

    const GLubyte *pixel[4] = { 0 };

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    EXPECT_GL_NO_ERROR();

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    EXPECT_GL_NO_ERROR();

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    EXPECT_GL_NO_ERROR();
}

// Test drawing with two texture types, to trigger an ANGLE bug in validation
TEST_P(TextureTest, CubeMapBug)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mTexture2D);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureCube);
    EXPECT_GL_ERROR(GL_NO_ERROR);

    glUseProgram(mCubeProgram);
    GLint tex2DUniformLocation = glGetUniformLocation(mCubeProgram, "tex2D");
    GLint texCubeUniformLocation = glGetUniformLocation(mCubeProgram, "texCube");
    EXPECT_NE(-1, tex2DUniformLocation);
    EXPECT_NE(-1, texCubeUniformLocation);
    glUniform1i(tex2DUniformLocation, 0);
    glUniform1i(texCubeUniformLocation, 1);
    drawQuad(mCubeProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
}

// Copy of a test in conformance/textures/texture-mips, to test generate mipmaps
TEST_P(TextureTest, MipmapsTwice)
{
    int px = getWindowWidth() / 2;
    int py = getWindowHeight() / 2;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mTexture2D);

    // Fill with red
    std::vector<GLubyte> pixels(4 * 16 * 16);
    for (size_t pixelId = 0; pixelId < 16 * 16; ++pixelId)
    {
        pixels[pixelId * 4 + 0] = 255;
        pixels[pixelId * 4 + 1] = 0;
        pixels[pixelId * 4 + 2] = 0;
        pixels[pixelId * 4 + 3] = 255;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    glUseProgram(m2DProgram);
    glUniform1i(mTexture2DUniformLocation, 0);
    glUniform2f(mTextureScaleUniformLocation, 0.0625f, 0.0625f);
    drawQuad(m2DProgram, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(px, py, 255, 0, 0, 255);

    // Fill with blue
    for (size_t pixelId = 0; pixelId < 16 * 16; ++pixelId)
    {
        pixels[pixelId * 4 + 0] = 0;
        pixels[pixelId * 4 + 1] = 0;
        pixels[pixelId * 4 + 2] = 255;
        pixels[pixelId * 4 + 3] = 255;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    // Fill with green
    for (size_t pixelId = 0; pixelId < 16 * 16; ++pixelId)
    {
        pixels[pixelId * 4 + 0] = 0;
        pixels[pixelId * 4 + 1] = 255;
        pixels[pixelId * 4 + 2] = 0;
        pixels[pixelId * 4 + 3] = 255;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    drawQuad(m2DProgram, "position", 0.5f);

    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(px, py, 0, 255, 0, 255);
}

// Test creating a FBO with a cube map render target, to test an ANGLE bug
// https://code.google.com/p/angleproject/issues/detail?id=849
TEST_P(TextureTest, CubeMapFBO)
{
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureCube);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, mTextureCube, 0);

    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    glDeleteFramebuffers(1, &fbo);

    EXPECT_GL_NO_ERROR();
}

// Test that glTexSubImage2D works properly when glTexStorage2DEXT has initialized the image with a default color.
TEST_P(TextureTest, TexStorage)
{
    int width = getWindowWidth();
    int height = getWindowHeight();

    GLuint tex2D;
    glGenTextures(1, &tex2D);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex2D);

    // Fill with red
    std::vector<GLubyte> pixels(3 * 16 * 16);
    for (size_t pixelId = 0; pixelId < 16 * 16; ++pixelId)
    {
        pixels[pixelId * 3 + 0] = 255;
        pixels[pixelId * 3 + 1] = 0;
        pixels[pixelId * 3 + 2] = 0;
    }

    // ANGLE internally uses RGBA as the DirectX format for RGB images
    // therefore glTexStorage2DEXT initializes the image to a default color to get a consistent alpha color.
    // The data is kept in a CPU-side image and the image is marked as dirty.
    glTexStorage2DEXT(GL_TEXTURE_2D, 1, GL_RGB8, 16, 16);

    // Initializes the color of the upper-left 8x8 pixels, leaves the other pixels untouched.
    // glTexSubImage2D should take into account that the image is dirty.
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 8, 8, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glUseProgram(m2DProgram);
    glUniform1i(mTexture2DUniformLocation, 0);
    glUniform2f(mTextureScaleUniformLocation, 1.f, 1.f);
    drawQuad(m2DProgram, "position", 0.5f);
    glDeleteTextures(1, &tex2D);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(width / 4, height / 4, 255, 0, 0, 255);

    // Validate that the region of the texture without data has an alpha of 1.0
    GLubyte pixel[4];
    glReadPixels(3 * width / 4, 3 * height / 4, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    EXPECT_EQ(pixel[3], 255);
}

// Test that glTexSubImage2D combined with a PBO works properly when glTexStorage2DEXT has initialized the image with a default color.
TEST_P(TextureTest, TexStorageWithPBO)
{
    if (extensionEnabled("NV_pixel_buffer_object"))
    {
        int width = getWindowWidth();
        int height = getWindowHeight();

        GLuint tex2D;
        glGenTextures(1, &tex2D);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex2D);

        // Fill with red
        std::vector<GLubyte> pixels(3 * 16 * 16);
        for (size_t pixelId = 0; pixelId < 16 * 16; ++pixelId)
        {
            pixels[pixelId * 3 + 0] = 255;
            pixels[pixelId * 3 + 1] = 0;
            pixels[pixelId * 3 + 2] = 0;
        }

        // Read 16x16 region from red backbuffer to PBO
        GLuint pbo;
        glGenBuffers(1, &pbo);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, 3 * 16 * 16, pixels.data(), GL_STATIC_DRAW);

        // ANGLE internally uses RGBA as the DirectX format for RGB images
        // therefore glTexStorage2DEXT initializes the image to a default color to get a consistent alpha color.
        // The data is kept in a CPU-side image and the image is marked as dirty.
        glTexStorage2DEXT(GL_TEXTURE_2D, 1, GL_RGB8, 16, 16);

        // Initializes the color of the upper-left 8x8 pixels, leaves the other pixels untouched.
        // glTexSubImage2D should take into account that the image is dirty.
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 8, 8, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glUseProgram(m2DProgram);
        glUniform1i(mTexture2DUniformLocation, 0);
        glUniform2f(mTextureScaleUniformLocation, 1.f, 1.f);
        drawQuad(m2DProgram, "position", 0.5f);
        glDeleteTextures(1, &tex2D);
        glDeleteTextures(1, &pbo);
        EXPECT_GL_NO_ERROR();
        EXPECT_PIXEL_EQ(3 * width / 4, 3 * height / 4, 0, 0, 0, 255);
        EXPECT_PIXEL_EQ(width / 4, height / 4, 255, 0, 0, 255);
    }
}

// See description on testFloatCopySubImage
TEST_P(TextureTest, CopySubImageFloat_R_R)
{
    testFloatCopySubImage(1, 1);
}

TEST_P(TextureTest, CopySubImageFloat_RG_R)
{
    testFloatCopySubImage(2, 1);
}

TEST_P(TextureTest, CopySubImageFloat_RG_RG)
{
    testFloatCopySubImage(2, 2);
}

TEST_P(TextureTest, CopySubImageFloat_RGB_R)
{
    testFloatCopySubImage(3, 1);
}

TEST_P(TextureTest, CopySubImageFloat_RGB_RG)
{
    testFloatCopySubImage(3, 2);
}

TEST_P(TextureTest, CopySubImageFloat_RGB_RGB)
{
    testFloatCopySubImage(3, 3);
}

TEST_P(TextureTest, CopySubImageFloat_RGBA_R)
{
    testFloatCopySubImage(4, 1);
}

TEST_P(TextureTest, CopySubImageFloat_RGBA_RG)
{
    testFloatCopySubImage(4, 2);
}

TEST_P(TextureTest, CopySubImageFloat_RGBA_RGB)
{
    testFloatCopySubImage(4, 3);
}

TEST_P(TextureTest, CopySubImageFloat_RGBA_RGBA)
{
    testFloatCopySubImage(4, 4);
}

// Port of https://www.khronos.org/registry/webgl/conformance-suites/1.0.3/conformance/textures/texture-npot.html
// Run against GL_ALPHA/UNSIGNED_BYTE format, to ensure that D3D11 Feature Level 9_3 correctly handles GL_ALPHA
TEST_P(TextureTest, TextureNPOT_GL_ALPHA_UBYTE)
{
    const int npotTexSize = 5;
    const int potTexSize = 4; // Should be less than npotTexSize
    GLuint tex2D;

    if (extensionEnabled("GL_OES_texture_npot"))
    {
        // This test isn't applicable if texture_npot is enabled
        return;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &tex2D);
    glBindTexture(GL_TEXTURE_2D, tex2D);

    std::vector<GLubyte> pixels(1 * npotTexSize * npotTexSize);
    for (size_t pixelId = 0; pixelId < npotTexSize * npotTexSize; ++pixelId)
    {
        pixels[pixelId] = 64;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Check that an NPOT texture not on level 0 generates INVALID_VALUE
    glTexImage2D(GL_TEXTURE_2D, 1, GL_ALPHA, npotTexSize, npotTexSize, 0, GL_ALPHA, GL_UNSIGNED_BYTE, pixels.data());
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // Check that an NPOT texture on level 0 succeeds
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, npotTexSize, npotTexSize, 0, GL_ALPHA, GL_UNSIGNED_BYTE, pixels.data());
    EXPECT_GL_NO_ERROR();

    // Check that generateMipmap fails on NPOT
    glGenerateMipmap(GL_TEXTURE_2D);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Check that nothing is drawn if filtering is not correct for NPOT
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glClear(GL_COLOR_BUFFER_BIT);
    drawQuad(m2DProgram, "position", 1.0f);
    EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 0, 0, 0, 255);

    // NPOT texture with TEXTURE_MIN_FILTER not NEAREST or LINEAR should draw with 0,0,0,255
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glClear(GL_COLOR_BUFFER_BIT);
    drawQuad(m2DProgram, "position", 1.0f);
    EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 0, 0, 0, 255);

    // NPOT texture with TEXTURE_MIN_FILTER set to LINEAR should draw
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glClear(GL_COLOR_BUFFER_BIT);
    drawQuad(m2DProgram, "position", 1.0f);
    EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 0, 0, 0, 64);

    // Check that glTexImage2D for POT texture succeeds
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, potTexSize, potTexSize, 0, GL_ALPHA, GL_UNSIGNED_BYTE, pixels.data());
    EXPECT_GL_NO_ERROR();

    // Check that generateMipmap for an POT texture succeeds
    glGenerateMipmap(GL_TEXTURE_2D);
    EXPECT_GL_NO_ERROR();

    // POT texture with TEXTURE_MIN_FILTER set to LINEAR_MIPMAP_LINEAR should draw
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glClear(GL_COLOR_BUFFER_BIT);
    drawQuad(m2DProgram, "position", 1.0f);
    EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 0, 0, 0, 64);
    EXPECT_GL_NO_ERROR();
}

// In the D3D11 renderer, we need to initialize some texture formats, to fill empty channels. EG RBA->RGBA8, with 1.0
// in the alpha channel. This test covers a bug where redefining array textures with these formats does not work as
// expected.
TEST_P(TextureTestES3, RedefineInittableArray)
{
    std::vector<GLubyte> pixelData;
    for (size_t count = 0; count < 5000; count++)
    {
        pixelData.push_back(0u);
        pixelData.push_back(255u);
        pixelData.push_back(0u);
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, m2DArrayTexture);
    glUseProgram(m2DArrayProgram);
    glUniform1i(mTextureArrayLocation, 0);

    // The first draw worked correctly.
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB, 4, 4, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, &pixelData[0]);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    drawQuad(m2DArrayProgram, "position", 1.0f);
    EXPECT_PIXEL_EQ(0, 0, 0, 255, 0, 255);

    // The dimension of the respecification must match the original exactly to trigger the bug.
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB, 4, 4, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, &pixelData[0]);
    drawQuad(m2DArrayProgram, "position", 1.0f);
    EXPECT_PIXEL_EQ(0, 0, 0, 255, 0, 255);

    ASSERT_GL_NO_ERROR();
}

class TextureLimitsTest : public ANGLETest
{
  protected:
    struct RGBA8
    {
        uint8_t R, G, B, A;
    };

    TextureLimitsTest()
        : mProgram(0), mMaxVertexTextures(0), mMaxFragmentTextures(0), mMaxCombinedTextures(0)
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    ~TextureLimitsTest()
    {
        if (mProgram != 0)
        {
            glDeleteProgram(mProgram);
            mProgram = 0;

            if (!mTextures.empty())
            {
                glDeleteTextures(static_cast<GLsizei>(mTextures.size()), &mTextures[0]);
            }
        }
    }

    void SetUp() override
    {
        ANGLETest::SetUp();

        glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &mMaxVertexTextures);
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &mMaxFragmentTextures);
        glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &mMaxCombinedTextures);

        ASSERT_GL_NO_ERROR();
    }

    void compileProgramWithTextureCounts(const std::string &vertexPrefix,
                                         GLint vertexTextureCount,
                                         GLint vertexActiveTextureCount,
                                         const std::string &fragPrefix,
                                         GLint fragmentTextureCount,
                                         GLint fragmentActiveTextureCount)
    {
        std::stringstream vertexShaderStr;
        vertexShaderStr << "attribute vec2 position;\n"
                        << "varying vec4 color;\n"
                        << "varying vec2 texCoord;\n";

        for (GLint textureIndex = 0; textureIndex < vertexTextureCount; ++textureIndex)
        {
            vertexShaderStr << "uniform sampler2D " << vertexPrefix << textureIndex << ";\n";
        }

        vertexShaderStr << "void main() {\n"
                        << "  gl_Position = vec4(position, 0, 1);\n"
                        << "  texCoord = (position * 0.5) + 0.5;\n"
                        << "  color = vec4(0);\n";

        for (GLint textureIndex = 0; textureIndex < vertexActiveTextureCount; ++textureIndex)
        {
            vertexShaderStr << "  color += texture2D(" << vertexPrefix << textureIndex
                            << ", texCoord);\n";
        }

        vertexShaderStr << "}";

        std::stringstream fragmentShaderStr;
        fragmentShaderStr << "varying mediump vec4 color;\n"
                          << "varying mediump vec2 texCoord;\n";

        for (GLint textureIndex = 0; textureIndex < fragmentTextureCount; ++textureIndex)
        {
            fragmentShaderStr << "uniform sampler2D " << fragPrefix << textureIndex << ";\n";
        }

        fragmentShaderStr << "void main() {\n"
                          << "  gl_FragColor = color;\n";

        for (GLint textureIndex = 0; textureIndex < fragmentActiveTextureCount; ++textureIndex)
        {
            fragmentShaderStr << "  gl_FragColor += texture2D(" << fragPrefix << textureIndex
                              << ", texCoord);\n";
        }

        fragmentShaderStr << "}";

        const std::string &vertexShaderSource   = vertexShaderStr.str();
        const std::string &fragmentShaderSource = fragmentShaderStr.str();

        mProgram = CompileProgram(vertexShaderSource, fragmentShaderSource);
    }

    RGBA8 getPixel(GLint texIndex)
    {
        RGBA8 pixel = {static_cast<uint8_t>(texIndex & 0x7u), static_cast<uint8_t>(texIndex >> 3),
                       0, 255u};
        return pixel;
    }

    void initTextures(GLint tex2DCount, GLint texCubeCount)
    {
        GLint totalCount = tex2DCount + texCubeCount;
        mTextures.assign(totalCount, 0);
        glGenTextures(totalCount, &mTextures[0]);
        ASSERT_GL_NO_ERROR();

        std::vector<RGBA8> texData(16 * 16);

        GLint texIndex = 0;
        for (; texIndex < tex2DCount; ++texIndex)
        {
            texData.assign(texData.size(), getPixel(texIndex));
            glActiveTexture(GL_TEXTURE0 + texIndex);
            glBindTexture(GL_TEXTURE_2D, mTextures[texIndex]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                         &texData[0]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }

        ASSERT_GL_NO_ERROR();

        for (; texIndex < texCubeCount; ++texIndex)
        {
            texData.assign(texData.size(), getPixel(texIndex));
            glActiveTexture(GL_TEXTURE0 + texIndex);
            glBindTexture(GL_TEXTURE_CUBE_MAP, mTextures[texIndex]);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, 16, 16, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, &texData[0]);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, 16, 16, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, &texData[0]);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, 16, 16, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, &texData[0]);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, 16, 16, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, &texData[0]);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, 16, 16, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, &texData[0]);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, 16, 16, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, &texData[0]);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }

        ASSERT_GL_NO_ERROR();
    }

    void testWithTextures(GLint vertexTextureCount,
                          const std::string &vertexTexturePrefix,
                          GLint fragmentTextureCount,
                          const std::string &fragmentTexturePrefix)
    {
        // Generate textures
        initTextures(vertexTextureCount + fragmentTextureCount, 0);

        glUseProgram(mProgram);
        RGBA8 expectedSum = {0};
        for (GLint texIndex = 0; texIndex < vertexTextureCount; ++texIndex)
        {
            std::stringstream uniformNameStr;
            uniformNameStr << vertexTexturePrefix << texIndex;
            const std::string &uniformName = uniformNameStr.str();
            GLint location = glGetUniformLocation(mProgram, uniformName.c_str());
            ASSERT_NE(-1, location);

            glUniform1i(location, texIndex);
            RGBA8 contribution = getPixel(texIndex);
            expectedSum.R += contribution.R;
            expectedSum.G += contribution.G;
        }

        for (GLint texIndex = 0; texIndex < fragmentTextureCount; ++texIndex)
        {
            std::stringstream uniformNameStr;
            uniformNameStr << fragmentTexturePrefix << texIndex;
            const std::string &uniformName = uniformNameStr.str();
            GLint location = glGetUniformLocation(mProgram, uniformName.c_str());
            ASSERT_NE(-1, location);

            glUniform1i(location, texIndex + vertexTextureCount);
            RGBA8 contribution = getPixel(texIndex + vertexTextureCount);
            expectedSum.R += contribution.R;
            expectedSum.G += contribution.G;
        }

        ASSERT_GE(256u, expectedSum.G);

        drawQuad(mProgram, "position", 0.5f);
        ASSERT_GL_NO_ERROR();
        EXPECT_PIXEL_EQ(0, 0, expectedSum.R, expectedSum.G, 0, 255);
    }

    GLuint mProgram;
    std::vector<GLuint> mTextures;
    GLint mMaxVertexTextures;
    GLint mMaxFragmentTextures;
    GLint mMaxCombinedTextures;
};

// Test rendering with the maximum vertex texture units.
TEST_P(TextureLimitsTest, MaxVertexTextures)
{
    compileProgramWithTextureCounts("tex", mMaxVertexTextures, mMaxVertexTextures, "tex", 0, 0);
    ASSERT_NE(0u, mProgram);
    ASSERT_GL_NO_ERROR();

    testWithTextures(mMaxVertexTextures, "tex", 0, "tex");
}

// Test rendering with the maximum fragment texture units.
TEST_P(TextureLimitsTest, MaxFragmentTextures)
{
    compileProgramWithTextureCounts("tex", 0, 0, "tex", mMaxFragmentTextures, mMaxFragmentTextures);
    ASSERT_NE(0u, mProgram);
    ASSERT_GL_NO_ERROR();

    testWithTextures(mMaxFragmentTextures, "tex", 0, "tex");
}

// Test rendering with maximum combined texture units.
TEST_P(TextureLimitsTest, MaxCombinedTextures)
{
    // TODO(jmadill): Investigate workaround.
    if (isIntel() && GetParam() == ES2_OPENGL())
    {
        std::cout << "Test skipped on Intel." << std::endl;
        return;
    }

    GLint vertexTextures = mMaxVertexTextures;

    if (vertexTextures + mMaxFragmentTextures > mMaxCombinedTextures)
    {
        vertexTextures = mMaxCombinedTextures - mMaxFragmentTextures;
    }

    compileProgramWithTextureCounts("vtex", vertexTextures, vertexTextures, "ftex",
                                    mMaxFragmentTextures, mMaxFragmentTextures);
    ASSERT_NE(0u, mProgram);
    ASSERT_GL_NO_ERROR();

    testWithTextures(vertexTextures, "vtex", mMaxFragmentTextures, "ftex");
}

// Negative test for exceeding the number of vertex textures
TEST_P(TextureLimitsTest, ExcessiveVertexTextures)
{
    compileProgramWithTextureCounts("tex", mMaxVertexTextures + 1, mMaxVertexTextures + 1, "tex", 0,
                                    0);
    ASSERT_EQ(0u, mProgram);
}

// Negative test for exceeding the number of fragment textures
TEST_P(TextureLimitsTest, ExcessiveFragmentTextures)
{
    compileProgramWithTextureCounts("tex", 0, 0, "tex", mMaxFragmentTextures + 1,
                                    mMaxFragmentTextures + 1);
    ASSERT_EQ(0u, mProgram);
}

// Test active vertex textures under the limit, but excessive textures specified.
TEST_P(TextureLimitsTest, MaxActiveVertexTextures)
{
    compileProgramWithTextureCounts("tex", mMaxVertexTextures + 4, mMaxVertexTextures, "tex", 0, 0);
    ASSERT_NE(0u, mProgram);
    ASSERT_GL_NO_ERROR();

    testWithTextures(mMaxVertexTextures, "tex", 0, "tex");
}

// Test active fragment textures under the limit, but excessive textures specified.
TEST_P(TextureLimitsTest, MaxActiveFragmentTextures)
{
    compileProgramWithTextureCounts("tex", 0, 0, "tex", mMaxFragmentTextures + 4,
                                    mMaxFragmentTextures);
    ASSERT_NE(0u, mProgram);
    ASSERT_GL_NO_ERROR();

    testWithTextures(0, "tex", mMaxFragmentTextures, "tex");
}

// Negative test for pointing two sampler uniforms of different types to the same texture.
TEST_P(TextureLimitsTest, TextureTypeConflict)
{
    const std::string &vertexShader =
        "attribute vec2 position;\n"
        "varying float color;\n"
        "uniform sampler2D tex2D;\n"
        "uniform samplerCube texCube;\n"
        "void main() {\n"
        "  gl_Position = vec4(position, 0, 1);\n"
        "  vec2 texCoord = (position * 0.5) + 0.5;\n"
        "  color = texture2D(tex2D, texCoord).x;\n"
        "  color += textureCube(texCube, vec3(texCoord, 0)).x;\n"
        "}";
    const std::string &fragmentShader =
        "varying mediump float color;\n"
        "void main() {\n"
        "  gl_FragColor = vec4(color, 0, 0, 1);\n"
        "}";

    mProgram = CompileProgram(vertexShader, fragmentShader);
    ASSERT_NE(0u, mProgram);

    initTextures(1, 0);

    glUseProgram(mProgram);
    GLint tex2DLocation = glGetUniformLocation(mProgram, "tex2D");
    ASSERT_NE(-1, tex2DLocation);
    GLint texCubeLocation = glGetUniformLocation(mProgram, "texCube");
    ASSERT_NE(-1, texCubeLocation);

    glUniform1i(tex2DLocation, 0);
    glUniform1i(texCubeLocation, 0);
    ASSERT_GL_NO_ERROR();

    drawQuad(mProgram, "position", 0.5f);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Negative test for rendering with texture outside the valid range.
// TODO(jmadill): Research if this is correct.
TEST_P(TextureLimitsTest, DrawWithTexturePastMaximum)
{
    const std::string &vertexShader =
        "attribute vec2 position;\n"
        "varying float color;\n"
        "uniform sampler2D tex2D;\n"
        "void main() {\n"
        "  gl_Position = vec4(position, 0, 1);\n"
        "  vec2 texCoord = (position * 0.5) + 0.5;\n"
        "  color = texture2D(tex2D, texCoord).x;\n"
        "}";
    const std::string &fragmentShader =
        "varying mediump float color;\n"
        "void main() {\n"
        "  gl_FragColor = vec4(color, 0, 0, 1);\n"
        "}";

    mProgram = CompileProgram(vertexShader, fragmentShader);
    ASSERT_NE(0u, mProgram);

    glUseProgram(mProgram);
    GLint tex2DLocation = glGetUniformLocation(mProgram, "tex2D");
    ASSERT_NE(-1, tex2DLocation);

    glUniform1i(tex2DLocation, mMaxCombinedTextures);
    ASSERT_GL_NO_ERROR();

    drawQuad(mProgram, "position", 0.5f);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
ANGLE_INSTANTIATE_TEST(TextureTest, ES2_D3D9(), ES2_D3D11(), ES2_D3D11_FL9_3()); // TODO(geofflang): Figure out why this test fails on Intel OpenGL
ANGLE_INSTANTIATE_TEST(TextureTestES3, ES3_D3D11(), ES3_OPENGL());
ANGLE_INSTANTIATE_TEST(TextureLimitsTest, ES2_D3D11(), ES2_OPENGL());

} // namespace
