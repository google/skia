//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RecordConstantPrecision_test.cpp:
//   Test for recording constant variable precision when it affects consuming expression.
//

#include "angle_gl.h"
#include "gtest/gtest.h"
#include "GLSLANG/ShaderLang.h"
#include "tests/test_utils/compiler_test.h"

class RecordConstantPrecisionTest : public testing::Test
{
  public:
    RecordConstantPrecisionTest() {}

  protected:
    void compile(const std::string &shaderString)
    {
        std::string infoLog;
        bool compilationSuccess = compileTestShader(GL_FRAGMENT_SHADER, SH_GLES2_SPEC, SH_ESSL_OUTPUT,
                                                    shaderString, &mTranslatedCode, &infoLog);
        if (!compilationSuccess)
        {
            FAIL() << "Shader compilation into ESSL failed " << infoLog;
        }
    }

    bool foundInCode(const char *stringToFind)
    {
        return mTranslatedCode.find(stringToFind) != std::string::npos;
    }
  private:
    std::string mTranslatedCode;
};

// The constant cannot be folded if its precision is higher than the other operands, since it
// increases the precision of the consuming expression.
TEST_F(RecordConstantPrecisionTest, HigherPrecisionConstantAsParameter)
{
    const std::string &shaderString =
        "uniform mediump float u;"
        "void main()\n"
        "{\n"
        "    const highp float a = 4096.5;\n"
        "    mediump float b = fract(a + u);\n"
        "    gl_FragColor = vec4(b);\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("const highp float s"));
    ASSERT_FALSE(foundInCode("fract(4096.5"));
    ASSERT_FALSE(foundInCode("fract((4096.5"));
}

// The constant can be folded if its precision is equal to the other operands, as it does not
// increase the precision of the consuming expression.
TEST_F(RecordConstantPrecisionTest, EqualPrecisionConstantAsParameter)
{
    const std::string &shaderString =
        "uniform mediump float u;"
        "void main()\n"
        "{\n"
        "    const mediump float a = 4096.5;\n"
        "    mediump float b = fract(a + u);\n"
        "    gl_FragColor = vec4(b);\n"
        "}\n";
    compile(shaderString);
    ASSERT_FALSE(foundInCode("const mediump float s"));
    ASSERT_TRUE(foundInCode("fract((4096.5"));
}

// The constant cannot be folded if its precision is higher than the other operands, since it
// increases the precision of the consuming expression. This applies also when the constant is
// part of a constant expression that can be folded.
TEST_F(RecordConstantPrecisionTest, FoldedBinaryConstantPrecisionIsHigher)
{
    const std::string &shaderString =
        "uniform mediump float u;"
        "void main()\n"
        "{\n"
        "    const highp float a = 4095.5;\n"
        "    mediump float b = fract((a + 1.0) + u);\n"
        "    gl_FragColor = vec4(b);\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("const highp float s"));
    ASSERT_FALSE(foundInCode("fract(4096.5"));
    ASSERT_FALSE(foundInCode("fract((4096.5"));
}


// The constant cannot be folded if its precision is higher than the other operands, since it
// increases the precision of the consuming expression. This applies also when the constant is
// part of a constant expression that can be folded.
TEST_F(RecordConstantPrecisionTest, FoldedUnaryConstantPrecisionIsHigher)
{
    const std::string &shaderString =
        "uniform mediump float u;"
        "void main()\n"
        "{\n"
        "    const highp float a = 0.5;\n"
        "    mediump float b = sin(fract(a) + u);\n"
        "    gl_FragColor = vec4(b);\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("const highp float s"));
    ASSERT_FALSE(foundInCode("sin(0.5"));
    ASSERT_FALSE(foundInCode("sin((0.5"));
}
