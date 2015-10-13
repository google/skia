//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// D3DImageFormatConversionTest:
//   Basic tests to validate code relating to D3D Image format conversions.

#include "test_utils/ANGLETest.h"

#include "libANGLE/renderer/d3d/imageformats.h"

using namespace angle;

namespace
{

class D3DImageFormatConversionTest : public ANGLETest
{
  protected:
    D3DImageFormatConversionTest()
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

        const std::string vertexShaderSource = SHADER_SOURCE
        (
            precision highp float;
            attribute vec4 position;
            varying vec2 texcoord;

            void main()
            {
                gl_Position = vec4(position.xy, 0.0, 1.0);
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

        m2DProgram = CompileProgram(vertexShaderSource, fragmentShaderSource2D);
        mTexture2DUniformLocation = glGetUniformLocation(m2DProgram, "tex");
    }

    void TearDown() override
    {
        glDeleteProgram(m2DProgram);

        ANGLETest::TearDown();
    }

    // Uses ColorStructType::writeColor to populate initial data for a texture, pass it to glTexImage2D, then render with it.
    // The resulting colors should match the colors passed into ::writeColor.
    template <typename ColorStructType>
    void runTest(GLenum tex2DFormat, GLenum tex2DType)
    {
        gl::ColorF srcColorF[4];
        ColorStructType pixels[4];

        GLuint tex = 0;
        GLuint fbo = 0;
        glGenTextures(1, &tex);
        glGenFramebuffers(1, &fbo);
        EXPECT_GL_NO_ERROR();

        srcColorF[0].red = 1.0f; srcColorF[0].green = 0.0f; srcColorF[0].blue = 0.0f; srcColorF[0].alpha = 1.0f; // Red
        srcColorF[1].red = 0.0f; srcColorF[1].green = 1.0f; srcColorF[1].blue = 0.0f; srcColorF[1].alpha = 1.0f; // Green
        srcColorF[2].red = 0.0f; srcColorF[2].green = 0.0f; srcColorF[2].blue = 1.0f; srcColorF[2].alpha = 1.0f; // Blue
        srcColorF[3].red = 1.0f; srcColorF[3].green = 1.0f; srcColorF[3].blue = 0.0f; srcColorF[3].alpha = 1.0f; // Red + Green (Yellow)

        // Convert the ColorF into the pixels that will be fed to glTexImage2D
        for (unsigned int i = 0; i < 4; i++)
        {
            ColorStructType::writeColor(&(pixels[i]), &(srcColorF[i]));
        }

        // Generate the texture
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, tex2DFormat, 2, 2, 0, tex2DFormat, tex2DType, pixels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        EXPECT_GL_NO_ERROR();

        // Draw a quad using the texture
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(m2DProgram);
        glUniform1i(mTexture2DUniformLocation, 0);
        drawQuad(m2DProgram, "position", 0.5f);
        EXPECT_GL_NO_ERROR();

        // Check that the pixel colors match srcColorF
        EXPECT_PIXEL_EQ(                    0,                    0, 255,   0,   0, 255);
        EXPECT_PIXEL_EQ(getWindowHeight() - 1,                    0,   0, 255,   0, 255);
        EXPECT_PIXEL_EQ(                    0, getWindowWidth() - 1,   0,   0, 255, 255);
        EXPECT_PIXEL_EQ(getWindowHeight() - 1, getWindowWidth() - 1, 255, 255,   0, 255);
        swapBuffers();

        glDeleteFramebuffers(1, &fbo);
        glDeleteTextures(1, &tex);
    }

    GLuint m2DProgram;
    GLint mTexture2DUniformLocation;
};

// Validation test for rx::R4G4B4A4's writeColor functions
TEST_P(D3DImageFormatConversionTest, WriteColorFunctionR4G4B4A4)
{
    // These tests fail on certain Intel machines running an un-updated version of Win7
    // The tests pass after installing the latest updates from Windows Update.
    // TODO: reenable these tests once the bots have been updated
    if (isIntel() && getPlatformRenderer() == EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE)
    {
        std::cout << "Test skipped on Intel D3D11." << std::endl;
        return;
    }

    runTest<rx::R4G4B4A4>(GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4);
}

// Validation test for rx::R5G5B5A1's writeColor functions
TEST_P(D3DImageFormatConversionTest, WriteColorFunctionR5G5B5A1)
{
    // These tests fail on certain Intel machines running an un-updated version of Win7
    // The tests pass after installing the latest updates from Windows Update.
    // TODO: reenable these tests once the bots have been updated
    if (isIntel() && getPlatformRenderer() == EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE)
    {
        std::cout << "Test skipped on Intel D3D11." << std::endl;
        return;
    }

    runTest<rx::R5G5B5A1>(GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1);
}

// Validation test for rx::R5G6B5's writeColor functions
TEST_P(D3DImageFormatConversionTest, WriteColorFunctionR5G6B5)
{
    // These tests fail on certain Intel machines running an un-updated version of Win7
    // The tests pass after installing the latest updates from Windows Update.
    // TODO: reenable these tests once the bots have been updated
    if (isIntel() && getPlatformRenderer() == EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE)
    {
        std::cout << "Test skipped on Intel D3D11." << std::endl;
        return;
    }

    runTest<rx::R5G6B5>(GL_RGB, GL_UNSIGNED_SHORT_5_6_5);
}

// Validation test for rx::R8G8B8A8's writeColor functions
TEST_P(D3DImageFormatConversionTest, WriteColorFunctionR8G8B8A8)
{
    // These tests fail on certain Intel machines running an un-updated version of Win7
    // The tests pass after installing the latest updates from Windows Update.
    // TODO: reenable these tests once the bots have been updated
    if (isIntel() && getPlatformRenderer() == EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE)
    {
        std::cout << "Test skipped on Intel D3D11." << std::endl;
        return;
    }

    runTest<rx::R8G8B8A8>(GL_RGBA, GL_UNSIGNED_BYTE);
}

// Validation test for rx::R8G8B8's writeColor functions
TEST_P(D3DImageFormatConversionTest, WriteColorFunctionR8G8B8)
{
    // These tests fail on certain Intel machines running an un-updated version of Win7
    // The tests pass after installing the latest updates from Windows Update.
    // TODO: reenable these tests once the bots have been updated
    if (isIntel() && getPlatformRenderer() == EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE)
    {
        std::cout << "Test skipped on Intel D3D11." << std::endl;
        return;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    runTest<rx::R8G8B8>(GL_RGB, GL_UNSIGNED_BYTE);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
// Even though this test is only run on Windows (since it includes imageformats.h from the D3D renderer), we can still run the test
// against OpenGL. This is valuable, since it provides extra validation using a renderer that doesn't use imageformats.h itself.
ANGLE_INSTANTIATE_TEST(D3DImageFormatConversionTest, ES2_D3D9(), ES2_D3D11(), ES2_D3D11_FL9_3(), ES2_OPENGL());

} // namespace
