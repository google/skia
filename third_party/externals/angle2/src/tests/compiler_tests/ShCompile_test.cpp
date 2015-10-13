//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ShCompile_test.cpp
//   Test the ShCompile interface with different parameters.
//

#include "angle_gl.h"
#include "gtest/gtest.h"
#include "GLSLANG/ShaderLang.h"

class ShCompileTest : public testing::Test
{
  public:
    ShCompileTest() {}

  protected:
    void SetUp() override
    {
        ShInitBuiltInResources(&mResources);
        mCompiler =
            ShConstructCompiler(GL_FRAGMENT_SHADER, SH_WEBGL_SPEC, SH_GLSL_OUTPUT, &mResources);
        ASSERT_TRUE(mCompiler != nullptr) << "Compiler could not be constructed.";
    }

    void TearDown() override
    {
        if (mCompiler)
        {
            ShDestruct(mCompiler);
            mCompiler = nullptr;
        }
    }

    void testCompile(const char **shaderStrings, int stringCount, bool expectation)
    {
        bool success                  = ShCompile(mCompiler, shaderStrings, stringCount, 0);
        const std::string &compileLog = ShGetInfoLog(mCompiler);
        EXPECT_EQ(expectation, success) << compileLog;
    }

  private:
    ShBuiltInResources mResources;
    ShHandle mCompiler;
};

// Test calling ShCompile with more than one shader source string.
TEST_F(ShCompileTest, MultipleShaderStrings)
{
    const std::string &shaderString1 =
        "precision mediump float;\n"
        "void main() {\n";
    const std::string &shaderString2 =
        "    gl_FragColor = vec4(0.0);\n"
        "}";

    const char *shaderStrings[] = {shaderString1.c_str(), shaderString2.c_str()};

    testCompile(shaderStrings, 2, true);
}

// Test calling ShCompile with a tokens split into different shader source strings.
TEST_F(ShCompileTest, TokensSplitInShaderStrings)
{
    const std::string &shaderString1 =
        "precision mediump float;\n"
        "void ma";
    const std::string &shaderString2 =
        "in() {\n"
        "#i";
    const std::string &shaderString3 =
        "f 1\n"
        "    gl_FragColor = vec4(0.0);\n"
        "#endif\n"
        "}";

    const char *shaderStrings[] = {shaderString1.c_str(), shaderString2.c_str(),
                                   shaderString3.c_str()};

    testCompile(shaderStrings, 3, true);
}
