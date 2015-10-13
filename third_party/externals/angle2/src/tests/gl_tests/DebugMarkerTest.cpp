//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DebugMarkerTest:
//   Basic tests to ensure EXT_debug_marker entry points work.

#include "test_utils/ANGLETest.h"

using namespace angle;

namespace
{

class DebugMarkerTest : public ANGLETest
{
  protected:
    DebugMarkerTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
};

// Simple test to ensure the various EXT_debug_marker entry points don't crash.
// The debug markers can be validated by capturing this test under PIX/Graphics Diagnostics.
TEST_P(DebugMarkerTest, BasicValidation)
{
    if (!extensionEnabled("GL_EXT_debug_marker"))
    {
        std::cout << "Test skipped due to missing GL_EXT_debug_marker" << std::endl;
        return;
    }

    std::string eventMarkerCaption = "Test event marker caption";
    std::string groupMarkerCaption = "Test group marker caption";

    glPushGroupMarkerEXT(static_cast<GLsizei>(groupMarkerCaption.length()),
                         groupMarkerCaption.c_str());

    // Do some basic operations between calls to extension entry points
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glInsertEventMarkerEXT(static_cast<GLsizei>(eventMarkerCaption.length()),
                           eventMarkerCaption.c_str());
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glPushGroupMarkerEXT(0, NULL);
    glClearColor(0.0f, 1.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glPopGroupMarkerEXT();
    glClearColor(0.0f, 0.0f, 1.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glPopGroupMarkerEXT();

    ASSERT_GL_NO_ERROR();
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
ANGLE_INSTANTIATE_TEST(DebugMarkerTest, ES2_D3D9(), ES2_D3D11(), ES2_OPENGL());

} // namespace
