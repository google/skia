//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

using namespace angle;

class FenceNVTest : public ANGLETest
{
  protected:
    FenceNVTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }
};

class FenceSyncTest : public ANGLETest
{
  protected:
    FenceSyncTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }
};

// FenceNV objects should respond false to glIsFenceNV until they've been set
TEST_P(FenceNVTest, IsFence)
{
    if (!extensionEnabled("GL_NV_fence"))
    {
        std::cout << "Test skipped due to missing GL_NV_fence extension." << std::endl;
        return;
    }

    GLuint fence = 0;
    glGenFencesNV(1, &fence);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(GL_FALSE, glIsFenceNV(fence));
    EXPECT_GL_NO_ERROR();

    glSetFenceNV(fence, GL_ALL_COMPLETED_NV);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(GL_TRUE, glIsFenceNV(fence));
    EXPECT_GL_NO_ERROR();
}

// Test error cases for all FenceNV functions
TEST_P(FenceNVTest, Errors)
{
    if (!extensionEnabled("GL_NV_fence"))
    {
        std::cout << "Test skipped due to missing GL_NV_fence extension." << std::endl;
        return;
    }

    // glTestFenceNV should still return TRUE for an invalid fence and generate an INVALID_OPERATION
    EXPECT_EQ(GL_TRUE, glTestFenceNV(10));
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    GLuint fence = 20;

    // glGenFencesNV should generate INVALID_VALUE for a negative n and not write anything to the fences pointer
    glGenFencesNV(-1, &fence);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    EXPECT_EQ(20u, fence);

    // Generate a real fence
    glGenFencesNV(1, &fence);
    EXPECT_GL_NO_ERROR();

    // glTestFenceNV should still return TRUE for a fence that is not started and generate an INVALID_OPERATION
    EXPECT_EQ(GL_TRUE, glTestFenceNV(fence));
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // glGetFenceivNV should generate an INVALID_OPERATION for an invalid or unstarted fence and not modify the params
    GLint result = 30;
    glGetFenceivNV(10, GL_FENCE_STATUS_NV, &result);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    EXPECT_EQ(30, result);

    glGetFenceivNV(fence, GL_FENCE_STATUS_NV, &result);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    EXPECT_EQ(30, result);

    // glSetFenceNV should generate an error for any condition that is not ALL_COMPLETED_NV
    glSetFenceNV(fence, 0);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    // glSetFenceNV should generate INVALID_OPERATION for an invalid fence
    glSetFenceNV(10, GL_ALL_COMPLETED_NV);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test that basic usage works and doesn't generate errors or crash
TEST_P(FenceNVTest, BasicOperations)
{
    if (!extensionEnabled("GL_NV_fence"))
    {
        std::cout << "Test skipped due to missing GL_NV_fence extension." << std::endl;
        return;
    }

    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);

    GLuint fences[20] = { 0 };
    glGenFencesNV(static_cast<GLsizei>(ArraySize(fences)), fences);
    EXPECT_GL_NO_ERROR();

    for (GLuint fence : fences)
    {
        glSetFenceNV(fence, GL_ALL_COMPLETED_NV);

        glClear(GL_COLOR_BUFFER_BIT);
    }

    glFinish();

    for (GLuint fence : fences)
    {
        GLint status = 0;
        glGetFenceivNV(fence, GL_FENCE_STATUS_NV, &status);
        EXPECT_GL_NO_ERROR();

        // Fence should be complete now that Finish has been called
        EXPECT_EQ(GL_TRUE, status);
    }

    EXPECT_PIXEL_EQ(0, 0, 255, 0, 255, 255);
}

// FenceSync objects should respond true to IsSync after they are created with glFenceSync
TEST_P(FenceSyncTest, IsSync)
{
    GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(GL_TRUE, glIsSync(sync));
    EXPECT_EQ(GL_FALSE, glIsSync(reinterpret_cast<GLsync>(40)));
}

// Test error cases for all FenceSync function
TEST_P(FenceSyncTest, Errors)
{
    GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

    // DeleteSync generates INVALID_VALUE when the sync is not valid
    glDeleteSync(reinterpret_cast<GLsync>(20));
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // glFenceSync generates GL_INVALID_ENUM if the condition is not GL_SYNC_GPU_COMMANDS_COMPLETE
    EXPECT_EQ(0, glFenceSync(0, 0));
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    // glFenceSync generates GL_INVALID_ENUM if the flags is not 0
    EXPECT_EQ(0, glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 10));
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // glClientWaitSync generates GL_INVALID_VALUE and returns GL_WAIT_FAILED if flags contains more than just GL_SYNC_FLUSH_COMMANDS_BIT
    EXPECT_GLENUM_EQ(GL_WAIT_FAILED, glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT | 0x2, 0));
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // glClientWaitSync generates GL_INVALID_VALUE and returns GL_WAIT_FAILED if the sync object is not valid
    EXPECT_GLENUM_EQ(GL_WAIT_FAILED, glClientWaitSync(reinterpret_cast<GLsync>(30), GL_SYNC_FLUSH_COMMANDS_BIT, 0));
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // glWaitSync generates GL_INVALID_VALUE if flags is non-zero
    glWaitSync(sync, 1, GL_TIMEOUT_IGNORED);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // glWaitSync generates GL_INVALID_VALUE if GLuint64 is not GL_TIMEOUT_IGNORED
    glWaitSync(sync, 0, 0);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // glWaitSync generates GL_INVALID_VALUE if the sync object is not valid
    glWaitSync(reinterpret_cast<GLsync>(30), 0, GL_TIMEOUT_IGNORED);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // glGetSynciv generates GL_INVALID_VALUE if bufSize is less than zero, results should be untouched
    GLsizei length = 20;
    GLint value = 30;
    glGetSynciv(sync, GL_OBJECT_TYPE, -1, &length, &value);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    EXPECT_EQ(20, length);
    EXPECT_EQ(30, value);

    // glGetSynciv generates GL_INVALID_VALUE if the sync object tis not valid, results should be untouched
    glGetSynciv(reinterpret_cast<GLsync>(30), GL_OBJECT_TYPE, 1, &length, &value);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    EXPECT_EQ(20, length);
    EXPECT_EQ(30, value);
}

// Test usage of glGetSynciv
TEST_P(FenceSyncTest, BasicQueries)
{
    GLsizei length = 0;
    GLint value = 0;
    GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

    glGetSynciv(sync, GL_SYNC_CONDITION, 1, &length, &value);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(GL_SYNC_GPU_COMMANDS_COMPLETE, value);

    glGetSynciv(sync, GL_OBJECT_TYPE, 1, &length, &value);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(GL_SYNC_FENCE, value);

    glGetSynciv(sync, GL_SYNC_FLAGS, 1, &length, &value);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(0, value);
}

// Test that basic usage works and doesn't generate errors or crash
TEST_P(FenceSyncTest, BasicOperations)
{
    // TODO(geofflang): Figure out why this is broken on Intel OpenGL
    if (isIntel() && getPlatformRenderer() == EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE)
    {
        std::cout << "Test skipped on Intel OpenGL." << std::endl;
        return;
    }

    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);

    GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

    glClear(GL_COLOR_BUFFER_BIT);
    glWaitSync(sync, 0, GL_TIMEOUT_IGNORED);
    EXPECT_GL_NO_ERROR();

    GLsizei length = 0;
    GLint value = 0;
    unsigned int loopCount = 0;

    glFlush();

    // Use 'loopCount' to make sure the test doesn't get stuck in an infinite loop
    while (value != GL_SIGNALED && loopCount <= 1000000)
    {
        loopCount++;

        glGetSynciv(sync, GL_SYNC_STATUS, 1, &length, &value);
        ASSERT_GL_NO_ERROR();
    }

    ASSERT_GLENUM_EQ(GL_SIGNALED, value);

    for (size_t i = 0; i < 20; i++)
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
        EXPECT_GL_NO_ERROR();
    }
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
ANGLE_INSTANTIATE_TEST(FenceNVTest, ES2_D3D9(), ES2_D3D11(), ES3_D3D11(), ES2_OPENGL(), ES3_OPENGL());
ANGLE_INSTANTIATE_TEST(FenceSyncTest, ES3_D3D11(), ES3_OPENGL());
