//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DebugShaderPrecision_test.cpp:
//   Tests for writing the code for shader precision emulation.
//

#include "angle_gl.h"
#include "gtest/gtest.h"
#include "GLSLANG/ShaderLang.h"
#include "tests/test_utils/compiler_test.h"

class DebugShaderPrecisionTest : public testing::Test
{
  public:
    DebugShaderPrecisionTest() {}

  protected:
    void compile(const std::string& shaderString)
    {
        std::string infoLog;
        bool compilationSuccess = compileWithSettings(SH_ESSL_OUTPUT, shaderString, &mESSLCode, &infoLog);
        if (!compilationSuccess)
        {
            FAIL() << "Shader compilation into ESSL failed " << infoLog;
        }

        compilationSuccess = compileWithSettings(SH_GLSL_COMPATIBILITY_OUTPUT, shaderString, &mGLSLCode, &infoLog);
        if (!compilationSuccess)
        {
            FAIL() << "Shader compilation into GLSL failed " << infoLog;
        }
    }

    bool foundInESSLCode(const char* stringToFind)
    {
        return mESSLCode.find(stringToFind) != std::string::npos;
    }

    bool foundInGLSLCode(const char* stringToFind)
    {
        return mGLSLCode.find(stringToFind) != std::string::npos;
    }

    bool foundInCode(const char* stringToFind)
    {
        return foundInESSLCode(stringToFind) && foundInGLSLCode(stringToFind);
    }

    bool notFoundInCode(const char* stringToFind)
    {
        return !foundInESSLCode(stringToFind) && !foundInGLSLCode(stringToFind);
    }

  private:
    bool compileWithSettings(ShShaderOutput output, const std::string &shaderString,
                             std::string *translatedCode, std::string *infoLog)
    {
        ShBuiltInResources resources;
        ShInitBuiltInResources(&resources);
        resources.WEBGL_debug_shader_precision = 1;
        return compileTestShader(GL_FRAGMENT_SHADER, SH_GLES3_SPEC, output, shaderString,
                                 &resources, translatedCode, infoLog);
    }

    std::string mESSLCode;
    std::string mGLSLCode;
};

class NoDebugShaderPrecisionTest : public testing::Test
{
  public:
    NoDebugShaderPrecisionTest() {}

  protected:
    bool compile(const std::string& shaderString)
    {
        return compileTestShader(GL_FRAGMENT_SHADER, SH_GLES2_SPEC, SH_GLSL_COMPATIBILITY_OUTPUT,
                                 shaderString, &mCode, nullptr);
    }

    bool foundInCode(const char* stringToFind)
    {
        return mCode.find(stringToFind) != std::string::npos;
    }

  private:
    std::string mCode;
};

TEST_F(DebugShaderPrecisionTest, RoundingFunctionsDefined)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform float u;\n"
        "void main() {\n"
        "   gl_FragColor = vec4(u);\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInESSLCode("highp float angle_frm(in highp float"));
    ASSERT_TRUE(foundInESSLCode("highp vec2 angle_frm(in highp vec2"));
    ASSERT_TRUE(foundInESSLCode("highp vec3 angle_frm(in highp vec3"));
    ASSERT_TRUE(foundInESSLCode("highp vec4 angle_frm(in highp vec4"));
    ASSERT_TRUE(foundInESSLCode("highp mat2 angle_frm(in highp mat2"));
    ASSERT_TRUE(foundInESSLCode("highp mat3 angle_frm(in highp mat3"));
    ASSERT_TRUE(foundInESSLCode("highp mat4 angle_frm(in highp mat4"));

    ASSERT_TRUE(foundInESSLCode("highp float angle_frl(in highp float"));
    ASSERT_TRUE(foundInESSLCode("highp vec2 angle_frl(in highp vec2"));
    ASSERT_TRUE(foundInESSLCode("highp vec3 angle_frl(in highp vec3"));
    ASSERT_TRUE(foundInESSLCode("highp vec4 angle_frl(in highp vec4"));
    ASSERT_TRUE(foundInESSLCode("highp mat2 angle_frl(in highp mat2"));
    ASSERT_TRUE(foundInESSLCode("highp mat3 angle_frl(in highp mat3"));
    ASSERT_TRUE(foundInESSLCode("highp mat4 angle_frl(in highp mat4"));

    ASSERT_TRUE(foundInGLSLCode("float angle_frm(in float"));
    ASSERT_TRUE(foundInGLSLCode("vec2 angle_frm(in vec2"));
    ASSERT_TRUE(foundInGLSLCode("vec3 angle_frm(in vec3"));
    ASSERT_TRUE(foundInGLSLCode("vec4 angle_frm(in vec4"));
    ASSERT_TRUE(foundInGLSLCode("mat2 angle_frm(in mat2"));
    ASSERT_TRUE(foundInGLSLCode("mat3 angle_frm(in mat3"));
    ASSERT_TRUE(foundInGLSLCode("mat4 angle_frm(in mat4"));

    ASSERT_TRUE(foundInGLSLCode("float angle_frl(in float"));
    ASSERT_TRUE(foundInGLSLCode("vec2 angle_frl(in vec2"));
    ASSERT_TRUE(foundInGLSLCode("vec3 angle_frl(in vec3"));
    ASSERT_TRUE(foundInGLSLCode("vec4 angle_frl(in vec4"));
    ASSERT_TRUE(foundInGLSLCode("mat2 angle_frl(in mat2"));
    ASSERT_TRUE(foundInGLSLCode("mat3 angle_frl(in mat3"));
    ASSERT_TRUE(foundInGLSLCode("mat4 angle_frl(in mat4"));
}

TEST_F(DebugShaderPrecisionTest, PragmaDisablesEmulation)
{
    const std::string &shaderString =
        "#pragma webgl_debug_shader_precision(off)\n"
        "precision mediump float;\n"
        "uniform float u;\n"
        "void main() {\n"
        "   gl_FragColor = vec4(u);\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(notFoundInCode("angle_frm"));
    const std::string &shaderStringPragmaOn =
        "#pragma webgl_debug_shader_precision(on)\n"
        "precision mediump float;\n"
        "uniform float u;\n"
        "void main() {\n"
        "   gl_FragColor = vec4(u);\n"
        "}\n";
    compile(shaderStringPragmaOn);
    ASSERT_TRUE(foundInCode("angle_frm"));
}

// Emulation can't be toggled on for only a part of a shader.
// Only the last pragma in the shader has an effect.
TEST_F(DebugShaderPrecisionTest, MultiplePragmas)
{
    const std::string &shaderString =
        "#pragma webgl_debug_shader_precision(off)\n"
        "precision mediump float;\n"
        "uniform float u;\n"
        "void main() {\n"
        "   gl_FragColor = vec4(u);\n"
        "}\n"
        "#pragma webgl_debug_shader_precision(on)\n";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("angle_frm"));
}

TEST_F(NoDebugShaderPrecisionTest, HelpersWrittenOnlyWithExtension)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform float u;\n"
        "void main() {\n"
        "   gl_FragColor = vec4(u);\n"
        "}\n";
    ASSERT_TRUE(compile(shaderString));
    ASSERT_FALSE(foundInCode("angle_frm"));
}

TEST_F(NoDebugShaderPrecisionTest, PragmaHasEffectsOnlyWithExtension)
{
    const std::string &shaderString =
        "#pragma webgl_debug_shader_precision(on)\n"
        "precision mediump float;\n"
        "uniform float u;\n"
        "void main() {\n"
        "   gl_FragColor = vec4(u);\n"
        "}\n";
    ASSERT_TRUE(compile(shaderString));
    ASSERT_FALSE(foundInCode("angle_frm"));
}

TEST_F(DebugShaderPrecisionTest, DeclarationsAndConstants)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform vec4 f;\n"
        "uniform float uu, uu2;\n"
        "varying float vv, vv2;\n"
        "float gg = 0.0, gg2;\n"
        "void main() {\n"
        "   float aa = 0.0, aa2;\n"
        "   gl_FragColor = f;\n"
        "}\n";
    compile(shaderString);
    // Declarations or constants should not have rounding inserted around them
    ASSERT_TRUE(notFoundInCode("angle_frm(0"));
    ASSERT_TRUE(notFoundInCode("angle_frm(uu"));
    ASSERT_TRUE(notFoundInCode("angle_frm(vv"));
    ASSERT_TRUE(notFoundInCode("angle_frm(gg"));
    ASSERT_TRUE(notFoundInCode("angle_frm(aa"));
}

TEST_F(DebugShaderPrecisionTest, InitializerRounding)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform float u;\n"
        "void main() {\n"
        "   float a = u;\n"
        "   gl_FragColor = vec4(a);\n"
        "}\n";
    compile(shaderString);
    // An expression that's part of initialization should have rounding
    ASSERT_TRUE(foundInCode("angle_frm(u)"));
}

TEST_F(DebugShaderPrecisionTest, CompoundAddFunction)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform vec4 u;\n"
        "uniform vec4 u2;\n"
        "void main() {\n"
        "   vec4 v = u;\n"
        "   v += u2;\n"
        "   gl_FragColor = v;\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInESSLCode(
        "highp vec4 angle_compound_add_frm(inout highp vec4 x, in highp vec4 y) {\n"
        "    x = angle_frm(angle_frm(x) + y);"
    ));
    ASSERT_TRUE(foundInGLSLCode(
        "vec4 angle_compound_add_frm(inout vec4 x, in vec4 y) {\n"
        "    x = angle_frm(angle_frm(x) + y);"
    ));
    ASSERT_TRUE(foundInCode("angle_compound_add_frm(v, angle_frm(u2));"));
    ASSERT_TRUE(notFoundInCode("+="));
}

TEST_F(DebugShaderPrecisionTest, CompoundSubFunction)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform vec4 u;\n"
        "uniform vec4 u2;\n"
        "void main() {\n"
        "   vec4 v = u;\n"
        "   v -= u2;\n"
        "   gl_FragColor = v;\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInESSLCode(
        "highp vec4 angle_compound_sub_frm(inout highp vec4 x, in highp vec4 y) {\n"
        "    x = angle_frm(angle_frm(x) - y);"
    ));
    ASSERT_TRUE(foundInGLSLCode(
        "vec4 angle_compound_sub_frm(inout vec4 x, in vec4 y) {\n"
        "    x = angle_frm(angle_frm(x) - y);"
    ));
    ASSERT_TRUE(foundInCode("angle_compound_sub_frm(v, angle_frm(u2));"));
    ASSERT_TRUE(notFoundInCode("-="));
}

TEST_F(DebugShaderPrecisionTest, CompoundDivFunction)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform vec4 u;\n"
        "uniform vec4 u2;\n"
        "void main() {\n"
        "   vec4 v = u;\n"
        "   v /= u2;\n"
        "   gl_FragColor = v;\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInESSLCode(
        "highp vec4 angle_compound_div_frm(inout highp vec4 x, in highp vec4 y) {\n"
        "    x = angle_frm(angle_frm(x) / y);"
    ));
    ASSERT_TRUE(foundInGLSLCode(
        "vec4 angle_compound_div_frm(inout vec4 x, in vec4 y) {\n"
        "    x = angle_frm(angle_frm(x) / y);"
    ));
    ASSERT_TRUE(foundInCode("angle_compound_div_frm(v, angle_frm(u2));"));
    ASSERT_TRUE(notFoundInCode("/="));
}

TEST_F(DebugShaderPrecisionTest, CompoundMulFunction)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform vec4 u;\n"
        "uniform vec4 u2;\n"
        "void main() {\n"
        "   vec4 v = u;\n"
        "   v *= u2;\n"
        "   gl_FragColor = v;\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInESSLCode(
        "highp vec4 angle_compound_mul_frm(inout highp vec4 x, in highp vec4 y) {\n"
        "    x = angle_frm(angle_frm(x) * y);"
    ));
    ASSERT_TRUE(foundInGLSLCode(
        "vec4 angle_compound_mul_frm(inout vec4 x, in vec4 y) {\n"
        "    x = angle_frm(angle_frm(x) * y);"
    ));
    ASSERT_TRUE(foundInCode("angle_compound_mul_frm(v, angle_frm(u2));"));
    ASSERT_TRUE(notFoundInCode("*="));
}

TEST_F(DebugShaderPrecisionTest, CompoundAddVectorPlusScalarFunction)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform vec4 u;\n"
        "uniform float u2;\n"
        "void main() {\n"
        "   vec4 v = u;\n"
        "   v += u2;\n"
        "   gl_FragColor = v;\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInESSLCode(
        "highp vec4 angle_compound_add_frm(inout highp vec4 x, in highp float y) {\n"
        "    x = angle_frm(angle_frm(x) + y);"
    ));
    ASSERT_TRUE(foundInGLSLCode(
        "vec4 angle_compound_add_frm(inout vec4 x, in float y) {\n"
        "    x = angle_frm(angle_frm(x) + y);"
    ));
    ASSERT_TRUE(foundInCode("angle_compound_add_frm(v, angle_frm(u2));"));
    ASSERT_TRUE(notFoundInCode("+="));
}

TEST_F(DebugShaderPrecisionTest, CompoundMatrixTimesMatrixFunction)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform mat4 u;\n"
        "uniform mat4 u2;\n"
        "void main() {\n"
        "   mat4 m = u;\n"
        "   m *= u2;\n"
        "   gl_FragColor = m[0];\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInESSLCode(
        "highp mat4 angle_compound_mul_frm(inout highp mat4 x, in highp mat4 y) {\n"
        "    x = angle_frm(angle_frm(x) * y);"
    ));
    ASSERT_TRUE(foundInGLSLCode(
        "mat4 angle_compound_mul_frm(inout mat4 x, in mat4 y) {\n"
        "    x = angle_frm(angle_frm(x) * y);"
    ));
    ASSERT_TRUE(foundInCode("angle_compound_mul_frm(m, angle_frm(u2));"));
    ASSERT_TRUE(notFoundInCode("*="));
}

TEST_F(DebugShaderPrecisionTest, CompoundMatrixTimesScalarFunction)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform mat4 u;\n"
        "uniform float u2;\n"
        "void main() {\n"
        "   mat4 m = u;\n"
        "   m *= u2;\n"
        "   gl_FragColor = m[0];\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInESSLCode(
        "highp mat4 angle_compound_mul_frm(inout highp mat4 x, in highp float y) {\n"
        "    x = angle_frm(angle_frm(x) * y);"
    ));
    ASSERT_TRUE(foundInGLSLCode(
        "mat4 angle_compound_mul_frm(inout mat4 x, in float y) {\n"
        "    x = angle_frm(angle_frm(x) * y);"
    ));
    ASSERT_TRUE(foundInCode("angle_compound_mul_frm(m, angle_frm(u2));"));
    ASSERT_TRUE(notFoundInCode("*="));
}

TEST_F(DebugShaderPrecisionTest, CompoundVectorTimesMatrixFunction)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform vec4 u;\n"
        "uniform mat4 u2;\n"
        "void main() {\n"
        "   vec4 v = u;\n"
        "   v *= u2;\n"
        "   gl_FragColor = v;\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInESSLCode(
        "highp vec4 angle_compound_mul_frm(inout highp vec4 x, in highp mat4 y) {\n"
        "    x = angle_frm(angle_frm(x) * y);"
    ));
    ASSERT_TRUE(foundInGLSLCode("vec4 angle_compound_mul_frm(inout vec4 x, in mat4 y) {\n"
        "    x = angle_frm(angle_frm(x) * y);"
    ));
    ASSERT_TRUE(foundInCode("angle_compound_mul_frm(v, angle_frm(u2));"));
    ASSERT_TRUE(notFoundInCode("*="));
}

TEST_F(DebugShaderPrecisionTest, CompoundVectorTimesScalarFunction)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform vec4 u;\n"
        "uniform float u2;\n"
        "void main() {\n"
        "   vec4 v = u;\n"
        "   v *= u2;\n"
        "   gl_FragColor = v;\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInESSLCode(
        "highp vec4 angle_compound_mul_frm(inout highp vec4 x, in highp float y) {\n"
        "    x = angle_frm(angle_frm(x) * y);"
    ));
    ASSERT_TRUE(foundInGLSLCode(
        "vec4 angle_compound_mul_frm(inout vec4 x, in float y) {\n"
        "    x = angle_frm(angle_frm(x) * y);"
    ));
    ASSERT_TRUE(foundInCode("angle_compound_mul_frm(v, angle_frm(u2));"));
    ASSERT_TRUE(notFoundInCode("*="));
}

TEST_F(DebugShaderPrecisionTest, BinaryMathRounding)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform vec4 u1;\n"
        "uniform vec4 u2;\n"
        "uniform vec4 u3;\n"
        "uniform vec4 u4;\n"
        "uniform vec4 u5;\n"
        "void main() {\n"
        "   vec4 v1 = u1 + u2;\n"
        "   vec4 v2 = u2 - u3;\n"
        "   vec4 v3 = u3 * u4;\n"
        "   vec4 v4 = u4 / u5;\n"
        "   vec4 v5;\n"
        "   vec4 v6 = (v5 = u5);\n"
        "   gl_FragColor = v1 + v2 + v3 + v4;\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("v1 = angle_frm((angle_frm(u1) + angle_frm(u2)))"));
    ASSERT_TRUE(foundInCode("v2 = angle_frm((angle_frm(u2) - angle_frm(u3)))"));
    ASSERT_TRUE(foundInCode("v3 = angle_frm((angle_frm(u3) * angle_frm(u4)))"));
    ASSERT_TRUE(foundInCode("v4 = angle_frm((angle_frm(u4) / angle_frm(u5)))"));
    ASSERT_TRUE(foundInCode("v6 = angle_frm((v5 = angle_frm(u5)))"));
}

TEST_F(DebugShaderPrecisionTest, BuiltInMathFunctionRounding)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform vec4 u1;\n"
        "uniform vec4 u2;\n"
        "uniform vec4 u3;\n"
        "uniform float uf;\n"
        "uniform float uf2;\n"
        "uniform vec3 uf31;\n"
        "uniform vec3 uf32;\n"
        "uniform mat4 um1;\n"
        "uniform mat4 um2;\n"
        "void main() {\n"
        "   vec4 v1 = radians(u1);\n"
        "   vec4 v2 = degrees(u1);\n"
        "   vec4 v3 = sin(u1);\n"
        "   vec4 v4 = cos(u1);\n"
        "   vec4 v5 = tan(u1);\n"
        "   vec4 v6 = asin(u1);\n"
        "   vec4 v7 = acos(u1);\n"
        "   vec4 v8 = atan(u1);\n"
        "   vec4 v9 = atan(u1, u2);\n"
        "   vec4 v10 = pow(u1, u2);\n"
        "   vec4 v11 = exp(u1);\n"
        "   vec4 v12 = log(u1);\n"
        "   vec4 v13 = exp2(u1);\n"
        "   vec4 v14 = log2(u1);\n"
        "   vec4 v15 = sqrt(u1);\n"
        "   vec4 v16 = inversesqrt(u1);\n"
        "   vec4 v17 = abs(u1);\n"
        "   vec4 v18 = sign(u1);\n"
        "   vec4 v19 = floor(u1);\n"
        "   vec4 v20 = ceil(u1);\n"
        "   vec4 v21 = fract(u1);\n"
        "   vec4 v22 = mod(u1, uf);\n"
        "   vec4 v23 = mod(u1, u2);\n"
        "   vec4 v24 = min(u1, uf);\n"
        "   vec4 v25 = min(u1, u2);\n"
        "   vec4 v26 = max(u1, uf);\n"
        "   vec4 v27 = max(u1, u2);\n"
        "   vec4 v28 = clamp(u1, u2, u3);\n"
        "   vec4 v29 = clamp(u1, uf, uf2);\n"
        "   vec4 v30 = mix(u1, u2, u3);\n"
        "   vec4 v31 = mix(u1, u2, uf);\n"
        "   vec4 v32 = step(u1, u2);\n"
        "   vec4 v33 = step(uf, u1);\n"
        "   vec4 v34 = smoothstep(u1, u2, u3);\n"
        "   vec4 v35 = smoothstep(uf, uf2, u1);\n"
        "   vec4 v36 = normalize(u1);\n"
        "   vec4 v37 = faceforward(u1, u2, u3);\n"
        "   vec4 v38 = reflect(u1, u2);\n"
        "   vec4 v39 = refract(u1, u2, uf);\n"

        "   float f1 = length(u1);\n"
        "   float f2 = distance(u1, u2);\n"
        "   float f3 = dot(u1, u2);\n"
        "   vec3 vf31 = cross(uf31, uf32);\n"
        "   mat4 m1 = matrixCompMult(um1, um2);\n"

        "   gl_FragColor = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +"
            "v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +"
            "v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30 +"
            "v31 + v32 + v33 + v34 + v35 + v36 + v37 + v38 + v39 +"
            "vec4(f1, f2, f3, 0.0) + vec4(vf31, 0.0) + m1[0];\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("v1 = angle_frm(radians(angle_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v2 = angle_frm(degrees(angle_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v3 = angle_frm(sin(angle_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v4 = angle_frm(cos(angle_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v5 = angle_frm(tan(angle_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v6 = angle_frm(asin(angle_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v7 = angle_frm(acos(angle_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v8 = angle_frm(atan(angle_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v9 = angle_frm(atan(angle_frm(u1), angle_frm(u2)))"));
    ASSERT_TRUE(foundInCode("v10 = angle_frm(pow(angle_frm(u1), angle_frm(u2)))"));
    ASSERT_TRUE(foundInCode("v11 = angle_frm(exp(angle_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v12 = angle_frm(log(angle_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v13 = angle_frm(exp2(angle_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v14 = angle_frm(log2(angle_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v15 = angle_frm(sqrt(angle_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v16 = angle_frm(inversesqrt(angle_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v17 = angle_frm(abs(angle_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v18 = angle_frm(sign(angle_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v19 = angle_frm(floor(angle_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v20 = angle_frm(ceil(angle_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v21 = angle_frm(fract(angle_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v22 = angle_frm(mod(angle_frm(u1), angle_frm(uf)))"));
    ASSERT_TRUE(foundInCode("v23 = angle_frm(mod(angle_frm(u1), angle_frm(u2)))"));
    ASSERT_TRUE(foundInCode("v24 = angle_frm(min(angle_frm(u1), angle_frm(uf)))"));
    ASSERT_TRUE(foundInCode("v25 = angle_frm(min(angle_frm(u1), angle_frm(u2)))"));
    ASSERT_TRUE(foundInCode("v26 = angle_frm(max(angle_frm(u1), angle_frm(uf)))"));
    ASSERT_TRUE(foundInCode("v27 = angle_frm(max(angle_frm(u1), angle_frm(u2)))"));
    ASSERT_TRUE(foundInCode("v28 = angle_frm(clamp(angle_frm(u1), angle_frm(u2), angle_frm(u3)))"));
    ASSERT_TRUE(foundInCode("v29 = angle_frm(clamp(angle_frm(u1), angle_frm(uf), angle_frm(uf2)))"));
    ASSERT_TRUE(foundInCode("v30 = angle_frm(mix(angle_frm(u1), angle_frm(u2), angle_frm(u3)))"));
    ASSERT_TRUE(foundInCode("v31 = angle_frm(mix(angle_frm(u1), angle_frm(u2), angle_frm(uf)))"));
    ASSERT_TRUE(foundInCode("v32 = angle_frm(step(angle_frm(u1), angle_frm(u2)))"));
    ASSERT_TRUE(foundInCode("v33 = angle_frm(step(angle_frm(uf), angle_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v34 = angle_frm(smoothstep(angle_frm(u1), angle_frm(u2), angle_frm(u3)))"));
    ASSERT_TRUE(foundInCode("v35 = angle_frm(smoothstep(angle_frm(uf), angle_frm(uf2), angle_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v36 = angle_frm(normalize(angle_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v37 = angle_frm(faceforward(angle_frm(u1), angle_frm(u2), angle_frm(u3)))"));
    ASSERT_TRUE(foundInCode("v38 = angle_frm(reflect(angle_frm(u1), angle_frm(u2)))"));
    ASSERT_TRUE(foundInCode("v39 = angle_frm(refract(angle_frm(u1), angle_frm(u2), angle_frm(uf)))"));

    ASSERT_TRUE(foundInCode("f1 = angle_frm(length(angle_frm(u1)))"));
    ASSERT_TRUE(foundInCode("f2 = angle_frm(distance(angle_frm(u1), angle_frm(u2)))"));
    ASSERT_TRUE(foundInCode("f3 = angle_frm(dot(angle_frm(u1), angle_frm(u2)))"));
    ASSERT_TRUE(foundInCode("vf31 = angle_frm(cross(angle_frm(uf31), angle_frm(uf32)))"));
    ASSERT_TRUE(foundInCode("m1 = angle_frm(matrixCompMult(angle_frm(um1), angle_frm(um2)))"));
}

TEST_F(DebugShaderPrecisionTest, BuiltInRelationalFunctionRounding)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform vec4 u1;\n"
        "uniform vec4 u2;\n"
        "void main() {\n"
        "   bvec4 bv1 = lessThan(u1, u2);\n"
        "   bvec4 bv2 = lessThanEqual(u1, u2);\n"
        "   bvec4 bv3 = greaterThan(u1, u2);\n"
        "   bvec4 bv4 = greaterThanEqual(u1, u2);\n"
        "   bvec4 bv5 = equal(u1, u2);\n"
        "   bvec4 bv6 = notEqual(u1, u2);\n"
        "   gl_FragColor = vec4(bv1) + vec4(bv2) + vec4(bv3) + vec4(bv4) + vec4(bv5) + vec4(bv6);\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("bv1 = lessThan(angle_frm(u1), angle_frm(u2))"));
    ASSERT_TRUE(foundInCode("bv2 = lessThanEqual(angle_frm(u1), angle_frm(u2))"));
    ASSERT_TRUE(foundInCode("bv3 = greaterThan(angle_frm(u1), angle_frm(u2))"));
    ASSERT_TRUE(foundInCode("bv4 = greaterThanEqual(angle_frm(u1), angle_frm(u2))"));
    ASSERT_TRUE(foundInCode("bv5 = equal(angle_frm(u1), angle_frm(u2))"));
    ASSERT_TRUE(foundInCode("bv6 = notEqual(angle_frm(u1), angle_frm(u2))"));
}

TEST_F(DebugShaderPrecisionTest, ConstructorRounding)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "precision mediump int;\n"
        "uniform float u1;\n"
        "uniform float u2;\n"
        "uniform float u3;\n"
        "uniform float u4;\n"
        "uniform ivec4 uiv;\n"
        "void main() {\n"
        "   vec4 v1 = vec4(u1, u2, u3, u4);\n"
        "   vec4 v2 = vec4(uiv);\n"
        "   gl_FragColor = v1 + v2;\n"
        "}\n";
    compile(shaderString);
    // Note: this is suboptimal for the case taking four floats, but optimizing would be tricky.
    ASSERT_TRUE(foundInCode("v1 = angle_frm(vec4(angle_frm(u1), angle_frm(u2), angle_frm(u3), angle_frm(u4)))"));
    ASSERT_TRUE(foundInCode("v2 = angle_frm(vec4(uiv))"));
}

TEST_F(DebugShaderPrecisionTest, StructConstructorNoRounding)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "struct S { mediump vec4 a; };\n"
        "uniform vec4 u;\n"
        "void main() {\n"
        "   S s = S(u);\n"
        "   gl_FragColor = s.a;\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("s = S(angle_frm(u))"));
    ASSERT_TRUE(notFoundInCode("angle_frm(S"));
}

TEST_F(DebugShaderPrecisionTest, SwizzleRounding)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform vec4 u;\n"
        "void main() {\n"
        "   vec4 v = u.xyxy;"
        "   gl_FragColor = v;\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("v = angle_frm(u).xyxy"));
}

TEST_F(DebugShaderPrecisionTest, BuiltInTexFunctionRounding)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "precision lowp sampler2D;\n"
        "uniform vec2 u;\n"
        "uniform sampler2D s;\n"
        "void main() {\n"
        "   lowp vec4 v = texture2D(s, u);\n"
        "   gl_FragColor = v;\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("v = angle_frl(texture2D(s, angle_frm(u)))"));
}

TEST_F(DebugShaderPrecisionTest, FunctionCallParameterQualifiersFromDefinition)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform vec4 u1;\n"
        "uniform vec4 u2;\n"
        "uniform vec4 u3;\n"
        "uniform vec4 u4;\n"
        "uniform vec4 u5;\n"
        "vec4 add(in vec4 x, in vec4 y) {\n"
        "   return x + y;\n"
        "}\n"
        "void compound_add(inout vec4 x, in vec4 y) {\n"
        "   x = x + y;\n"
        "}\n"
        "void add_to_last(in vec4 x, in vec4 y, out vec4 z) {\n"
        "   z = x + y;\n"
        "}\n"
        "void main() {\n"
        "   vec4 v = add(u1, u2);\n"
        "   compound_add(v, u3);\n"
        "   vec4 v2;\n"
        "   add_to_last(u4, u5, v2);\n"
        "   gl_FragColor = v + v2;\n"
        "}\n";
    compile(shaderString);
    // Note that this is not optimal code, there are redundant frm calls.
    // However, getting the implementation working when other operations
    // are nested within function calls would be tricky if to get right
    // otherwise.
    // Test in parameters
    ASSERT_TRUE(foundInCode("v = add(angle_frm(u1), angle_frm(u2))"));
    // Test inout parameter
    ASSERT_TRUE(foundInCode("compound_add(v, angle_frm(u3))"));
    // Test out parameter
    ASSERT_TRUE(foundInCode("add_to_last(angle_frm(u4), angle_frm(u5), v2)"));
}

TEST_F(DebugShaderPrecisionTest, FunctionCallParameterQualifiersFromPrototype)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform vec4 u1;\n"
        "uniform vec4 u2;\n"
        "uniform vec4 u3;\n"
        "uniform vec4 u4;\n"
        "uniform vec4 u5;\n"
        "vec4 add(in vec4 x, in vec4 y);\n"
        "void compound_add(inout vec4 x, in vec4 y);\n"
        "void add_to_last(in vec4 x, in vec4 y, out vec4 z);\n"
        "void main() {\n"
        "   vec4 v = add(u1, u2);\n"
        "   compound_add(v, u3);\n"
        "   vec4 v2;\n"
        "   add_to_last(u4, u5, v2);\n"
        "   gl_FragColor = v + v2;\n"
        "}\n"
        "vec4 add(in vec4 x, in vec4 y) {\n"
        "   return x + y;\n"
        "}\n"
        "void compound_add(inout vec4 x, in vec4 y) {\n"
        "   x = x + y;\n"
        "}\n"
        "void add_to_last(in vec4 x, in vec4 y, out vec4 z) {\n"
        "   z = x + y;\n"
        "}\n";
    compile(shaderString);
    // Test in parameters
    ASSERT_TRUE(foundInCode("v = add(angle_frm(u1), angle_frm(u2))"));
    // Test inout parameter
    ASSERT_TRUE(foundInCode("compound_add(v, angle_frm(u3))"));
    // Test out parameter
    ASSERT_TRUE(foundInCode("add_to_last(angle_frm(u4), angle_frm(u5), v2)"));
}

TEST_F(DebugShaderPrecisionTest, NestedFunctionCalls)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform vec4 u1;\n"
        "uniform vec4 u2;\n"
        "uniform vec4 u3;\n"
        "vec4 add(in vec4 x, in vec4 y) {\n"
        "   return x + y;\n"
        "}\n"
        "vec4 compound_add(inout vec4 x, in vec4 y) {\n"
        "   x = x + y;\n"
        "   return x;\n"
        "}\n"
        "void main() {\n"
        "   vec4 v = u1;\n"
        "   vec4 v2 = add(compound_add(v, u2), fract(u3));\n"
        "   gl_FragColor = v + v2;\n"
        "}\n";
    compile(shaderString);
    // Test nested calls
    ASSERT_TRUE(foundInCode("v2 = add(compound_add(v, angle_frm(u2)), angle_frm(fract(angle_frm(u3))))"));
}

// Test that code inside an index of a function out parameter gets processed.
TEST_F(DebugShaderPrecisionTest, OpInIndexOfFunctionOutParameter)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "void foo(out vec4 f) { f.x = 0.0; }\n"
        "uniform float u2;\n"
        "void main() {\n"
        "   vec4 v[2];\n"
        "   foo(v[int(exp2(u2))]);\n"
        "   gl_FragColor = v[0];\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("angle_frm(exp2(angle_frm(u2)))"));
}

// Test that code inside an index of an l-value gets processed.
TEST_F(DebugShaderPrecisionTest, OpInIndexOfLValue)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform vec4 u1;\n"
        "uniform float u2;\n"
        "void main() {\n"
        "   vec4 v[2];\n"
        "   v[int(exp2(u2))] = u1;\n"
        "   gl_FragColor = v[0];\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("angle_frm(exp2(angle_frm(u2)))"));
}

// Test that the out parameter of modf doesn't get rounded
TEST_F(DebugShaderPrecisionTest, ModfOutParameter)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "uniform float u;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "   float o;\n"
        "   float f = modf(u, o);\n"
        "   my_FragColor = vec4(f, o, 0, 1);\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("modf(angle_frm(u), o)"));
}
