//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// PointSpritesBenchmark:
//   Performance test for ANGLE point sprites.
//
//
#include "ANGLEPerfTest.h"

#include <iostream>
#include <sstream>

#include "shader_utils.h"
#include "random_utils.h"

using namespace angle;

namespace
{

struct PointSpritesParams final : public RenderTestParams
{
    PointSpritesParams()
    {
        // Common default params
        majorVersion = 2;
        minorVersion = 0;
        windowWidth = 1280;
        windowHeight = 720;
        iterations = 10;
        count = 10;
        size = 3.0f;
        numVaryings = 3;
    }

    std::string suffix() const override;

    unsigned int count;
    float size;
    unsigned int numVaryings;

    // static parameters
    unsigned int iterations;
};

inline std::ostream &operator<<(std::ostream &os, const PointSpritesParams &params)
{
    os << params.suffix().substr(1);
    return os;
}

class PointSpritesBenchmark : public ANGLERenderTest,
                              public ::testing::WithParamInterface<PointSpritesParams>
{
  public:
    PointSpritesBenchmark();

    void initializeBenchmark() override;
    void destroyBenchmark() override;
    void beginDrawBenchmark() override;
    void drawBenchmark() override;

  private:
    GLuint mProgram;
    GLuint mBuffer;
};

std::string PointSpritesParams::suffix() const
{
    std::stringstream strstr;

    strstr << RenderTestParams::suffix()
           << "_" << count << "_" << size << "px"
           << "_" << numVaryings << "vars";

    return strstr.str();
}

PointSpritesBenchmark::PointSpritesBenchmark()
    : ANGLERenderTest("PointSprites", GetParam())
{
}

void PointSpritesBenchmark::initializeBenchmark()
{
    const auto &params = GetParam();

    mDrawIterations = params.iterations;
    ASSERT_TRUE(params.iterations > 0);

    std::stringstream vstrstr;

    // Verify "numVaryings" is within MAX_VARYINGS limit
    GLint maxVaryings;
    glGetIntegerv(GL_MAX_VARYING_VECTORS, &maxVaryings);

    if (params.numVaryings > static_cast<unsigned int>(maxVaryings))
    {
        FAIL() << "Varying count (" << params.numVaryings << ")"
               << " exceeds maximum varyings: " << maxVaryings << std::endl;
    }

    vstrstr << "attribute vec2 vPosition;\n"
               "uniform float uPointSize;\n";

    for (unsigned int varCount = 0; varCount < params.numVaryings; varCount++)
    {
        vstrstr << "varying vec4 v" << varCount << ";\n";
    }

    vstrstr << "void main()\n"
               "{\n";

    for (unsigned int varCount = 0; varCount < params.numVaryings; varCount++)
    {
        vstrstr << "    v" << varCount << " = vec4(1.0);\n";
    }

    vstrstr << "    gl_Position = vec4(vPosition, 0, 1.0);\n"
               "    gl_PointSize = uPointSize;\n"
               "}";

    std::stringstream fstrstr;

    fstrstr << "precision mediump float;\n";

    for (unsigned int varCount = 0; varCount < params.numVaryings; varCount++)
    {
        fstrstr << "varying vec4 v" << varCount << ";\n";
    }

    fstrstr << "void main()\n"
               "{\n"
               "    vec4 colorOut = vec4(1.0, 0.0, 0.0, 1.0);\n";

    for (unsigned int varCount = 0; varCount < params.numVaryings; varCount++)
    {
        fstrstr << "    colorOut.r += v" << varCount << ".r;\n";
    }

    fstrstr << "    gl_FragColor = colorOut;\n"
               "}\n";

    mProgram = CompileProgram(vstrstr.str(), fstrstr.str());
    ASSERT_TRUE(mProgram != 0);

    // Use the program object
    glUseProgram(mProgram);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    std::vector<float> vertexPositions(params.count * 2);
    for (size_t pointIndex = 0; pointIndex < vertexPositions.size(); ++pointIndex)
    {
        vertexPositions[pointIndex] = RandomBetween(-1.0f, 1.0f);
    }

    glGenBuffers(1, &mBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertexPositions.size() * sizeof(float), &vertexPositions[0], GL_STATIC_DRAW);

    int positionLocation = glGetAttribLocation(mProgram, "vPosition");
    ASSERT_TRUE(positionLocation != -1);

    glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(positionLocation);

    // Set the viewport
    glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

    int pointSizeLocation = glGetUniformLocation(mProgram, "uPointSize");
    ASSERT_TRUE(pointSizeLocation != -1);

    glUniform1f(pointSizeLocation, params.size);

    ASSERT_GL_NO_ERROR();
}

void PointSpritesBenchmark::destroyBenchmark()
{
    glDeleteProgram(mProgram);
    glDeleteBuffers(1, &mBuffer);
}

void PointSpritesBenchmark::beginDrawBenchmark()
{
    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT);
}

void PointSpritesBenchmark::drawBenchmark()
{
    const auto &params = GetParam();

    for (unsigned int it = 0; it < params.iterations; it++)
    {
        //TODO(jmadill): Indexed point rendering. ANGLE is bad at this.
        glDrawArrays(GL_POINTS, 0, params.count);
    }

    ASSERT_GL_NO_ERROR();
}

PointSpritesParams D3D11Params()
{
    PointSpritesParams params;
    params.eglParameters = egl_platform::D3D11();
    return params;
}

PointSpritesParams D3D9Params()
{
    PointSpritesParams params;
    params.eglParameters = egl_platform::D3D9();
    return params;
}

PointSpritesParams OpenGLParams()
{
    PointSpritesParams params;
    params.eglParameters = egl_platform::OPENGL();
    return params;
}

} // namespace

TEST_P(PointSpritesBenchmark, Run)
{
    run();
}

ANGLE_INSTANTIATE_TEST(PointSpritesBenchmark,
                       D3D11Params(), D3D9Params(), OpenGLParams());
