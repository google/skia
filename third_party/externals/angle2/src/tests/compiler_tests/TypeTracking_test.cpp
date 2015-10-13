//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TypeTracking_test.cpp:
//   Test for tracking types resulting from math operations, including their
//   precision.
//

#include "angle_gl.h"
#include "gtest/gtest.h"
#include "GLSLANG/ShaderLang.h"
#include "compiler/translator/TranslatorESSL.h"

class TypeTrackingTest : public testing::Test
{
  public:
    TypeTrackingTest() {}

  protected:
    virtual void SetUp()
    {
        ShBuiltInResources resources;
        ShInitBuiltInResources(&resources);
        resources.FragmentPrecisionHigh = 1;

        mTranslator = new TranslatorESSL(GL_FRAGMENT_SHADER, SH_GLES3_SPEC);
        ASSERT_TRUE(mTranslator->Init(resources));
    }

    virtual void TearDown()
    {
        delete mTranslator;
    }

    void compile(const std::string& shaderString)
    {
        const char *shaderStrings[] = { shaderString.c_str() };
        bool compilationSuccess = mTranslator->compile(shaderStrings, 1, SH_INTERMEDIATE_TREE);
        TInfoSink &infoSink = mTranslator->getInfoSink();
        mInfoLog = infoSink.info.c_str();
        if (!compilationSuccess)
            FAIL() << "Shader compilation failed " << mInfoLog;
    }

    bool foundErrorInIntermediateTree() const
    {
        return foundInIntermediateTree("ERROR:");
    }

    bool foundInIntermediateTree(const char* stringToFind) const
    {
        return mInfoLog.find(stringToFind) != std::string::npos;
    }

  private:
    TranslatorESSL *mTranslator;
    std::string mInfoLog;
};

TEST_F(TypeTrackingTest, FunctionPrototypeMangling)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "float fun(float a);\n"
        "uniform float f;\n"
        "void main() {\n"
        "   float ff = fun(f);\n"
        "   gl_FragColor = vec4(ff);\n"
        "}\n"
        "float fun(float a) {\n"
        "   return a * 2.0;\n"
        "}\n";
    compile(shaderString);
    ASSERT_FALSE(foundErrorInIntermediateTree());
    ASSERT_TRUE(foundInIntermediateTree("Function Prototype: fun(f1;"));
    ASSERT_TRUE(foundInIntermediateTree("Function Definition: fun(f1;"));
}

TEST_F(TypeTrackingTest, BuiltInFunctionResultPrecision)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform float f;\n"
        "void main() {\n"
        "   float ff = sin(f);\n"
        "   gl_FragColor = vec4(ff);\n"
        "}\n";
    compile(shaderString);
    ASSERT_FALSE(foundErrorInIntermediateTree());
    ASSERT_TRUE(foundInIntermediateTree("sine (mediump float)"));
}

TEST_F(TypeTrackingTest, BinaryMathResultPrecision)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform float f;\n"
        "void main() {\n"
        "   float ff = f * 0.5;\n"
        "   gl_FragColor = vec4(ff);\n"
        "}\n";
    compile(shaderString);
    ASSERT_FALSE(foundErrorInIntermediateTree());
    ASSERT_TRUE(foundInIntermediateTree("multiply (mediump float)"));
}

TEST_F(TypeTrackingTest, BuiltInVecFunctionResultTypeAndPrecision)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform vec2 a;\n"
        "void main() {\n"
        "   float b = length(a);\n"
        "   float c = dot(a, vec2(0.5));\n"
        "   float d = distance(vec2(0.5), a);\n"
        "   gl_FragColor = vec4(b, c, d, 1.0);\n"
        "}\n";
    compile(shaderString);
    ASSERT_FALSE(foundErrorInIntermediateTree());
    ASSERT_TRUE(foundInIntermediateTree("length (mediump float)"));
    ASSERT_TRUE(foundInIntermediateTree("dot-product (mediump float)"));
    ASSERT_TRUE(foundInIntermediateTree("distance (mediump float)"));
}

TEST_F(TypeTrackingTest, BuiltInMatFunctionResultTypeAndPrecision)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "uniform mat4 m;\n"
        "void main() {\n"
        "   mat3x2 tmp32 = mat3x2(m);\n"
        "   mat2x3 tmp23 = mat2x3(m);\n"
        "   mat4x2 tmp42 = mat4x2(m);\n"
        "   mat2x4 tmp24 = mat2x4(m);\n"
        "   mat4x3 tmp43 = mat4x3(m);\n"
        "   mat3x4 tmp34 = mat3x4(m);\n"
        "   my_FragColor = vec4(tmp32[2][1] * tmp23[1][2], tmp42[3][1] * tmp24[1][3], tmp43[3][2] * tmp34[2][3], 1.0);\n"
        "}\n";
    compile(shaderString);
    ASSERT_FALSE(foundErrorInIntermediateTree());
    ASSERT_TRUE(foundInIntermediateTree("Construct mat2x3 (mediump 2X3 matrix of float)"));
    ASSERT_TRUE(foundInIntermediateTree("Construct mat3x2 (mediump 3X2 matrix of float)"));
    ASSERT_TRUE(foundInIntermediateTree("Construct mat2x4 (mediump 2X4 matrix of float)"));
    ASSERT_TRUE(foundInIntermediateTree("Construct mat4x2 (mediump 4X2 matrix of float)"));
    ASSERT_TRUE(foundInIntermediateTree("Construct mat3x4 (mediump 3X4 matrix of float)"));
    ASSERT_TRUE(foundInIntermediateTree("Construct mat4x3 (mediump 4X3 matrix of float)"));
}

TEST_F(TypeTrackingTest, BuiltInFunctionChoosesHigherPrecision)
{
    const std::string &shaderString =
        "precision lowp float;\n"
        "uniform mediump vec2 a;\n"
        "uniform lowp vec2 b;\n"
        "void main() {\n"
        "   float c = dot(a, b);\n"
        "   float d = distance(b, a);\n"
        "   gl_FragColor = vec4(c, d, 0.0, 1.0);\n"
        "}\n";
    compile(shaderString);
    ASSERT_FALSE(foundErrorInIntermediateTree());
    ASSERT_TRUE(foundInIntermediateTree("dot-product (mediump float)"));
    ASSERT_TRUE(foundInIntermediateTree("distance (mediump float)"));
}

TEST_F(TypeTrackingTest, BuiltInBoolFunctionResultType)
{
    const std::string &shaderString =
        "uniform bvec4 bees;\n"
        "void main() {\n"
        "   bool b = any(bees);\n"
        "   bool c = all(bees);\n"
        "   bvec4 d = not(bees);\n"
        "   gl_FragColor = vec4(b ? 1.0 : 0.0, c ? 1.0 : 0.0, d.x ? 1.0 : 0.0, 1.0);\n"
        "}\n";
    compile(shaderString);
    ASSERT_FALSE(foundErrorInIntermediateTree());
    ASSERT_TRUE(foundInIntermediateTree("any (bool)"));
    ASSERT_TRUE(foundInIntermediateTree("all (bool)"));
    ASSERT_TRUE(foundInIntermediateTree("Negate conditional (4-component vector of bool)"));
}

TEST_F(TypeTrackingTest, BuiltInVecToBoolFunctionResultType)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform vec2 apples;\n"
        "uniform vec2 oranges;\n"
        "uniform ivec2 foo;\n"
        "uniform ivec2 bar;\n"
        "void main() {\n"
        "   bvec2 a = lessThan(apples, oranges);\n"
        "   bvec2 b = greaterThan(foo, bar);\n"
        "   gl_FragColor = vec4(any(a) ? 1.0 : 0.0, any(b) ? 1.0 : 0.0, 0.0, 1.0);\n"
        "}\n";
    compile(shaderString);
    ASSERT_FALSE(foundErrorInIntermediateTree());
    ASSERT_TRUE(foundInIntermediateTree("Less Than (2-component vector of bool)"));
    ASSERT_TRUE(foundInIntermediateTree("Greater Than (2-component vector of bool)"));
}

TEST_F(TypeTrackingTest, Texture2DResultTypeAndPrecision)
{
    // ESSL spec section 4.5.3: sampler2D and samplerCube are lowp by default
    // ESSL spec section 8: For the texture functions, the precision of the return type matches the precision of the sampler type.
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform sampler2D s;\n"
        "uniform vec2 a;\n"
        "void main() {\n"
        "   vec4 c = texture2D(s, a);\n"
        "   gl_FragColor = c;\n"
        "}\n";
    compile(shaderString);
    ASSERT_FALSE(foundErrorInIntermediateTree());
    ASSERT_TRUE(foundInIntermediateTree("texture2D(s21;vf2; (lowp 4-component vector of float)"));
}

TEST_F(TypeTrackingTest, TextureCubeResultTypeAndPrecision)
{
    // ESSL spec section 4.5.3: sampler2D and samplerCube are lowp by default
    // ESSL spec section 8: For the texture functions, the precision of the return type matches the precision of the sampler type.
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform samplerCube sc;\n"
        "uniform vec3 a;\n"
        "void main() {\n"
        "   vec4 c = textureCube(sc, a);\n"
        "   gl_FragColor = c;\n"
        "}\n";
    compile(shaderString);
    ASSERT_FALSE(foundErrorInIntermediateTree());
    ASSERT_TRUE(foundInIntermediateTree("textureCube(sC1;vf3; (lowp 4-component vector of float)"));
}

TEST_F(TypeTrackingTest, TextureSizeResultTypeAndPrecision)
{
    // ESSL 3.0 spec section 8: textureSize has predefined precision highp
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "uniform sampler2D s;\n"
        "void main() {\n"
        "   ivec2 size = textureSize(s, 0);\n"
        "   if (size.x > 100) {\n"
        "       my_FragColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
        "   } else {\n"
        "       my_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
        "   }\n"
        "}\n";
    compile(shaderString);
    ASSERT_FALSE(foundErrorInIntermediateTree());
    ASSERT_TRUE(foundInIntermediateTree("textureSize(s21;i1; (highp 2-component vector of int)"));
}

TEST_F(TypeTrackingTest, BuiltInConstructorResultTypeAndPrecision)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform float u1;\n"
        "uniform float u2;\n"
        "uniform float u3;\n"
        "uniform float u4;\n"
        "void main() {\n"
        "   vec4 a = vec4(u1, u2, u3, u4);\n"
        "   gl_FragColor = a;\n"
        "}\n";
    compile(shaderString);
    ASSERT_FALSE(foundErrorInIntermediateTree());
    ASSERT_TRUE(foundInIntermediateTree("Construct vec4 (mediump 4-component vector of float)"));
}

TEST_F(TypeTrackingTest, StructConstructorResultNoPrecision)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform vec4 u1;\n"
        "uniform vec4 u2;\n"
        "struct S { highp vec4 a; highp vec4 b; };\n"
        "void main() {\n"
        "   S s = S(u1, u2);\n"
        "   gl_FragColor = s.a;\n"
        "}\n";
    compile(shaderString);
    ASSERT_FALSE(foundErrorInIntermediateTree());
    ASSERT_TRUE(foundInIntermediateTree("Construct structure (structure)"));
}

TEST_F(TypeTrackingTest, PackResultTypeAndPrecision)
{
    // ESSL 3.0 spec section 8.4: pack functions have predefined precision highp
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "precision mediump uint;\n"
        "uniform vec2 uv;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "   uint u0 = packSnorm2x16(uv);\n"
        "   uint u1 = packUnorm2x16(uv);\n"
        "   uint u2 = packHalf2x16(uv);\n"
        "   if (u0 + u1 + u2 > 100u) {\n"
        "       my_FragColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
        "   } else {\n"
        "       my_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
        "   }\n"
        "}\n";
    compile(shaderString);
    ASSERT_FALSE(foundErrorInIntermediateTree());
    ASSERT_TRUE(foundInIntermediateTree("pack Snorm 2x16 (highp uint)"));
    ASSERT_TRUE(foundInIntermediateTree("pack Unorm 2x16 (highp uint)"));
    ASSERT_TRUE(foundInIntermediateTree("pack half 2x16 (highp uint)"));
}

TEST_F(TypeTrackingTest, UnpackNormResultTypeAndPrecision)
{
    // ESSL 3.0 spec section 8.4: unpack(S/U)norm2x16 has predefined precision highp
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "precision mediump uint;\n"
        "uniform uint uu;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "   vec2 v0 = unpackSnorm2x16(uu);\n"
        "   vec2 v1 = unpackUnorm2x16(uu);\n"
        "   if (v0.x * v1.x > 1.0) {\n"
        "       my_FragColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
        "   } else {\n"
        "       my_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
        "   }\n"
        "}\n";
    compile(shaderString);
    ASSERT_FALSE(foundErrorInIntermediateTree());
    ASSERT_TRUE(foundInIntermediateTree("unpack Snorm 2x16 (highp 2-component vector of float)"));
    ASSERT_TRUE(foundInIntermediateTree("unpack Unorm 2x16 (highp 2-component vector of float)"));
}

TEST_F(TypeTrackingTest, UnpackHalfResultTypeAndPrecision)
{
    // ESSL 3.0 spec section 8.4: unpackHalf2x16 has predefined precision mediump
    const std::string &shaderString =
        "#version 300 es\n"
        "precision highp float;\n"
        "precision highp uint;\n"
        "uniform uint uu;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "   vec2 v = unpackHalf2x16(uu);\n"
        "   if (v.x > 1.0) {\n"
        "       my_FragColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
        "   } else {\n"
        "       my_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
        "   }\n"
        "}\n";
    compile(shaderString);
    ASSERT_FALSE(foundErrorInIntermediateTree());
    ASSERT_TRUE(foundInIntermediateTree("unpack half 2x16 (mediump 2-component vector of float)"));
}

TEST_F(TypeTrackingTest, BuiltInAbsSignFunctionFloatResultTypeAndPrecision)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform float fval1;\n"
        "void main() {\n"
        "   float fval2 = abs(fval1);\n"
        "   float fval3 = sign(fval1);\n"
        "   gl_FragColor = vec4(fval1, 0.0, 0.0, 1.0); \n"
        "}\n";
    compile(shaderString);
    ASSERT_FALSE(foundErrorInIntermediateTree());
    ASSERT_TRUE(foundInIntermediateTree("Absolute value (mediump float)"));
    ASSERT_TRUE(foundInIntermediateTree("Sign (mediump float)"));
}

TEST_F(TypeTrackingTest, BuiltInAbsSignFunctionIntResultTypeAndPrecision)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "precision mediump int;\n"
        "uniform int ival1;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "   int ival2 = abs(ival1);\n"
        "   int ival3 = sign(ival1);\n"
        "   my_FragColor = vec4(0.0, 0.0, 0.0, 1.0); \n"
        "}\n";
    compile(shaderString);
    ASSERT_FALSE(foundErrorInIntermediateTree());
    ASSERT_TRUE(foundInIntermediateTree("Absolute value (mediump int)"));
    ASSERT_TRUE(foundInIntermediateTree("Sign (mediump int)"));
}

TEST_F(TypeTrackingTest, BuiltInFloatBitsToIntResultTypeAndPrecision)
{
    // ESSL 3.00 section 8.3: floatBitsTo(U)int returns a highp value.
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "precision mediump int;\n"
        "uniform float f;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "   int i = floatBitsToInt(f);\n"
        "   uint u = floatBitsToUint(f);\n"
        "   my_FragColor = vec4(0.0, 0.0, 0.0, 1.0); \n"
        "}\n";
    compile(shaderString);
    ASSERT_FALSE(foundErrorInIntermediateTree());
    ASSERT_TRUE(foundInIntermediateTree("float bits to int (highp int)"));
    ASSERT_TRUE(foundInIntermediateTree("float bits to uint (highp uint)"));
}

TEST_F(TypeTrackingTest, BuiltInIntBitsToFloatResultTypeAndPrecision)
{
    // ESSL 3.00 section 8.3: (u)intBitsToFloat returns a highp value.
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "precision mediump int;\n"
        "uniform int i;\n"
        "uniform uint u;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "   float f1 = intBitsToFloat(i);\n"
        "   float f2 = uintBitsToFloat(u);\n"
        "   my_FragColor = vec4(f1, f2, 0.0, 1.0); \n"
        "}\n";
    compile(shaderString);
    ASSERT_FALSE(foundErrorInIntermediateTree());
    ASSERT_TRUE(foundInIntermediateTree("int bits to float (highp float)"));
    ASSERT_TRUE(foundInIntermediateTree("uint bits to float (highp float)"));
}
