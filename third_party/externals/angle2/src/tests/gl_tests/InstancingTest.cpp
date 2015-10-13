//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

using namespace angle;

class InstancingTest : public ANGLETest
{
  protected:
    InstancingTest()
    {
        setWindowWidth(256);
        setWindowHeight(256);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    virtual void SetUp()
    {
        ANGLETest::SetUp();

        mVertexAttribDivisorANGLE = NULL;
        mDrawArraysInstancedANGLE = NULL;
        mDrawElementsInstancedANGLE = NULL;

        char *extensionString = (char*)glGetString(GL_EXTENSIONS);
        if (strstr(extensionString, "GL_ANGLE_instanced_arrays"))
        {
            mVertexAttribDivisorANGLE = (PFNGLVERTEXATTRIBDIVISORANGLEPROC)eglGetProcAddress("glVertexAttribDivisorANGLE");
            mDrawArraysInstancedANGLE = (PFNGLDRAWARRAYSINSTANCEDANGLEPROC)eglGetProcAddress("glDrawArraysInstancedANGLE");
            mDrawElementsInstancedANGLE = (PFNGLDRAWELEMENTSINSTANCEDANGLEPROC)eglGetProcAddress("glDrawElementsInstancedANGLE");
        }

        ASSERT_TRUE(mVertexAttribDivisorANGLE != NULL);
        ASSERT_TRUE(mDrawArraysInstancedANGLE != NULL);
        ASSERT_TRUE(mDrawElementsInstancedANGLE != NULL);

        // Initialize the vertex and index vectors
        GLfloat qvertex1[3] = {-quadRadius,  quadRadius, 0.0f};
        GLfloat qvertex2[3] = {-quadRadius, -quadRadius, 0.0f};
        GLfloat qvertex3[3] = { quadRadius, -quadRadius, 0.0f};
        GLfloat qvertex4[3] = { quadRadius,  quadRadius, 0.0f};
        mQuadVertices.insert(mQuadVertices.end(), qvertex1, qvertex1 + 3);
        mQuadVertices.insert(mQuadVertices.end(), qvertex2, qvertex2 + 3);
        mQuadVertices.insert(mQuadVertices.end(), qvertex3, qvertex3 + 3);
        mQuadVertices.insert(mQuadVertices.end(), qvertex4, qvertex4 + 3);

        GLfloat coord1[2] = {0.0f, 0.0f};
        GLfloat coord2[2] = {0.0f, 1.0f};
        GLfloat coord3[2] = {1.0f, 1.0f};
        GLfloat coord4[2] = {1.0f, 0.0f};
        mTexcoords.insert(mTexcoords.end(), coord1, coord1 + 2);
        mTexcoords.insert(mTexcoords.end(), coord2, coord2 + 2);
        mTexcoords.insert(mTexcoords.end(), coord3, coord3 + 2);
        mTexcoords.insert(mTexcoords.end(), coord4, coord4 + 2);

        mIndices.push_back(0);
        mIndices.push_back(1);
        mIndices.push_back(2);
        mIndices.push_back(0);
        mIndices.push_back(2);
        mIndices.push_back(3);

        for (size_t vertexIndex = 0; vertexIndex < 6; ++vertexIndex)
        {
            mNonIndexedVertices.insert(mNonIndexedVertices.end(),
                                       mQuadVertices.begin() + mIndices[vertexIndex] * 3,
                                       mQuadVertices.begin() + mIndices[vertexIndex] * 3 + 3);
        }

        for (size_t vertexIndex = 0; vertexIndex < 6; ++vertexIndex)
        {
            mNonIndexedVertices.insert(mNonIndexedVertices.end(),
                                       mQuadVertices.begin() + mIndices[vertexIndex] * 3,
                                       mQuadVertices.begin() + mIndices[vertexIndex] * 3 + 3);
        }

        // Tile a 2x2 grid of the tiles
        for (float y = -1.0f + quadRadius; y < 1.0f - quadRadius; y += quadRadius * 3)
        {
            for (float x = -1.0f + quadRadius; x < 1.0f - quadRadius; x += quadRadius * 3)
            {
                GLfloat instance[3] = {x + quadRadius, y + quadRadius, 0.0f};
                mInstances.insert(mInstances.end(), instance, instance + 3);
            }
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        ASSERT_GL_NO_ERROR();
    }

    GLuint setupDrawArraysTest(const std::string &vs)
    {
        const std::string fs = SHADER_SOURCE
        (
            precision mediump float;
            void main()
            {
                gl_FragColor = vec4(1.0, 0, 0, 1.0);
            }
        );

        GLuint program = CompileProgram(vs, fs);
        if (program == 0)
        {
            return 0;
        }

        // Set the viewport
        glViewport(0, 0, getWindowWidth(), getWindowHeight());

        // Clear the color buffer
        glClear(GL_COLOR_BUFFER_BIT);

        // Use the program object
        glUseProgram(program);

        return program;
    }

    void runDrawArraysTest(GLuint program, GLint first, GLsizei count, GLsizei instanceCount, float *offset)
    {
        GLuint vertexBuffer;
        glGenBuffers(1, &vertexBuffer);

        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, mInstances.size() * sizeof(mInstances[0]), &mInstances[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Get the attribute locations
        GLint positionLoc = glGetAttribLocation(program, "a_position");
        GLint instancePosLoc = glGetAttribLocation(program, "a_instancePos");

        // Load the vertex position
        glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, mNonIndexedVertices.data());
        glEnableVertexAttribArray(positionLoc);

        // Load the instance position
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glVertexAttribPointer(instancePosLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glEnableVertexAttribArray(instancePosLoc);

        // Enable instancing
        mVertexAttribDivisorANGLE(instancePosLoc, 1);

        // Offset
        GLint uniformLoc = glGetUniformLocation(program, "u_offset");
        ASSERT_NE(uniformLoc, -1);
        glUniform3fv(uniformLoc, 1, offset);

        // Do the instanced draw
        mDrawArraysInstancedANGLE(GL_TRIANGLES, first, count, instanceCount);

        ASSERT_GL_NO_ERROR();
    }

    virtual void runDrawElementsTest(std::string vs, bool shouldAttribZeroBeInstanced)
    {
        const std::string fs = SHADER_SOURCE
        (
            precision mediump float;
            void main()
            {
                gl_FragColor = vec4(1.0, 0, 0, 1.0);
            }
        );

        GLuint program = CompileProgram(vs, fs);
        ASSERT_NE(program, 0u);

        // Get the attribute locations
        GLint positionLoc = glGetAttribLocation(program, "a_position");
        GLint instancePosLoc = glGetAttribLocation(program, "a_instancePos");

        // If this ASSERT fails then the vertex shader code should be refactored
        ASSERT_EQ(shouldAttribZeroBeInstanced, (instancePosLoc == 0));

        // Set the viewport
        glViewport(0, 0, getWindowWidth(), getWindowHeight());

        // Clear the color buffer
        glClear(GL_COLOR_BUFFER_BIT);

        // Use the program object
        glUseProgram(program);

        // Load the vertex position
        glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, mQuadVertices.data());
        glEnableVertexAttribArray(positionLoc);

        // Load the instance position
        glVertexAttribPointer(instancePosLoc, 3, GL_FLOAT, GL_FALSE, 0, mInstances.data());
        glEnableVertexAttribArray(instancePosLoc);

        // Enable instancing
        mVertexAttribDivisorANGLE(instancePosLoc, 1);

        // Do the instanced draw
        mDrawElementsInstancedANGLE(GL_TRIANGLES, static_cast<GLsizei>(mIndices.size()),
                                    GL_UNSIGNED_SHORT, mIndices.data(),
                                    static_cast<GLsizei>(mInstances.size()) / 3);

        ASSERT_GL_NO_ERROR();

        checkQuads();
    }

    void checkQuads()
    {
        // Check that various pixels are the expected color.
        for (unsigned int quadIndex = 0; quadIndex < 4; ++quadIndex)
        {
            unsigned int baseOffset = quadIndex * 3;

            int quadx = static_cast<int>(((mInstances[baseOffset + 0]) * 0.5f + 0.5f) * getWindowWidth());
            int quady = static_cast<int>(((mInstances[baseOffset + 1]) * 0.5f + 0.5f) * getWindowHeight());

            EXPECT_PIXEL_EQ(quadx, quady, 255, 0, 0, 255);
        }
    }

    // Loaded entry points
    PFNGLVERTEXATTRIBDIVISORANGLEPROC mVertexAttribDivisorANGLE;
    PFNGLDRAWARRAYSINSTANCEDANGLEPROC mDrawArraysInstancedANGLE;
    PFNGLDRAWELEMENTSINSTANCEDANGLEPROC mDrawElementsInstancedANGLE;

    // Vertex data
    std::vector<GLfloat> mQuadVertices;
    std::vector<GLfloat> mNonIndexedVertices;
    std::vector<GLfloat> mTexcoords;
    std::vector<GLfloat> mInstances;
    std::vector<GLushort> mIndices;

    const GLfloat quadRadius = 0.30f;
};

class InstancingTestAllConfigs : public InstancingTest
{
  protected:
    InstancingTestAllConfigs() {}
};

class InstancingTestNo9_3 : public InstancingTest
{
  protected:
    InstancingTestNo9_3() {}
};

// This test uses a vertex shader with the first attribute (attribute zero) instanced.
// On D3D9 and D3D11 FL9_3, this triggers a special codepath that rearranges the input layout sent to D3D,
// to ensure that slot/stream zero of the input layout doesn't contain per-instance data.
TEST_P(InstancingTestAllConfigs, AttributeZeroInstanced)
{
    const std::string vs = SHADER_SOURCE
    (
        attribute vec3 a_instancePos;
        attribute vec3 a_position;
        void main()
        {
            gl_Position = vec4(a_position.xyz + a_instancePos.xyz, 1.0);
        }
    );

    runDrawElementsTest(vs, true);
}

// Same as AttributeZeroInstanced, but attribute zero is not instanced.
// This ensures the general instancing codepath (i.e. without rearranging the input layout) works as expected.
TEST_P(InstancingTestAllConfigs, AttributeZeroNotInstanced)
{
    const std::string vs = SHADER_SOURCE
    (
        attribute vec3 a_position;
        attribute vec3 a_instancePos;
        void main()
        {
            gl_Position = vec4(a_position.xyz + a_instancePos.xyz, 1.0);
        }
    );

    runDrawElementsTest(vs, false);
}

// Tests that the "first" parameter to glDrawArraysInstancedANGLE is only an offset into
// the non-instanced vertex attributes.
TEST_P(InstancingTestNo9_3, DrawArraysWithOffset)
{
    const std::string vs = SHADER_SOURCE
    (
        attribute vec3 a_position;
        attribute vec3 a_instancePos;
        uniform vec3 u_offset;
        void main()
        {
            gl_Position = vec4(a_position.xyz + a_instancePos.xyz + u_offset, 1.0);
        }
    );

    GLuint program = setupDrawArraysTest(vs);
    ASSERT_NE(program, 0u);

    float offset1[3] = { 0, 0, 0 };
    runDrawArraysTest(program, 0, 6, 2, offset1);

    float offset2[3] = { 0.0f, 1.0f, 0 };
    runDrawArraysTest(program, 6, 6, 2, offset2);

    checkQuads();

    glDeleteProgram(program);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
// We test on D3D9 and D3D11 9_3 because they use special codepaths when attribute zero is instanced, unlike D3D11.
ANGLE_INSTANTIATE_TEST(InstancingTestAllConfigs, ES2_D3D9(), ES2_D3D11(), ES2_D3D11_FL9_3());

// TODO(jmadill): Figure out the situation with DrawInstanced on FL 9_3
ANGLE_INSTANTIATE_TEST(InstancingTestNo9_3, ES2_D3D9(), ES2_D3D11());
