//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

#include <cmath>

using namespace angle;

namespace
{

class UniformTest : public ANGLETest
{
  protected:
    UniformTest() : mProgram(0), mUniformFLocation(-1), mUniformILocation(-1), mUniformBLocation(-1)
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

        const std::string &vertexShader = "void main() { gl_Position = vec4(1); }";
        const std::string &fragShader =
            "precision mediump float;\n"
            "uniform float uniF;\n"
            "uniform int uniI;\n"
            "uniform bool uniB;\n"
            "uniform bool uniBArr[4];\n"
            "void main() {\n"
            "  gl_FragColor = vec4(uniF + float(uniI));\n"
            "  gl_FragColor += vec4(uniB ? 1.0 : 0.0);\n"
            "  gl_FragColor += vec4(uniBArr[0] ? 1.0 : 0.0);\n"
            "}";

        mProgram = CompileProgram(vertexShader, fragShader);
        ASSERT_NE(mProgram, 0u);

        mUniformFLocation = glGetUniformLocation(mProgram, "uniF");
        ASSERT_NE(mUniformFLocation, -1);

        mUniformILocation = glGetUniformLocation(mProgram, "uniI");
        ASSERT_NE(mUniformILocation, -1);

        mUniformBLocation = glGetUniformLocation(mProgram, "uniB");
        ASSERT_NE(mUniformBLocation, -1);

        ASSERT_GL_NO_ERROR();
    }

    void TearDown() override
    {
        glDeleteProgram(mProgram);
        ANGLETest::TearDown();
    }

    GLuint mProgram;
    GLint mUniformFLocation;
    GLint mUniformILocation;
    GLint mUniformBLocation;
};

TEST_P(UniformTest, GetUniformNoCurrentProgram)
{
    glUseProgram(mProgram);
    glUniform1f(mUniformFLocation, 1.0f);
    glUniform1i(mUniformILocation, 1);
    glUseProgram(0);

    GLfloat f;
    glGetnUniformfvEXT(mProgram, mUniformFLocation, 4, &f);
    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(1.0f, f);

    glGetUniformfv(mProgram, mUniformFLocation, &f);
    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(1.0f, f);

    GLint i;
    glGetnUniformivEXT(mProgram, mUniformILocation, 4, &i);
    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(1, i);

    glGetUniformiv(mProgram, mUniformILocation, &i);
    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(1, i);
}

TEST_P(UniformTest, UniformArrayLocations)
{
    // TODO(geofflang): Figure out why this is broken on Intel OpenGL
    if (isIntel() && getPlatformRenderer() == EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE)
    {
        std::cout << "Test skipped on Intel OpenGL." << std::endl;
        return;
    }

    const std::string vertexShader = SHADER_SOURCE
    (
        precision mediump float;
        uniform float uPosition[4];
        void main(void)
        {
            gl_Position = vec4(uPosition[0], uPosition[1], uPosition[2], uPosition[3]);
        }
    );

    const std::string fragShader = SHADER_SOURCE
    (
        precision mediump float;
        uniform float uColor[4];
        void main(void)
        {
            gl_FragColor = vec4(uColor[0], uColor[1], uColor[2], uColor[3]);
        }
    );

    GLuint program = CompileProgram(vertexShader, fragShader);
    ASSERT_NE(program, 0u);

    // Array index zero should be equivalent to the un-indexed uniform
    EXPECT_NE(-1, glGetUniformLocation(program, "uPosition"));
    EXPECT_EQ(glGetUniformLocation(program, "uPosition"), glGetUniformLocation(program, "uPosition[0]"));

    EXPECT_NE(-1, glGetUniformLocation(program, "uColor"));
    EXPECT_EQ(glGetUniformLocation(program, "uColor"), glGetUniformLocation(program, "uColor[0]"));

    // All array uniform locations should be unique
    GLint positionLocations[4] =
    {
        glGetUniformLocation(program, "uPosition[0]"),
        glGetUniformLocation(program, "uPosition[1]"),
        glGetUniformLocation(program, "uPosition[2]"),
        glGetUniformLocation(program, "uPosition[3]"),
    };

    GLint colorLocations[4] =
    {
        glGetUniformLocation(program, "uColor[0]"),
        glGetUniformLocation(program, "uColor[1]"),
        glGetUniformLocation(program, "uColor[2]"),
        glGetUniformLocation(program, "uColor[3]"),
    };

    for (size_t i = 0; i < 4; i++)
    {
        EXPECT_NE(-1, positionLocations[i]);
        EXPECT_NE(-1, colorLocations[i]);

        for (size_t j = i + 1; j < 4; j++)
        {
            EXPECT_NE(positionLocations[i], positionLocations[j]);
            EXPECT_NE(colorLocations[i], colorLocations[j]);
        }
    }

    glDeleteProgram(program);
}

// Test that float to integer GetUniform rounds values correctly.
TEST_P(UniformTest, FloatUniformStateQuery)
{
    std::vector<GLfloat> inValues;
    std::vector<GLfloat> expectedFValues;
    std::vector<GLint> expectedIValues;

    GLfloat intMaxF = static_cast<GLfloat>(std::numeric_limits<GLint>::max());
    GLfloat intMinF = static_cast<GLfloat>(std::numeric_limits<GLint>::min());

    // TODO(jmadill): Investigate rounding of .5

    inValues.push_back(-1.0f);
    inValues.push_back(-0.6f);
    // inValues.push_back(-0.5f); // undefined behaviour?
    inValues.push_back(-0.4f);
    inValues.push_back(0.0f);
    inValues.push_back(0.4f);
    // inValues.push_back(0.5f); // undefined behaviour?
    inValues.push_back(0.6f);
    inValues.push_back(1.0f);
    inValues.push_back(999999.2f);
    inValues.push_back(intMaxF * 2.0f);
    inValues.push_back(intMaxF + 1.0f);
    inValues.push_back(intMinF * 2.0f);
    inValues.push_back(intMinF - 1.0f);

    for (GLfloat value : inValues)
    {
        expectedFValues.push_back(value);

        GLfloat clampedValue = std::max(intMinF, std::min(intMaxF, value));
        GLfloat rounded = roundf(clampedValue);
        expectedIValues.push_back(static_cast<GLint>(rounded));
    }

    glUseProgram(mProgram);
    ASSERT_GL_NO_ERROR();

    for (size_t index = 0; index < inValues.size(); ++index)
    {
        GLfloat inValue       = inValues[index];
        GLfloat expectedValue = expectedFValues[index];

        glUniform1f(mUniformFLocation, inValue);
        GLfloat testValue;
        glGetUniformfv(mProgram, mUniformFLocation, &testValue);
        ASSERT_GL_NO_ERROR();
        EXPECT_EQ(expectedValue, testValue);
    }

    for (size_t index = 0; index < inValues.size(); ++index)
    {
        GLfloat inValue     = inValues[index];
        GLint expectedValue = expectedIValues[index];

        glUniform1f(mUniformFLocation, inValue);
        GLint testValue;
        glGetUniformiv(mProgram, mUniformFLocation, &testValue);
        ASSERT_GL_NO_ERROR();
        EXPECT_EQ(expectedValue, testValue);
    }
}

// Test that integer to float GetUniform rounds values correctly.
TEST_P(UniformTest, IntUniformStateQuery)
{
    std::vector<GLint> inValues;
    std::vector<GLint> expectedIValues;
    std::vector<GLfloat> expectedFValues;

    GLint intMax = std::numeric_limits<GLint>::max();
    GLint intMin = std::numeric_limits<GLint>::min();

    inValues.push_back(-1);
    inValues.push_back(0);
    inValues.push_back(1);
    inValues.push_back(999999);
    inValues.push_back(intMax);
    inValues.push_back(intMax - 1);
    inValues.push_back(intMin);
    inValues.push_back(intMin + 1);

    for (GLint value : inValues)
    {
        expectedIValues.push_back(value);
        expectedFValues.push_back(static_cast<GLfloat>(value));
    }

    glUseProgram(mProgram);
    ASSERT_GL_NO_ERROR();

    for (size_t index = 0; index < inValues.size(); ++index)
    {
        GLint inValue       = inValues[index];
        GLint expectedValue = expectedIValues[index];

        glUniform1i(mUniformILocation, inValue);
        GLint testValue;
        glGetUniformiv(mProgram, mUniformILocation, &testValue);
        ASSERT_GL_NO_ERROR();
        EXPECT_EQ(expectedValue, testValue);
    }

    for (size_t index = 0; index < inValues.size(); ++index)
    {
        GLint inValue         = inValues[index];
        GLfloat expectedValue = expectedFValues[index];

        glUniform1i(mUniformILocation, inValue);
        GLfloat testValue;
        glGetUniformfv(mProgram, mUniformILocation, &testValue);
        ASSERT_GL_NO_ERROR();
        EXPECT_EQ(expectedValue, testValue);
    }
}

// Test that queries of boolean uniforms round correctly.
TEST_P(UniformTest, BooleanUniformStateQuery)
{
    glUseProgram(mProgram);
    GLint intValue     = 0;
    GLfloat floatValue = 0.0f;

    // Calling Uniform1i
    glUniform1i(mUniformBLocation, GL_FALSE);

    glGetUniformiv(mProgram, mUniformBLocation, &intValue);
    EXPECT_EQ(0, intValue);

    glGetUniformfv(mProgram, mUniformBLocation, &floatValue);
    EXPECT_EQ(0.0f, floatValue);

    glUniform1i(mUniformBLocation, GL_TRUE);

    glGetUniformiv(mProgram, mUniformBLocation, &intValue);
    EXPECT_EQ(1, intValue);

    glGetUniformfv(mProgram, mUniformBLocation, &floatValue);
    EXPECT_EQ(1.0f, floatValue);

    // Calling Uniform1f
    glUniform1f(mUniformBLocation, 0.0f);

    glGetUniformiv(mProgram, mUniformBLocation, &intValue);
    EXPECT_EQ(0, intValue);

    glGetUniformfv(mProgram, mUniformBLocation, &floatValue);
    EXPECT_EQ(0.0f, floatValue);

    glUniform1f(mUniformBLocation, 1.0f);

    glGetUniformiv(mProgram, mUniformBLocation, &intValue);
    EXPECT_EQ(1, intValue);

    glGetUniformfv(mProgram, mUniformBLocation, &floatValue);
    EXPECT_EQ(1.0f, floatValue);

    ASSERT_GL_NO_ERROR();
}

// Test queries for arrays of boolean uniforms.
TEST_P(UniformTest, BooleanArrayUniformStateQuery)
{
    glUseProgram(mProgram);
    GLint intValues[4]     = {0};
    GLfloat floatValues[4] = {0.0f};
    GLint boolValuesi[4]   = {0, 1, 0, 1};
    GLfloat boolValuesf[4] = {0, 1, 0, 1};

    GLint location = glGetUniformLocation(mProgram, "uniBArr");

    // Calling Uniform1iv
    glUniform1iv(location, 4, boolValuesi);

    glGetUniformiv(mProgram, location, intValues);
    for (unsigned int idx = 0; idx < 4; ++idx)
    {
        EXPECT_EQ(boolValuesi[idx], intValues[idx]);
    }

    glGetUniformfv(mProgram, location, floatValues);
    for (unsigned int idx = 0; idx < 4; ++idx)
    {
        EXPECT_EQ(boolValuesf[idx], floatValues[idx]);
    }

    // Calling Uniform1fv
    glUniform1fv(location, 4, boolValuesf);

    glGetUniformiv(mProgram, location, intValues);
    for (unsigned int idx = 0; idx < 4; ++idx)
    {
        EXPECT_EQ(boolValuesi[idx], intValues[idx]);
    }

    glGetUniformfv(mProgram, location, floatValues);
    for (unsigned int idx = 0; idx < 4; ++idx)
    {
        EXPECT_EQ(boolValuesf[idx], floatValues[idx]);
    }

    ASSERT_GL_NO_ERROR();
}

class UniformTestES3 : public ANGLETest
{
  protected:
    UniformTestES3() : mProgram(0) {}

    void SetUp() override
    {
        ANGLETest::SetUp();

        const std::string &vertexShader =
            "#version 300 es\n"
            "void main() { gl_Position = vec4(1); }";
        const std::string &fragShader =
            "#version 300 es\n"
            "precision mediump float;\n"
            "uniform mat3x2 uniMat3x2[5];\n"
            "out vec4 color;\n"
            "void main() {\n"
            "  color = vec4(uniMat3x2[0][0][0]);\n"
            "}";

        mProgram = CompileProgram(vertexShader, fragShader);
        ASSERT_NE(mProgram, 0u);
    }

    void TearDown() override
    {
        if (mProgram != 0)
        {
            glDeleteProgram(mProgram);
            mProgram = 0;
        }
    }

    GLuint mProgram;
};

// Test queries for transposed arrays of non-square matrix uniforms.
TEST_P(UniformTestES3, TranposedMatrixArrayUniformStateQuery)
{
    glUseProgram(mProgram);

    std::vector<GLfloat> transposedValues;

    for (size_t arrayElement = 0; arrayElement < 5; ++arrayElement)
    {
        transposedValues.push_back(1.0f + arrayElement);
        transposedValues.push_back(3.0f + arrayElement);
        transposedValues.push_back(5.0f + arrayElement);
        transposedValues.push_back(2.0f + arrayElement);
        transposedValues.push_back(4.0f + arrayElement);
        transposedValues.push_back(6.0f + arrayElement);
    }

    // Setting as a clump
    GLint baseLocation = glGetUniformLocation(mProgram, "uniMat3x2");
    ASSERT_NE(-1, baseLocation);

    glUniformMatrix3x2fv(baseLocation, 5, GL_TRUE, &transposedValues[0]);

    for (size_t arrayElement = 0; arrayElement < 5; ++arrayElement)
    {
        std::stringstream nameStr;
        nameStr << "uniMat3x2[" << arrayElement << "]";
        std::string name = nameStr.str();
        GLint location = glGetUniformLocation(mProgram, name.c_str());
        ASSERT_NE(-1, location);

        std::vector<GLfloat> sequentialValues(6, 0);
        glGetUniformfv(mProgram, location, &sequentialValues[0]);

        ASSERT_GL_NO_ERROR();

        for (size_t comp = 0; comp < 6; ++comp)
        {
            EXPECT_EQ(static_cast<GLfloat>(comp + 1 + arrayElement), sequentialValues[comp]);
        }
    }
}

// Check that sampler uniforms only show up one time in the list
TEST_P(UniformTest, SamplerUniformsAppearOnce)
{
    const std::string &vertShader =
        "attribute vec2 position;\n"
        "uniform sampler2D tex2D;\n"
        "varying vec4 color;\n"
        "void main() {\n"
        "  gl_Position = vec4(position, 0, 1);\n"
        "  color = texture2D(tex2D, vec2(0));\n"
        "}";

    const std::string &fragShader =
        "precision mediump float;\n"
        "varying vec4 color;\n"
        "uniform sampler2D tex2D;\n"
        "void main() {\n"
        "  gl_FragColor = texture2D(tex2D, vec2(0)) + color;\n"
        "}";

    GLuint program = CompileProgram(vertShader, fragShader);
    ASSERT_NE(0u, program);

    GLint activeUniformsCount = 0;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &activeUniformsCount);
    ASSERT_EQ(1, activeUniformsCount);

    GLint size       = 0;
    GLenum type      = GL_NONE;
    GLchar name[120] = {0};
    glGetActiveUniform(program, 0, 100, nullptr, &size, &type, name);
    EXPECT_EQ(1, size);
    EXPECT_GLENUM_EQ(GL_SAMPLER_2D, type);
    EXPECT_STREQ("tex2D", name);

    EXPECT_GL_NO_ERROR();

    glDeleteProgram(program);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
ANGLE_INSTANTIATE_TEST(UniformTest, ES2_D3D9(), ES2_D3D11(), ES2_OPENGL());
ANGLE_INSTANTIATE_TEST(UniformTestES3, ES3_D3D11(), ES3_OPENGL());

} // namespace
