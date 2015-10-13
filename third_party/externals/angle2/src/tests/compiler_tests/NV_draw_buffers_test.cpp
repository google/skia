//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// NV_draw_buffers_test.cpp:
//   Test for NV_draw_buffers setting
//

#include "angle_gl.h"
#include "gtest/gtest.h"
#include "GLSLANG/ShaderLang.h"
#include "tests/test_utils/compiler_test.h"

class NVDrawBuffersTest : public testing::Test
{
  public:
    NVDrawBuffersTest() {}

  protected:
    void compile(const std::string &shaderString)
    {
        ShBuiltInResources resources;
        ShInitBuiltInResources(&resources);
        resources.MaxDrawBuffers = 8;
        resources.EXT_draw_buffers = 1;
        resources.NV_draw_buffers = 1;

        std::string infoLog;
        bool compilationSuccess = compileTestShader(GL_FRAGMENT_SHADER, SH_GLES2_SPEC, SH_ESSL_OUTPUT,
                                                    shaderString, &resources, &mGLSLCode, &infoLog);
        if (!compilationSuccess)
        {
            FAIL() << "Shader compilation into ESSL failed " << infoLog;
        }
    }

    bool foundInCode(const char *stringToFind)
    {
        return mGLSLCode.find(stringToFind) != std::string::npos;
    }

  private:
    std::string mGLSLCode;
};

TEST_F(NVDrawBuffersTest, NVDrawBuffers)
{
    const std::string &shaderString =
        "#extension GL_EXT_draw_buffers : require\n"
        "precision mediump float;\n"
        "void main() {\n"
        "   gl_FragData[0] = vec4(1.0);\n"
        "   gl_FragData[1] = vec4(0.0);\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("GL_NV_draw_buffers"));
    ASSERT_FALSE(foundInCode("GL_EXT_draw_buffers"));
}
