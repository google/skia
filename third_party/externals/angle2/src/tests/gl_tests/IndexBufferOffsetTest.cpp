//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// IndexBufferOffsetTest.cpp: Test glDrawElements with an offset and an index buffer

#include "test_utils/ANGLETest.h"
#include "system_utils.h"

using namespace angle;

class IndexBufferOffsetTest : public ANGLETest
{
  protected:
    IndexBufferOffsetTest()
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
            SHADER_SOURCE(precision highp float; attribute vec2 position;

                          void main()
                          {
                              gl_Position = vec4(position, 0.0, 1.0);
                          });

        const std::string fragmentShaderSource =
            SHADER_SOURCE(precision highp float; uniform vec4 color;

                          void main()
                          {
                              gl_FragColor = color;
                          });

        mProgram = CompileProgram(vertexShaderSource, fragmentShaderSource);
        ASSERT_NE(0u, mProgram);

        mColorUniformLocation      = glGetUniformLocation(mProgram, "color");
        mPositionAttributeLocation = glGetAttribLocation(mProgram, "position");

        const GLfloat vertices[] =
        {
            -1.0f, -1.0f,
            -1.0f,  1.0f,
             1.0f, -1.0f,
             1.0f,  1.0f
        };
        glGenBuffers(1, &mVertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);

        glGenBuffers(1, &mIndexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
    }

    void TearDown() override
    {
        glDeleteBuffers(1, &mVertexBuffer);
        glDeleteBuffers(1, &mIndexBuffer);
        glDeleteProgram(mProgram);
        ANGLETest::TearDown();
    }

    void runTest(GLenum type, int typeWidth, void *indexData)
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        GLuint nullIndexData[] = {0, 0, 0, 0, 0, 0};

        size_t indexDataWidth = 6 * typeWidth;

        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * indexDataWidth, nullptr, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indexDataWidth, nullIndexData);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, indexDataWidth, indexDataWidth, indexData);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 2 * indexDataWidth, indexDataWidth, nullIndexData);

        glUseProgram(mProgram);

        glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
        glVertexAttribPointer(mPositionAttributeLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(mPositionAttributeLocation);

        glUniform4f(mColorUniformLocation, 1.0f, 0.0f, 0.0f, 1.0f);

        for (int i = 0; i < 16; i++)
        {
            glDrawElements(GL_TRIANGLES, 6, type, reinterpret_cast<GLvoid *>(indexDataWidth));
            EXPECT_PIXEL_EQ(64, 64, 255, 0, 0, 255);
        }

        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, indexDataWidth, indexDataWidth, nullIndexData);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 2 * indexDataWidth, indexDataWidth, indexData);

        glUniform4f(mColorUniformLocation, 0.0f, 1.0f, 0.0f, 1.0f);
        glDrawElements(GL_TRIANGLES, 6, type, reinterpret_cast<GLvoid *>(indexDataWidth * 2));
        EXPECT_PIXEL_EQ(64, 64, 0, 255, 0, 255);

        EXPECT_GL_NO_ERROR();
        swapBuffers();
    }

    GLuint mProgram;
    GLint mColorUniformLocation;
    GLint mPositionAttributeLocation;
    GLuint mVertexBuffer;
    GLuint mIndexBuffer;
};

// Test using an offset for an UInt8 index buffer
TEST_P(IndexBufferOffsetTest, UInt8Index)
{
    GLubyte indexData[] = {0, 1, 2, 1, 2, 3};
    runTest(GL_UNSIGNED_BYTE, 1, indexData);
}

// Test using an offset for an UInt16 index buffer
TEST_P(IndexBufferOffsetTest, UInt16Index)
{
    GLushort indexData[] = {0, 1, 2, 1, 2, 3};
    runTest(GL_UNSIGNED_SHORT, 2, indexData);
}

// Test using an offset for an UInt32 index buffer
TEST_P(IndexBufferOffsetTest, UInt32Index)
{
    if (getClientVersion() < 3 && !extensionEnabled("GL_OES_element_index_uint"))
    {
        std::cout << "Test skipped because ES3 or GL_OES_element_index_uint is not available." << std::endl;
        return;
    }

    GLuint indexData[] = {0, 1, 2, 1, 2, 3};
    runTest(GL_UNSIGNED_INT, 4, indexData);
}

ANGLE_INSTANTIATE_TEST(IndexBufferOffsetTest,
                       ES2_D3D9(),
                       ES2_D3D11(),
                       ES3_D3D11(),
                       ES2_OPENGL(),
                       ES3_OPENGL());

