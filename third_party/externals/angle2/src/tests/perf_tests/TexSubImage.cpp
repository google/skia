//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TexSubImageBenchmark:
//   Performace test for ANGLE texture updates.
//

#include <sstream>

#include "ANGLEPerfTest.h"
#include "shader_utils.h"

using namespace angle;

namespace
{

struct TexSubImageParams final : public RenderTestParams
{
    TexSubImageParams()
    {
        // Common default parameters
        majorVersion = 2;
        minorVersion = 0;
        windowWidth = 512;
        windowHeight = 512;

        imageWidth = 1024;
        imageHeight = 1024;
        subImageWidth = 64;
        subImageHeight = 64;
        iterations = 3;
    }

    std::string suffix() const override;

    // Static parameters
    int imageWidth;
    int imageHeight;
    int subImageWidth;
    int subImageHeight;
    unsigned int iterations;
};

inline std::ostream &operator<<(std::ostream &os, const TexSubImageParams &params)
{
    os << params.suffix().substr(1);
    return os;
}

class TexSubImageBenchmark : public ANGLERenderTest,
                             public ::testing::WithParamInterface<TexSubImageParams>
{
  public:
    TexSubImageBenchmark();

    void initializeBenchmark() override;
    void destroyBenchmark() override;
    void beginDrawBenchmark() override;
    void drawBenchmark() override;

  private:
    GLuint createTexture();

    // Handle to a program object
    GLuint mProgram;

    // Attribute locations
    GLint mPositionLoc;
    GLint mTexCoordLoc;

    // Sampler location
    GLint mSamplerLoc;

    // Texture handle
    GLuint mTexture;

    // Buffer handle
    GLuint mVertexBuffer;
    GLuint mIndexBuffer;

    GLubyte *mPixels;
};

std::string TexSubImageParams::suffix() const
{
    // TODO(jmadill)
    return RenderTestParams::suffix();
}

TexSubImageBenchmark::TexSubImageBenchmark()
    : ANGLERenderTest("TexSubImage", GetParam()),
      mProgram(0),
      mPositionLoc(-1),
      mTexCoordLoc(-1),
      mSamplerLoc(-1),
      mTexture(0),
      mVertexBuffer(0),
      mIndexBuffer(0),
      mPixels(NULL)
{
}

GLuint TexSubImageBenchmark::createTexture()
{
    const auto &params = GetParam();

    assert(params.iterations > 0);
    mDrawIterations = params.iterations;

    // Use tightly packed data
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Generate a texture object
    GLuint texture;
    glGenTextures(1, &texture);

    // Bind the texture object
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexStorage2DEXT(GL_TEXTURE_2D, 1, GL_RGBA8, params.imageWidth, params.imageHeight);

    // Set the filtering mode
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return texture;
}

void TexSubImageBenchmark::initializeBenchmark()
{
    const auto &params = GetParam();

    const std::string vs = SHADER_SOURCE
    (
        attribute vec4 a_position;
        attribute vec2 a_texCoord;
        varying vec2 v_texCoord;
        void main()
        {
            gl_Position = a_position;
            v_texCoord = a_texCoord;
        }
    );

    const std::string fs = SHADER_SOURCE
    (
        precision mediump float;
        varying vec2 v_texCoord;
        uniform sampler2D s_texture;
        void main()
        {
            gl_FragColor = texture2D(s_texture, v_texCoord);
        }
    );

    mProgram = CompileProgram(vs, fs);
    ASSERT_TRUE(mProgram != 0);

    // Get the attribute locations
    mPositionLoc = glGetAttribLocation(mProgram, "a_position");
    mTexCoordLoc = glGetAttribLocation(mProgram, "a_texCoord");

    // Get the sampler location
    mSamplerLoc = glGetUniformLocation(mProgram, "s_texture");

    // Build the vertex buffer
    GLfloat vertices[] =
    {
        -0.5f, 0.5f, 0.0f,  // Position 0
        0.0f, 0.0f,        // TexCoord 0
        -0.5f, -0.5f, 0.0f,  // Position 1
        0.0f, 1.0f,        // TexCoord 1
        0.5f, -0.5f, 0.0f,  // Position 2
        1.0f, 1.0f,        // TexCoord 2
        0.5f, 0.5f, 0.0f,  // Position 3
        1.0f, 0.0f         // TexCoord 3
    };

    glGenBuffers(1, &mVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLushort indices[] = { 0, 1, 2, 0, 2, 3 };
    glGenBuffers(1, &mIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Load the texture
    mTexture = createTexture();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    mPixels = new GLubyte[params.subImageWidth * params.subImageHeight * 4];

    // Fill the pixels structure with random data:
    for (int y = 0; y < params.subImageHeight; ++y)
    {
        for (int x = 0; x < params.subImageWidth; ++x)
        {
            int offset = (x + (y * params.subImageWidth)) * 4;
            mPixels[offset + 0] = rand() % 255; // Red
            mPixels[offset + 1] = rand() % 255; // Green
            mPixels[offset + 2] = rand() % 255; // Blue
            mPixels[offset + 3] = 255; // Alpha
        }
    }

    ASSERT_GL_NO_ERROR();
}

void TexSubImageBenchmark::destroyBenchmark()
{
    glDeleteProgram(mProgram);
    glDeleteBuffers(1, &mVertexBuffer);
    glDeleteBuffers(1, &mIndexBuffer);
    glDeleteTextures(1, &mTexture);
    delete[] mPixels;
}

void TexSubImageBenchmark::beginDrawBenchmark()
{
    // Set the viewport
    glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT);

    // Use the program object
    glUseProgram(mProgram);

    // Bind the buffers
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);

    // Load the vertex position
    glVertexAttribPointer(mPositionLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);
    // Load the texture coordinate
    glVertexAttribPointer(mTexCoordLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));

    glEnableVertexAttribArray(mPositionLoc);
    glEnableVertexAttribArray(mTexCoordLoc);

    // Bind the texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mTexture);

    // Set the texture sampler to texture unit to 0
    glUniform1i(mSamplerLoc, 0);

    ASSERT_GL_NO_ERROR();
}

void TexSubImageBenchmark::drawBenchmark()
{
    const auto &params = GetParam();

    for (unsigned int iteration = 0; iteration < params.iterations; ++iteration)
    {
        glTexSubImage2D(GL_TEXTURE_2D, 0,
                        rand() % (params.imageWidth - params.subImageWidth),
                        rand() % (params.imageHeight - params.subImageHeight),
                        params.subImageWidth, params.subImageHeight,
                        GL_RGBA, GL_UNSIGNED_BYTE, mPixels);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    }

    ASSERT_GL_NO_ERROR();
}

TexSubImageParams D3D11Params()
{
    TexSubImageParams params;
    params.eglParameters = egl_platform::D3D11();
    return params;
}

TexSubImageParams D3D9Params()
{
    TexSubImageParams params;
    params.eglParameters = egl_platform::D3D9();
    return params;
}

TexSubImageParams OpenGLParams()
{
    TexSubImageParams params;
    params.eglParameters = egl_platform::OPENGL();
    return params;
}

} // namespace

TEST_P(TexSubImageBenchmark, Run)
{
    run();
}

ANGLE_INSTANTIATE_TEST(TexSubImageBenchmark,
                       D3D11Params(), D3D9Params(), OpenGLParams());
