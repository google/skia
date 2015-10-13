//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DrawCallPerf:
//   Performance tests for ANGLE draw call overhead.
//

#include <sstream>

#include "ANGLEPerfTest.h"
#include "shader_utils.h"

using namespace angle;

namespace
{

struct DrawCallPerfParams final : public RenderTestParams
{
    // Common default options
    DrawCallPerfParams()
    {
        majorVersion = 2;
        minorVersion = 0;
        windowWidth = 256;
        windowHeight = 256;
        iterations = 50;
        numTris = 1;
        runTimeSeconds = 10.0;
    }

    std::string suffix() const override
    {
        std::stringstream strstr;

        strstr << RenderTestParams::suffix();

        if (numTris == 0)
        {
            strstr << "_validation_only";
        }

        if (eglParameters.deviceType == EGL_PLATFORM_ANGLE_DEVICE_TYPE_NULL_ANGLE)
        {
            strstr << "_null";
        }

        return strstr.str();
    }

    unsigned int iterations;
    double runTimeSeconds;
    int numTris;
};

inline std::ostream &operator<<(std::ostream &os, const DrawCallPerfParams &params)
{
    os << params.suffix().substr(1);
    return os;
}

class DrawCallPerfBenchmark : public ANGLERenderTest,
                              public ::testing::WithParamInterface<DrawCallPerfParams>
{
  public:
    DrawCallPerfBenchmark();

    void initializeBenchmark() override;
    void destroyBenchmark() override;
    void beginDrawBenchmark() override;
    void drawBenchmark() override;

  private:
    GLuint mProgram;
    GLuint mBuffer;
    int mNumTris;
};

DrawCallPerfBenchmark::DrawCallPerfBenchmark()
    : ANGLERenderTest("DrawCallPerf", GetParam()),
      mProgram(0),
      mBuffer(0),
      mNumTris(GetParam().numTris)
{
    mRunTimeSeconds = GetParam().runTimeSeconds;
}

void DrawCallPerfBenchmark::initializeBenchmark()
{
    const auto &params = GetParam();

    ASSERT_TRUE(params.iterations > 0);
    mDrawIterations = params.iterations;

    const std::string vs = SHADER_SOURCE
    (
        attribute vec2 vPosition;
        uniform float uScale;
        uniform float uOffset;
        void main()
        {
            gl_Position = vec4(vPosition * vec2(uScale) - vec2(uOffset), 0, 1);
        }
    );

    const std::string fs = SHADER_SOURCE
    (
        precision mediump float;
        void main()
        {
            gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
        }
    );

    mProgram = CompileProgram(vs, fs);
    ASSERT_TRUE(mProgram != 0);

    // Use the program object
    glUseProgram(mProgram);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    std::vector<GLfloat> floatData;

    for (int quadIndex = 0; quadIndex < mNumTris; ++quadIndex)
    {
        floatData.push_back(1);
        floatData.push_back(2);
        floatData.push_back(0);
        floatData.push_back(0);
        floatData.push_back(2);
        floatData.push_back(0);
    }

    glGenBuffers(1, &mBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);

    // To avoid generating GL errors when testing validation-only
    if (floatData.empty())
    {
        floatData.push_back(0.0f);
    }

    glBufferData(GL_ARRAY_BUFFER, floatData.size() * sizeof(GLfloat), &floatData[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // Set the viewport
    glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

    GLfloat scale = 0.5f;
    GLfloat offset = 0.5f;

    glUniform1f(glGetUniformLocation(mProgram, "uScale"), scale);
    glUniform1f(glGetUniformLocation(mProgram, "uOffset"), offset);

    ASSERT_GL_NO_ERROR();
}

void DrawCallPerfBenchmark::destroyBenchmark()
{
    glDeleteProgram(mProgram);
    glDeleteBuffers(1, &mBuffer);
}

void DrawCallPerfBenchmark::beginDrawBenchmark()
{
    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT);
}

void DrawCallPerfBenchmark::drawBenchmark()
{
    const auto &params = GetParam();

    for (unsigned int it = 0; it < params.iterations; it++)
    {
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(3 * mNumTris));
    }

    ASSERT_GL_NO_ERROR();
}

using namespace egl_platform;

DrawCallPerfParams DrawCallPerfD3D11Params(bool useNullDevice)
{
    DrawCallPerfParams params;
    params.eglParameters = useNullDevice ? D3D11_NULL() : D3D11();
    return params;
}

DrawCallPerfParams DrawCallPerfD3D9Params(bool useNullDevice)
{
    DrawCallPerfParams params;
    params.eglParameters = useNullDevice ? D3D9_NULL() : D3D9();
    return params;
}

DrawCallPerfParams DrawCallPerfOpenGLParams(bool useNullDevice)
{
    DrawCallPerfParams params;
    params.eglParameters = useNullDevice ? OPENGL_NULL() : OPENGL();
    return params;
}

DrawCallPerfParams DrawCallPerfValidationOnly()
{
    DrawCallPerfParams params;
    params.eglParameters = DEFAULT();
    params.iterations = 100;
    params.numTris = 0;
    params.runTimeSeconds = 5.0;
    return params;
}

TEST_P(DrawCallPerfBenchmark, Run)
{
    run();
}

ANGLE_INSTANTIATE_TEST(DrawCallPerfBenchmark,
                       DrawCallPerfD3D11Params(false),
                       DrawCallPerfD3D9Params(false),
                       DrawCallPerfOpenGLParams(false),
                       DrawCallPerfD3D11Params(true),
                       DrawCallPerfD3D9Params(true),
                       DrawCallPerfOpenGLParams(true),
                       DrawCallPerfValidationOnly());

} // namespace
