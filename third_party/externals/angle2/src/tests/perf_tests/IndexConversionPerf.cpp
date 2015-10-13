//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// IndexConversionPerf:
//   Performance tests for ANGLE index conversion in D3D11.
//

#include <sstream>

#include "ANGLEPerfTest.h"
#include "shader_utils.h"

using namespace angle;

namespace
{

struct IndexConversionPerfParams final : public RenderTestParams
{
    std::string suffix() const override
    {
        std::stringstream strstr;

        strstr << RenderTestParams::suffix();

        return strstr.str();
    }

    unsigned int iterations;
    unsigned int numIndexTris;
};

// Provide a custom gtest parameter name function for IndexConversionPerfParams
// that includes the number of iterations and triangles in the test parameter name.
// This also fixes the resolution of the overloaded operator<< on MSVC.
std::ostream &operator<<(std::ostream &stream, const IndexConversionPerfParams &param)
{
    const PlatformParameters &platform = param;
    stream << platform << "_" << param.iterations << "_" << param.numIndexTris;
    return stream;
}

class IndexConversionPerfTest : public ANGLERenderTest,
                                public ::testing::WithParamInterface<IndexConversionPerfParams>
{
  public:
    IndexConversionPerfTest();

    void initializeBenchmark() override;
    void destroyBenchmark() override;
    void beginDrawBenchmark() override;
    void drawBenchmark() override;

    void updateBufferData();

  private:
    GLuint mProgram;
    GLuint mVertexBuffer;
    GLuint mIndexBuffer;
    std::vector<GLushort> mIndexData;
};

IndexConversionPerfTest::IndexConversionPerfTest()
    : ANGLERenderTest("IndexConversionPerfTest", GetParam()),
      mProgram(0),
      mVertexBuffer(0),
      mIndexBuffer(0)
{
    mRunTimeSeconds = 3.0;
}

void IndexConversionPerfTest::initializeBenchmark()
{
    const auto &params = GetParam();

    ASSERT_TRUE(params.iterations > 0);
    ASSERT_TRUE(params.numIndexTris > 0);

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

    // Initialize the vertex data
    std::vector<GLfloat> floatData;

    size_t numTris = std::numeric_limits<GLushort>::max() / 3 + 1;
    for (size_t triIndex = 0; triIndex < numTris; ++triIndex)
    {
        floatData.push_back(1);
        floatData.push_back(2);
        floatData.push_back(0);
        floatData.push_back(0);
        floatData.push_back(2);
        floatData.push_back(0);
    }

    glGenBuffers(1, &mVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, floatData.size() * sizeof(GLfloat), &floatData[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // Initialize the index buffer
    for (unsigned int triIndex = 0; triIndex < params.numIndexTris; ++triIndex)
    {
        mIndexData.push_back(std::numeric_limits<GLushort>::max());
        mIndexData.push_back(1);
        mIndexData.push_back(2);
    }

    glGenBuffers(1, &mIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
    updateBufferData();

    // Set the viewport
    glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

    GLfloat scale = 0.5f;
    GLfloat offset = 0.5f;

    glUniform1f(glGetUniformLocation(mProgram, "uScale"), scale);
    glUniform1f(glGetUniformLocation(mProgram, "uOffset"), offset);

    ASSERT_TRUE(glGetError() == GL_NO_ERROR);
}

void IndexConversionPerfTest::updateBufferData()
{
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndexData.size() * sizeof(mIndexData[0]), &mIndexData[0], GL_STATIC_DRAW);
}

void IndexConversionPerfTest::destroyBenchmark()
{
    glDeleteProgram(mProgram);
    glDeleteBuffers(1, &mVertexBuffer);
    glDeleteBuffers(1, &mIndexBuffer);
}

void IndexConversionPerfTest::beginDrawBenchmark()
{
    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT);
}

void IndexConversionPerfTest::drawBenchmark()
{
    const auto &params = GetParam();

    // Trigger an update to ensure we convert once a frame
    updateBufferData();

    for (unsigned int it = 0; it < params.iterations; it++)
    {
        glDrawElements(GL_TRIANGLES,
                       static_cast<GLsizei>(params.numIndexTris * 3 - 1),
                       GL_UNSIGNED_SHORT,
                       reinterpret_cast<GLvoid*>(0));
    }

    EXPECT_TRUE(glGetError() == GL_NO_ERROR);
}

IndexConversionPerfParams IndexConversionPerfD3D11Params()
{
    IndexConversionPerfParams params;
    params.eglParameters = egl_platform::D3D11_NULL();
    params.majorVersion = 2;
    params.minorVersion = 0;
    params.windowWidth = 256;
    params.windowHeight = 256;
    params.iterations = 15;
    params.numIndexTris = 3000;
    return params;
}

TEST_P(IndexConversionPerfTest, Run)
{
    run();
}

ANGLE_INSTANTIATE_TEST(IndexConversionPerfTest,
                       IndexConversionPerfD3D11Params());

} // namespace
