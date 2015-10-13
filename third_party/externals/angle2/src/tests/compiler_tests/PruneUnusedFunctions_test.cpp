//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// PruneUnusedFunctions_test.cpp:
//   Test for the pruning of unused function with the SH_PRUNE_UNUSED_FUNCTIONS compile flag
//

#include "angle_gl.h"
#include "gtest/gtest.h"
#include "GLSLANG/ShaderLang.h"
#include "tests/test_utils/compiler_test.h"

namespace
{

class PruneUnusedFunctionsTest : public testing::Test
{
  public:
    PruneUnusedFunctionsTest() {}

  protected:
    void compile(const std::string &shaderString, bool prune)
    {
        int compilationFlags = SH_VARIABLES | (prune ? 0 : SH_DONT_PRUNE_UNUSED_FUNCTIONS);

        std::string infoLog;
        bool compilationSuccess = compileTestShader(GL_FRAGMENT_SHADER, SH_GLES3_SPEC, SH_ESSL_OUTPUT,
                                                    shaderString, compilationFlags, &mTranslatedSource, &infoLog);
        if (!compilationSuccess)
        {
            FAIL() << "Shader compilation failed " << infoLog;
        }
    }

    bool kept(const char *functionName, int nOccurences) const
    {
        size_t currentPos = 0;
        while (nOccurences-- > 0)
        {
            auto position = mTranslatedSource.find(functionName, currentPos);
            if (position == std::string::npos)
            {
                return false;
            }
            currentPos = position + 1;
        }
        return mTranslatedSource.find(functionName, currentPos) == std::string::npos;
    }

    bool removed(const char* functionName) const
    {
        return mTranslatedSource.find(functionName) == std::string::npos;
    }

  private:
    std::string mTranslatedSource;
};

// Check that unused function and prototypes are removed iff the options is set
TEST_F(PruneUnusedFunctionsTest, UnusedFunctionAndProto)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "float unused(float a);\n"
        "void main() {\n"
        "    gl_FragColor = vec4(1.0);\n"
        "}\n"
        "float unused(float a) {\n"
        "    return a;\n"
        "}\n";
    compile(shaderString, true);
    EXPECT_TRUE(removed("unused("));
    EXPECT_TRUE(kept("main(", 1));

    compile(shaderString, false);
    EXPECT_TRUE(kept("unused(", 2));
    EXPECT_TRUE(kept("main(", 1));
}

// Check that unimplemented prototypes are removed iff the options is set
TEST_F(PruneUnusedFunctionsTest, UnimplementedPrototype)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "float unused(float a);\n"
        "void main() {\n"
        "    gl_FragColor = vec4(1.0);\n"
        "}\n";
    compile(shaderString, true);
    EXPECT_TRUE(removed("unused("));
    EXPECT_TRUE(kept("main(", 1));

    compile(shaderString, false);
    EXPECT_TRUE(kept("unused(", 1));
    EXPECT_TRUE(kept("main(", 1));
}

// Check that used functions are not prunued (duh)
TEST_F(PruneUnusedFunctionsTest, UsedFunction)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "float used(float a);\n"
        "void main() {\n"
        "    gl_FragColor = vec4(used(1.0));\n"
        "}\n"
        "float used(float a) {\n"
        "    return a;\n"
        "}\n";
    compile(shaderString, true);
    EXPECT_TRUE(kept("used(", 3));
    EXPECT_TRUE(kept("main(", 1));

    compile(shaderString, false);
    EXPECT_TRUE(kept("used(", 3));
    EXPECT_TRUE(kept("main(", 1));
}

}
