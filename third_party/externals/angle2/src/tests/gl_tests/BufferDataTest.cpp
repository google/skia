//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

#include <stdint.h>

using namespace angle;

class BufferDataTest : public ANGLETest
{
  protected:
    BufferDataTest()
    {
        setWindowWidth(16);
        setWindowHeight(16);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);

        mBuffer = 0;
        mProgram = 0;
        mAttribLocation = -1;
    }

    void SetUp() override
    {
        ANGLETest::SetUp();

        const char * vsSource = SHADER_SOURCE
        (
            attribute vec4 position;
            attribute float in_attrib;
            varying float v_attrib;
            void main()
            {
                v_attrib = in_attrib;
                gl_Position = position;
            }
        );

        const char * fsSource = SHADER_SOURCE
        (
            precision mediump float;
            varying float v_attrib;
            void main()
            {
                gl_FragColor = vec4(v_attrib, 0, 0, 1);
            }
        );

        glGenBuffers(1, &mBuffer);
        ASSERT_NE(mBuffer, 0U);

        mProgram = CompileProgram(vsSource, fsSource);
        ASSERT_NE(mProgram, 0U);

        mAttribLocation = glGetAttribLocation(mProgram, "in_attrib");
        ASSERT_NE(mAttribLocation, -1);

        glClearColor(0, 0, 0, 0);
        glClearDepthf(0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDisable(GL_DEPTH_TEST);

        ASSERT_GL_NO_ERROR();
    }

    void TearDown() override
    {
        glDeleteBuffers(1, &mBuffer);
        glDeleteProgram(mProgram);

        ANGLETest::TearDown();
    }

    GLuint mBuffer;
    GLuint mProgram;
    GLint mAttribLocation;
};

TEST_P(BufferDataTest, NULLData)
{
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    EXPECT_GL_NO_ERROR();

    const int numIterations = 128;
    for (int i = 0; i < numIterations; ++i)
    {
        GLsizei bufferSize = sizeof(GLfloat) * (i + 1);
        glBufferData(GL_ARRAY_BUFFER, bufferSize, NULL, GL_STATIC_DRAW);
        EXPECT_GL_NO_ERROR();

        for (int j = 0; j < bufferSize; j++)
        {
            for (int k = 0; k < bufferSize - j; k++)
            {
                glBufferSubData(GL_ARRAY_BUFFER, k, j, NULL);
                EXPECT_GL_NO_ERROR();
            }
        }
    }
}

TEST_P(BufferDataTest, ZeroNonNULLData)
{
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    EXPECT_GL_NO_ERROR();

    char *zeroData = new char[0];
    glBufferData(GL_ARRAY_BUFFER, 0, zeroData, GL_STATIC_DRAW);
    EXPECT_GL_NO_ERROR();

    glBufferSubData(GL_ARRAY_BUFFER, 0, 0, zeroData);
    EXPECT_GL_NO_ERROR();

    delete [] zeroData;
}

TEST_P(BufferDataTest, NULLResolvedData)
{
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, 128, NULL, GL_DYNAMIC_DRAW);

    glUseProgram(mProgram);
    glVertexAttribPointer(mAttribLocation, 1, GL_FLOAT, GL_FALSE, 4, NULL);
    glEnableVertexAttribArray(mAttribLocation);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    drawQuad(mProgram, "position", 0.5f);
}

// Tests that a huge allocation returns GL_OUT_OF_MEMORY
// TODO(jmadill): Figure out how to test this reliably on the Chromium bots
TEST_P(BufferDataTest, DISABLED_HugeSetDataShouldNotCrash)
{
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    EXPECT_GL_NO_ERROR();

    GLsizei allocSize = std::numeric_limits<GLsizei>::max() >> 2;

    uint8_t *data = NULL;
    while (data == NULL && allocSize >= 4)
    {
        data = new (std::nothrow) uint8_t[allocSize];

        if (data == NULL)
        {
            allocSize >>= 1;
        }
    }

    ASSERT_NE(static_cast<uint8_t*>(NULL), data);
    memset(data, 0, allocSize);

    float * fValue = reinterpret_cast<float*>(data);
    for (unsigned int f = 0; f < 6; f++)
    {
        fValue[f] = 1.0f;
    }

    glBufferData(GL_ARRAY_BUFFER, allocSize, data, GL_STATIC_DRAW);

    GLenum error = glGetError();
    if (error == GL_NO_ERROR)
    {
        // If we didn't fail because of an out of memory error, try drawing a quad
        // using the large buffer

        // DISABLED because it takes a long time, but left for posterity

        //glUseProgram(mProgram);
        //glVertexAttribPointer(mAttribLocation, 1, GL_FLOAT, GL_FALSE, 4, NULL);
        //glEnableVertexAttribArray(mAttribLocation);
        //glBindBuffer(GL_ARRAY_BUFFER, 0);
        //drawQuad(mProgram, "position", 0.5f);
        //swapBuffers();

        //// Draw operations can also generate out-of-memory, which is in-spec
        //error = glGetError();
        //if (error == GL_NO_ERROR)
        //{
        //    GLint viewportSize[4];
        //    glGetIntegerv(GL_VIEWPORT, viewportSize);

        //    GLint midPixelX = (viewportSize[0] + viewportSize[2]) / 2;
        //    GLint midPixelY = (viewportSize[1] + viewportSize[3]) / 2;

        //    EXPECT_PIXEL_EQ(midPixelX, midPixelY, 255, 0, 0, 255);
        //}
        //else
        //{
        //    EXPECT_EQ(GL_OUT_OF_MEMORY, error);
        //}
    }
    else
    {
        EXPECT_GLENUM_EQ(GL_OUT_OF_MEMORY, error);
    }

    delete[] data;
}

class IndexedBufferCopyTest : public ANGLETest
{
  protected:
    IndexedBufferCopyTest()
    {
        setWindowWidth(16);
        setWindowHeight(16);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }

    void SetUp() override
    {
        ANGLETest::SetUp();

        const char * vsSource = SHADER_SOURCE
        (
            attribute vec3 in_attrib;
            varying vec3 v_attrib;
            void main()
            {
                v_attrib = in_attrib;
                gl_Position = vec4(0.0, 0.0, 0.5, 1.0);
                gl_PointSize = 100.0;
            }
        );

        const char * fsSource = SHADER_SOURCE
        (
            precision mediump float;
            varying vec3 v_attrib;
            void main()
            {
                gl_FragColor = vec4(v_attrib, 1);
            }
        );

        glGenBuffers(2, mBuffers);
        ASSERT_NE(mBuffers[0], 0U);
        ASSERT_NE(mBuffers[1], 0U);

        glGenBuffers(1, &mElementBuffer);
        ASSERT_NE(mElementBuffer, 0U);

        mProgram = CompileProgram(vsSource, fsSource);
        ASSERT_NE(mProgram, 0U);

        mAttribLocation = glGetAttribLocation(mProgram, "in_attrib");
        ASSERT_NE(mAttribLocation, -1);

        glClearColor(0, 0, 0, 0);
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT);

        ASSERT_GL_NO_ERROR();
    }

    void TearDown() override
    {
        glDeleteBuffers(2, mBuffers);
        glDeleteBuffers(1, &mElementBuffer);
        glDeleteProgram(mProgram);

        ANGLETest::TearDown();
    }

    GLuint mBuffers[2];
    GLuint mElementBuffer;
    GLuint mProgram;
    GLint mAttribLocation;
};

// The following test covers an ANGLE bug where our index ranges
// weren't updated from CopyBufferSubData calls
// https://code.google.com/p/angleproject/issues/detail?id=709
TEST_P(IndexedBufferCopyTest, IndexRangeBug)
{
    unsigned char vertexData[] = { 255, 0, 0, 0, 0, 0 };
    unsigned int indexData[] = { 0, 1 };

    glBindBuffer(GL_ARRAY_BUFFER, mBuffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(char) * 6, vertexData, GL_STATIC_DRAW);

    glUseProgram(mProgram);
    glVertexAttribPointer(mAttribLocation, 3, GL_UNSIGNED_BYTE, GL_TRUE, 3, NULL);
    glEnableVertexAttribArray(mAttribLocation);

    ASSERT_GL_NO_ERROR();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * 1, indexData, GL_STATIC_DRAW);

    glUseProgram(mProgram);

    ASSERT_GL_NO_ERROR();

    glDrawElements(GL_POINTS, 1, GL_UNSIGNED_INT, NULL);

    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(0, 0, 255, 0, 0, 255);

    glBindBuffer(GL_COPY_READ_BUFFER, mBuffers[1]);
    glBufferData(GL_COPY_READ_BUFFER, 4, &indexData[1], GL_STATIC_DRAW);

    glBindBuffer(GL_COPY_WRITE_BUFFER, mElementBuffer);

    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, sizeof(int));

    ASSERT_GL_NO_ERROR();

    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_EQ(0, 0, 0, 0, 0, 0);

    unsigned char newData[] = { 0, 255, 0 };
    glBufferSubData(GL_ARRAY_BUFFER, 3, 3, newData);

    glDrawElements(GL_POINTS, 1, GL_UNSIGNED_INT, NULL);

    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(0, 0, 0, 255, 0, 255);
}

class BufferDataTestES3 : public BufferDataTest
{
};

// The following test covers an ANGLE bug where the buffer storage
// is not resized by Buffer11::getLatestBufferStorage when needed.
// https://code.google.com/p/angleproject/issues/detail?id=897
TEST_P(BufferDataTestES3, BufferResizing)
{
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    ASSERT_GL_NO_ERROR();

    // Allocate a buffer with one byte
    uint8_t singleByte[] = { 0xaa };
    glBufferData(GL_ARRAY_BUFFER, 1, singleByte, GL_STATIC_DRAW);

    // Resize the buffer
    // To trigger the bug, the buffer need to be big enough because some hardware copy buffers
    // by chunks of pages instead of the minimum number of bytes neeeded.
    const size_t numBytes = 4096*4;
    glBufferData(GL_ARRAY_BUFFER, numBytes, NULL, GL_STATIC_DRAW);

    // Copy the original data to the buffer
    uint8_t srcBytes[numBytes];
    for (size_t i = 0; i < numBytes; ++i)
    {
        srcBytes[i] = static_cast<uint8_t>(i);
    }

    void *dest = glMapBufferRange(GL_ARRAY_BUFFER, 0, numBytes, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

    ASSERT_GL_NO_ERROR();

    memcpy(dest, srcBytes, numBytes);
    glUnmapBuffer(GL_ARRAY_BUFFER);

    EXPECT_GL_NO_ERROR();

    // Create a new buffer and copy the data to it
    GLuint readBuffer;
    glGenBuffers(1, &readBuffer);
    glBindBuffer(GL_COPY_WRITE_BUFFER, readBuffer);
    uint8_t zeros[numBytes];
    for (size_t i = 0; i < numBytes; ++i)
    {
        zeros[i] = 0;
    }
    glBufferData(GL_COPY_WRITE_BUFFER, numBytes, zeros, GL_STATIC_DRAW);
    glCopyBufferSubData(GL_ARRAY_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, numBytes);

    ASSERT_GL_NO_ERROR();

    // Read back the data and compare it to the original
    uint8_t *data = reinterpret_cast<uint8_t*>(glMapBufferRange(GL_COPY_WRITE_BUFFER, 0, numBytes, GL_MAP_READ_BIT));

    ASSERT_GL_NO_ERROR();

    for (size_t i = 0; i < numBytes; ++i)
    {
        EXPECT_EQ(srcBytes[i], data[i]);
    }
    glUnmapBuffer(GL_COPY_WRITE_BUFFER);

    glDeleteBuffers(1, &readBuffer);

    EXPECT_GL_NO_ERROR();
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
ANGLE_INSTANTIATE_TEST(BufferDataTest, ES2_D3D9(), ES2_D3D11(), ES2_OPENGL());
ANGLE_INSTANTIATE_TEST(BufferDataTestES3, ES3_D3D11());
ANGLE_INSTANTIATE_TEST(IndexedBufferCopyTest, ES3_D3D11());

#ifdef _WIN64

// Test a bug where an integer overflow bug could trigger a crash in D3D.
// The test uses 8 buffers with a size just under 0x2000000 to overflow max uint
// (with the internal D3D rounding to 16-byte values) and trigger the bug.
// Only handle this bug on 64-bit Windows for now. Harder to repro on 32-bit.
class BufferDataOverflowTest : public ANGLETest
{
  protected:
    BufferDataOverflowTest()
        : mProgram(0)
    {
    }

    ~BufferDataOverflowTest()
    {
        if (!mBuffers.empty())
        {
            glDeleteBuffers(static_cast<GLsizei>(mBuffers.size()), &mBuffers[0]);
        }
        if (mProgram != 0u)
        {
            glDeleteProgram(mProgram);
        }
    }

    std::vector<GLuint> mBuffers;
    GLuint mProgram;
};

// See description above.
TEST_P(BufferDataOverflowTest, VertexBufferIntegerOverflow)
{
    // These values are special, to trigger the rounding bug.
    unsigned int numItems = 0x7FFFFFE;
    GLsizei bufferCnt = 8;

    mBuffers.resize(bufferCnt);

    std::stringstream vertexShaderStr;

    for (GLsizei bufferIndex = 0; bufferIndex < bufferCnt; ++bufferIndex)
    {
        vertexShaderStr << "attribute float attrib" << bufferIndex << ";\n";
    }

    vertexShaderStr << "attribute vec2 position;\n"
                       "varying float v_attrib;\n"
                       "void main() {\n"
                       "  gl_Position = vec4(position, 0, 1);\n"
                       "  v_attrib = 0.0;\n";

    for (GLsizei bufferIndex = 0; bufferIndex < bufferCnt; ++bufferIndex)
    {
        vertexShaderStr << "v_attrib += attrib" << bufferIndex << ";\n";
    }

    vertexShaderStr << "}";

    const std::string &fragmentShader =
        "varying highp float v_attrib;\n"
        "void main() {\n"
        "  gl_FragColor = vec4(v_attrib, 0, 0, 1);\n"
        "}";

    mProgram = CompileProgram(vertexShaderStr.str(), fragmentShader);
    ASSERT_NE(0u, mProgram);
    glUseProgram(mProgram);

    glGenBuffers(bufferCnt, &mBuffers[0]);

    std::vector<GLfloat> data(numItems, 1.0f);

    for (GLsizei bufferIndex = 0; bufferIndex < bufferCnt; ++bufferIndex)
    {
        glBindBuffer(GL_ARRAY_BUFFER, mBuffers[bufferIndex]);
        glBufferData(GL_ARRAY_BUFFER, numItems * sizeof(float), &data[0], GL_DYNAMIC_DRAW);

        std::stringstream attribNameStr;
        attribNameStr << "attrib" << bufferIndex;

        GLint attribLocation = glGetAttribLocation(mProgram, attribNameStr.str().c_str());
        ASSERT_NE(-1, attribLocation);

        glVertexAttribPointer(attribLocation, 1, GL_FLOAT, GL_FALSE, 4, nullptr);
        glEnableVertexAttribArray(attribLocation);
    }

    GLint positionLocation = glGetAttribLocation(mProgram, "position");
    ASSERT_NE(-1, positionLocation);
    glDisableVertexAttribArray(positionLocation);
    glVertexAttrib2f(positionLocation, 1.0f, 1.0f);

    EXPECT_GL_NO_ERROR();
    glDrawArrays(GL_TRIANGLES, 0, numItems);
    EXPECT_GL_ERROR(GL_OUT_OF_MEMORY);
}

ANGLE_INSTANTIATE_TEST(BufferDataOverflowTest, ES3_D3D11());

#endif  // _WIN64
