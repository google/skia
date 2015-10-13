//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// MalformedShader_test.cpp:
//   Tests that malformed shaders fail compilation.
//

#include "angle_gl.h"
#include "gtest/gtest.h"
#include "GLSLANG/ShaderLang.h"
#include "compiler/translator/TranslatorESSL.h"

class MalformedShaderTest : public testing::Test
{
  public:
    MalformedShaderTest() {}

  protected:
    virtual void SetUp()
    {
        ShBuiltInResources resources;
        ShInitBuiltInResources(&resources);

        mTranslator = new TranslatorESSL(GL_FRAGMENT_SHADER, SH_GLES3_SPEC);
        ASSERT_TRUE(mTranslator->Init(resources));
    }

    virtual void TearDown()
    {
        delete mTranslator;
    }

    // Return true when compilation succeeds
    bool compile(const std::string& shaderString)
    {
        const char *shaderStrings[] = { shaderString.c_str() };
        bool compilationSuccess = mTranslator->compile(shaderStrings, 1, SH_INTERMEDIATE_TREE);
        TInfoSink &infoSink = mTranslator->getInfoSink();
        mInfoLog = infoSink.info.c_str();
        return compilationSuccess;
    }

    bool hasWarning() const
    {
        return mInfoLog.find("WARNING: ") != std::string::npos;
    }

  protected:
    std::string mInfoLog;
    TranslatorESSL *mTranslator;
};

class MalformedVertexShaderTest : public MalformedShaderTest
{
  public:
    MalformedVertexShaderTest() {}

  protected:
    void SetUp() override
    {
        ShBuiltInResources resources;
        ShInitBuiltInResources(&resources);

        mTranslator = new TranslatorESSL(GL_VERTEX_SHADER, SH_GLES3_SPEC);
        ASSERT_TRUE(mTranslator->Init(resources));
    }
};

// This is a test for a bug that used to exist in ANGLE:
// Calling a function with all parameters missing should not succeed.
TEST_F(MalformedShaderTest, FunctionParameterMismatch)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "float fun(float a) {\n"
        "   return a * 2.0;\n"
        "}\n"
        "void main() {\n"
        "   float ff = fun();\n"
        "   gl_FragColor = vec4(ff);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Functions can't be redeclared as variables in the same scope (ESSL 1.00 section 4.2.7)
TEST_F(MalformedShaderTest, RedeclaringFunctionAsVariable)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "float fun(float a) {\n"
        "   return a * 2.0;\n"
        "}\n"
        "float fun;\n"
        "void main() {\n"
        "   gl_FragColor = vec4(0.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Functions can't be redeclared as structs in the same scope (ESSL 1.00 section 4.2.7)
TEST_F(MalformedShaderTest, RedeclaringFunctionAsStruct)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "float fun(float a) {\n"
        "   return a * 2.0;\n"
        "}\n"
        "struct fun { float a; };\n"
        "void main() {\n"
        "   gl_FragColor = vec4(0.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Functions can't be redeclared with different qualifiers (ESSL 1.00 section 6.1.0)
TEST_F(MalformedShaderTest, RedeclaringFunctionWithDifferentQualifiers)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "float fun(out float a);\n"
        "float fun(float a) {\n"
        "   return a * 2.0;\n"
        "}\n"
        "void main() {\n"
        "   gl_FragColor = vec4(0.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Assignment and equality are undefined for structures containing arrays (ESSL 1.00 section 5.7)
TEST_F(MalformedShaderTest, CompareStructsContainingArrays)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "struct s { float a[3]; };\n"
        "void main() {\n"
        "   s a;\n"
        "   s b;\n"
        "   bool c = (a == b);\n"
        "   gl_FragColor = vec4(c ? 1.0 : 0.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Assignment and equality are undefined for structures containing arrays (ESSL 1.00 section 5.7)
TEST_F(MalformedShaderTest, AssignStructsContainingArrays)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "struct s { float a[3]; };\n"
        "void main() {\n"
        "   s a;\n"
        "   s b;\n"
        "   b.a[0] = 0.0;\n"
        "   a = b;\n"
        "   gl_FragColor = vec4(a.a[0]);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Assignment and equality are undefined for structures containing samplers (ESSL 1.00 sections 5.7 and 5.9)
TEST_F(MalformedShaderTest, CompareStructsContainingSamplers)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "struct s { sampler2D foo; };\n"
        "uniform s a;\n"
        "uniform s b;\n"
        "void main() {\n"
        "   bool c = (a == b);\n"
        "   gl_FragColor = vec4(c ? 1.0 : 0.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Samplers are not allowed as l-values (ESSL 3.00 section 4.1.7), our interpretation is that this
// extends to structs containing samplers. ESSL 1.00 spec is clearer about this.
TEST_F(MalformedShaderTest, AssignStructsContainingSamplers)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "struct s { sampler2D foo; };\n"
        "uniform s a;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "   s b;\n"
        "   b = a;\n"
        "   my_FragColor = vec4(1.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// This is a regression test for a particular bug that was in ANGLE.
// It also verifies that ESSL3 functionality doesn't leak to ESSL1.
TEST_F(MalformedShaderTest, ArrayWithNoSizeInInitializerList)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "void main() {\n"
        "   float a[2], b[];\n"
        "   gl_FragColor = vec4(1.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Const variables need an initializer.
TEST_F(MalformedShaderTest, ConstVarNotInitialized)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "   const float a;\n"
        "   my_FragColor = vec4(1.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Const variables need an initializer. In ESSL1 const structs containing
// arrays are not allowed at all since it's impossible to initialize them.
// Even though this test is for ESSL3 the only thing that's critical for
// ESSL1 is the non-initialization check that's used for both language versions.
// Whether ESSL1 compilation generates the most helpful error messages is a
// secondary concern.
TEST_F(MalformedShaderTest, ConstStructNotInitialized)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "struct S {\n"
        "   float a[3];\n"
        "};\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "   const S b;\n"
        "   my_FragColor = vec4(1.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Const variables need an initializer. In ESSL1 const arrays are not allowed
// at all since it's impossible to initialize them.
// Even though this test is for ESSL3 the only thing that's critical for
// ESSL1 is the non-initialization check that's used for both language versions.
// Whether ESSL1 compilation generates the most helpful error messages is a
// secondary concern.
TEST_F(MalformedShaderTest, ConstArrayNotInitialized)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "   const float a[3];\n"
        "   my_FragColor = vec4(1.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Block layout qualifiers can't be used on non-block uniforms (ESSL 3.00 section 4.3.8.3)
TEST_F(MalformedShaderTest, BlockLayoutQualifierOnRegularUniform)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "layout(packed) uniform mat2 x;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "   my_FragColor = vec4(1.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Block layout qualifiers can't be used on non-block uniforms (ESSL 3.00 section 4.3.8.3)
TEST_F(MalformedShaderTest, BlockLayoutQualifierOnUniformWithEmptyDecl)
{
    // Yes, the comma in the declaration below is not a typo.
    // Empty declarations are allowed in GLSL.
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "layout(packed) uniform mat2, x;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "   my_FragColor = vec4(1.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Arrays of arrays are not allowed (ESSL 3.00 section 4.1.9)
TEST_F(MalformedShaderTest, ArraysOfArrays1)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "   float[5] a[3];\n"
        "   my_FragColor = vec4(1.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Arrays of arrays are not allowed (ESSL 3.00 section 4.1.9)
TEST_F(MalformedShaderTest, ArraysOfArrays2)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "   float[2] a, b[3];\n"
        "   my_FragColor = vec4(1.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Implicitly sized arrays need to be initialized (ESSL 3.00 section 4.1.9)
TEST_F(MalformedShaderTest, UninitializedImplicitArraySize)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "   float[] a;\n"
        "   my_FragColor = vec4(1.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// An operator can only form a constant expression if all the operands are constant expressions
// - even operands of ternary operator that are never evaluated. (ESSL 3.00 section 4.3.3)
TEST_F(MalformedShaderTest, TernaryOperatorNotConstantExpression)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "uniform bool u;\n"
        "void main() {\n"
        "   const bool a = true ? true : u;\n"
        "   my_FragColor = vec4(1.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Ternary operator can't operate on arrays (ESSL 3.00 section 5.7)
TEST_F(MalformedShaderTest, TernaryOperatorOnArrays)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "   float[1] a = float[1](0.0);\n"
        "   float[1] b = float[1](1.0);\n"
        "   float[1] c = true ? a : b;\n"
        "   my_FragColor = vec4(1.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Ternary operator can't operate on structs (ESSL 3.00 section 5.7)
TEST_F(MalformedShaderTest, TernaryOperatorOnStructs)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "struct S { float foo; };\n"
        "void main() {\n"
        "   S a = S(0.0);\n"
        "   S b = S(1.0);\n"
        "   S c = true ? a : b;\n"
        "   my_FragColor = vec4(1.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Array length() returns a constant signed integral expression (ESSL 3.00 section 4.1.9)
// Assigning it to unsigned should result in an error.
TEST_F(MalformedShaderTest, AssignArrayLengthToUnsigned)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "   int[1] arr;\n"
        "   uint l = arr.length();\n"
        "   my_FragColor = vec4(float(l));\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Global variable initializers need to be constant expressions (ESSL 1.00 section 4.3)
// Initializing with a varying should be an error.
TEST_F(MalformedShaderTest, AssignVaryingToGlobal)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "varying float a;\n"
        "float b = a * 2.0;\n"
        "void main() {\n"
        "   gl_FragColor = vec4(b);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Global variable initializers need to be constant expressions (ESSL 3.00 section 4.3)
// Initializing with an uniform should be an error.
TEST_F(MalformedShaderTest, AssignUniformToGlobalESSL3)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "uniform float a;\n"
        "float b = a * 2.0;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "   my_FragColor = vec4(b);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Global variable initializers need to be constant expressions (ESSL 1.00 section 4.3)
// Initializing with an uniform should generate a warning
// (we don't generate an error on ESSL 1.00 because of legacy compatibility)
TEST_F(MalformedShaderTest, AssignUniformToGlobalESSL1)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform float a;\n"
        "float b = a * 2.0;\n"
        "void main() {\n"
        "   gl_FragColor = vec4(b);\n"
        "}\n";
    if (compile(shaderString))
    {
        if (!hasWarning())
        {
            FAIL() << "Shader compilation succeeded without warnings, expecting warning " << mInfoLog;
        }
    }
    else
    {
        FAIL() << "Shader compilation failed, expecting success with warning " << mInfoLog;
    }
}

// Global variable initializers need to be constant expressions (ESSL 1.00 section 4.3)
// Initializing with an user-defined function call should be an error.
TEST_F(MalformedShaderTest, AssignFunctionCallToGlobal)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "float foo() { return 1.0; }\n"
        "float b = foo();\n"
        "void main() {\n"
        "   gl_FragColor = vec4(b);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Global variable initializers need to be constant expressions (ESSL 1.00 section 4.3)
// Initializing with an assignment to another global should be an error.
TEST_F(MalformedShaderTest, AssignAssignmentToGlobal)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "float c = 1.0;\n"
        "float b = (c = 0.0);\n"
        "void main() {\n"
        "   gl_FragColor = vec4(b);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Global variable initializers need to be constant expressions (ESSL 1.00 section 4.3)
// Initializing with incrementing another global should be an error.
TEST_F(MalformedShaderTest, AssignIncrementToGlobal)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "float c = 1.0;\n"
        "float b = (c++);\n"
        "void main() {\n"
        "   gl_FragColor = vec4(b);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Global variable initializers need to be constant expressions (ESSL 1.00 section 4.3)
// Initializing with a texture lookup function call should be an error.
TEST_F(MalformedShaderTest, AssignTexture2DToGlobal)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform mediump sampler2D s;\n"
        "float b = texture2D(s, vec2(0.5, 0.5)).x;\n"
        "void main() {\n"
        "   gl_FragColor = vec4(b);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Global variable initializers need to be constant expressions (ESSL 3.00 section 4.3)
// Initializing with a non-constant global should be an error.
TEST_F(MalformedShaderTest, AssignNonConstGlobalToGlobal)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "float a = 1.0;\n"
        "float b = a * 2.0;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "   my_FragColor = vec4(b);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Global variable initializers need to be constant expressions (ESSL 3.00 section 4.3)
// Initializing with a constant global should be fine.
TEST_F(MalformedShaderTest, AssignConstGlobalToGlobal)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "const float a = 1.0;\n"
        "float b = a * 2.0;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "   my_FragColor = vec4(b);\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success " << mInfoLog;
    }
}

// Statically assigning to both gl_FragData and gl_FragColor is forbidden (ESSL 1.00 section 7.2)
TEST_F(MalformedShaderTest, WriteBothFragDataAndFragColor)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "void foo() {\n"
        "   gl_FragData[0].a++;\n"
        "}\n"
        "void main() {\n"
        "   gl_FragColor.x += 0.0;\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Version directive must be on the first line (ESSL 3.00 section 3.3)
TEST_F(MalformedShaderTest, VersionOnSecondLine)
{
    const std::string &shaderString =
        "\n"
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "   my_FragColor = vec4(0.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Layout qualifier can only appear in global scope (ESSL 3.00 section 4.3.8)
TEST_F(MalformedShaderTest, LayoutQualifierInCondition)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "uniform vec4 u;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "    int i = 0;\n"
        "    for (int j = 0; layout(location = 0) bool b = false; ++j) {\n"
        "        ++i;\n"
        "    }\n"
        "    my_FragColor = u;\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Layout qualifier can only appear where specified (ESSL 3.00 section 4.3.8)
TEST_F(MalformedShaderTest, LayoutQualifierInFunctionReturnType)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "uniform vec4 u;\n"
        "out vec4 my_FragColor;\n"
        "layout(location = 0) vec4 foo() {\n"
        "    return u;\n"
        "}\n"
        "void main() {\n"
        "    my_FragColor = foo();\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// If there is more than one output, the location must be specified for all outputs.
// (ESSL 3.00.04 section 4.3.8.2)
TEST_F(MalformedShaderTest, TwoOutputsNoLayoutQualifiers)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "uniform vec4 u;\n"
        "out vec4 my_FragColor;\n"
        "out vec4 my_SecondaryFragColor;\n"
        "void main() {\n"
        "    my_FragColor = vec4(1.0);\n"
        "    my_SecondaryFragColor = vec4(0.5);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// (ESSL 3.00.04 section 4.3.8.2)
TEST_F(MalformedShaderTest, TwoOutputsFirstLayoutQualifier)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "uniform vec4 u;\n"
        "layout(location = 0) out vec4 my_FragColor;\n"
        "out vec4 my_SecondaryFragColor;\n"
        "void main() {\n"
        "    my_FragColor = vec4(1.0);\n"
        "    my_SecondaryFragColor = vec4(0.5);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// (ESSL 3.00.04 section 4.3.8.2)
TEST_F(MalformedShaderTest, TwoOutputsSecondLayoutQualifier)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "uniform vec4 u;\n"
        "out vec4 my_FragColor;\n"
        "layout(location = 0) out vec4 my_SecondaryFragColor;\n"
        "void main() {\n"
        "    my_FragColor = vec4(1.0);\n"
        "    my_SecondaryFragColor = vec4(0.5);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Uniforms can be arrays (ESSL 3.00 section 4.3.5)
TEST_F(MalformedShaderTest, UniformArray)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "uniform vec4[2] u;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "    my_FragColor = u[0];\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success " << mInfoLog;
    }
}

// Fragment shader input variables cannot be arrays of structs (ESSL 3.00 section 4.3.4)
TEST_F(MalformedShaderTest, FragmentInputArrayOfStructs)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "struct S {\n"
        "    vec4 foo;\n"
        "};\n"
        "in S i[2];\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "    my_FragColor = i[0].foo;\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Vertex shader inputs can't be arrays (ESSL 3.00 section 4.3.4)
// This test is testing the case where the array brackets are after the variable name, so
// the arrayness isn't known when the type and qualifiers are initially parsed.
TEST_F(MalformedVertexShaderTest, VertexShaderInputArray)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in vec4 i[2];\n"
        "void main() {\n"
        "    gl_Position = i[0];\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Vertex shader inputs can't be arrays (ESSL 3.00 section 4.3.4)
// This test is testing the case where the array brackets are after the type.
TEST_F(MalformedVertexShaderTest, VertexShaderInputArrayType)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in vec4[2] i;\n"
        "void main() {\n"
        "    gl_Position = i[0];\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Fragment shader inputs can't contain booleans (ESSL 3.00 section 4.3.4)
TEST_F(MalformedShaderTest, FragmentShaderInputStructWithBool)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "struct S {\n"
        "    bool foo;\n"
        "};\n"
        "in S s;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "    my_FragColor = vec4(0.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}

// Fragment shader inputs without a flat qualifier can't contain integers (ESSL 3.00 section 4.3.4)
TEST_F(MalformedShaderTest, FragmentShaderInputStructWithInt)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "struct S {\n"
        "    int foo;\n"
        "};\n"
        "in S s;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "    my_FragColor = vec4(0.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure " << mInfoLog;
    }
}
