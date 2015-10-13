//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// BuiltInFunctionEmulator_test.cpp:
//   Tests for writing the code for built-in function emulation.
//

#include "angle_gl.h"
#include "gtest/gtest.h"
#include "GLSLANG/ShaderLang.h"
#include "tests/test_utils/compiler_test.h"

namespace
{

// Test for the SH_EMULATE_BUILT_IN_FUNCTIONS flag.
class EmulateBuiltInFunctionsTest : public testing::Test
{
  public:
    EmulateBuiltInFunctionsTest() {}

  protected:
    void compile(const std::string &shaderString)
    {
        std::string infoLog;
        bool compilationSuccess = compileTestShader(GL_VERTEX_SHADER,
                                                    SH_GLES2_SPEC,
                                                    SH_GLSL_COMPATIBILITY_OUTPUT,
                                                    shaderString,
                                                    SH_EMULATE_BUILT_IN_FUNCTIONS,
                                                    &mGLSLCode,
                                                    &infoLog);
        if (!compilationSuccess)
        {
            FAIL() << "Shader compilation into GLSL failed " << infoLog;
        }
    }

    bool foundInCode(const char *stringToFind)
    {
        return mGLSLCode.find(stringToFind) != std::string::npos;
    }

  private:
    std::string mGLSLCode;
};

TEST_F(EmulateBuiltInFunctionsTest, DotEmulated)
{
    const std::string shaderString =
        "precision mediump float;\n"
        "uniform float u;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(dot(u, 1.0), 1.0, 1.0, 1.0);\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("webgl_dot_emu("));
}

} // namespace
