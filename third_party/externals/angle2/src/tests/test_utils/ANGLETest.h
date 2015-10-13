//
// Copyright (c) 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef ANGLE_TESTS_ANGLE_TEST_H_
#define ANGLE_TESTS_ANGLE_TEST_H_

#include <gtest/gtest.h>
#include <algorithm>

#include "angle_gl.h"
#include "angle_test_configs.h"
#include "common/angleutils.h"
#include "shader_utils.h"

#define EXPECT_GL_ERROR(err) EXPECT_EQ(static_cast<GLenum>(err), glGetError())
#define EXPECT_GL_NO_ERROR() EXPECT_GL_ERROR(GL_NO_ERROR)

#define ASSERT_GL_ERROR(err) ASSERT_EQ(static_cast<GLenum>(err), glGetError())
#define ASSERT_GL_NO_ERROR() ASSERT_GL_ERROR(GL_NO_ERROR)

#define EXPECT_EGL_ERROR(err) EXPECT_EQ((err), eglGetError())
#define EXPECT_EGL_SUCCESS() EXPECT_EGL_ERROR(EGL_SUCCESS)

#define ASSERT_EGL_ERROR(err) ASSERT_EQ((err), eglGetError())
#define ASSERT_EGL_SUCCESS() ASSERT_EGL_ERROR(EGL_SUCCESS)

#define ASSERT_GLENUM_EQ(expected, actual) ASSERT_EQ(static_cast<GLenum>(expected), static_cast<GLenum>(actual))
#define EXPECT_GLENUM_EQ(expected, actual) EXPECT_EQ(static_cast<GLenum>(expected), static_cast<GLenum>(actual))

#define EXPECT_PIXEL_EQ(x, y, r, g, b, a) \
{ \
    GLubyte pixel[4]; \
    glReadPixels((x), (y), 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel); \
    EXPECT_GL_NO_ERROR(); \
    EXPECT_EQ((r), pixel[0]); \
    EXPECT_EQ((g), pixel[1]); \
    EXPECT_EQ((b), pixel[2]); \
    EXPECT_EQ((a), pixel[3]); \
}

#define EXPECT_PIXEL_NEAR(x, y, r, g, b, a, abs_error) \
{ \
    GLubyte pixel[4]; \
    glReadPixels((x), (y), 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel); \
    EXPECT_GL_NO_ERROR(); \
    EXPECT_NEAR((r), pixel[0], abs_error); \
    EXPECT_NEAR((g), pixel[1], abs_error); \
    EXPECT_NEAR((b), pixel[2], abs_error); \
    EXPECT_NEAR((a), pixel[3], abs_error); \
}

class EGLWindow;
class OSWindow;

class ANGLETest : public ::testing::TestWithParam<angle::PlatformParameters>
{
  protected:
    ANGLETest();
    ~ANGLETest();

  public:
    static bool InitTestWindow();
    static bool DestroyTestWindow();
    static void SetWindowVisible(bool isVisible);
    static bool eglDisplayExtensionEnabled(EGLDisplay display, const std::string &extName);

  protected:
    virtual void SetUp();
    virtual void TearDown();

    virtual void swapBuffers();

    static void drawQuad(GLuint program, const std::string& positionAttribName, GLfloat quadDepth, GLfloat quadScale = 1.0f);
    static GLuint compileShader(GLenum type, const std::string &source);
    static bool extensionEnabled(const std::string &extName);
    static bool eglClientExtensionEnabled(const std::string &extName);

    void setWindowWidth(int width);
    void setWindowHeight(int height);
    void setConfigRedBits(int bits);
    void setConfigGreenBits(int bits);
    void setConfigBlueBits(int bits);
    void setConfigAlphaBits(int bits);
    void setConfigDepthBits(int bits);
    void setConfigStencilBits(int bits);
    void setMultisampleEnabled(bool enabled);

    int getClientVersion() const;

    EGLWindow *getEGLWindow() const;
    int getWindowWidth() const;
    int getWindowHeight() const;
    bool isMultisampleEnabled() const;

    bool isIntel() const;
    bool isAMD() const;
    bool isNVidia() const;
    EGLint getPlatformRenderer() const;

  private:
    bool createEGLContext();
    bool destroyEGLContext();

    EGLWindow *mEGLWindow;
    int mWidth;
    int mHeight;

    static OSWindow *mOSWindow;
};

class ANGLETestEnvironment : public testing::Environment
{
  public:
    virtual void SetUp();
    virtual void TearDown();
};

#endif  // ANGLE_TESTS_ANGLE_TEST_H_
