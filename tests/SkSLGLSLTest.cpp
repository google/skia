/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSLCompiler.h"

#include "Test.h"

#if SK_SUPPORT_GPU

static void test(skiatest::Reporter* r, const char* src, const SkSL::Program::Settings& settings,
                 const char* expected, SkSL::Program::Inputs* inputs) {
    SkSL::Compiler compiler;
    SkString output;
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(SkSL::Program::kFragment_Kind,
                                                                     SkString(src),
                                                                     settings);
    if (!program) {
        SkDebugf("Unexpected error compiling %s\n%s", src, compiler.errorText().c_str());
    }
    REPORTER_ASSERT(r, program);
    *inputs = program->fInputs;
    REPORTER_ASSERT(r, compiler.toGLSL(*program, &output));
    if (program) {
        SkString skExpected(expected);
        if (output != skExpected) {
            SkDebugf("GLSL MISMATCH:\nsource:\n%s\n\nexpected:\n'%s'\n\nreceived:\n'%s'", src,
                     expected, output.c_str());
        }
        REPORTER_ASSERT(r, output == skExpected);
    }
}

static void test(skiatest::Reporter* r, const char* src, const GrShaderCaps& caps,
                 const char* expected) {
    SkSL::Program::Settings settings;
    settings.fCaps = &caps;
    SkSL::Program::Inputs inputs;
    test(r, src, settings, expected, &inputs);
}

DEF_TEST(SkSLHelloWorld, r) {
    test(r,
         "void main() { sk_FragColor = vec4(0.75); }",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    sk_FragColor = vec4(0.75);\n"
         "}\n");
}

DEF_TEST(SkSLControl, r) {
    test(r,
         "void main() {"
         "if (sqrt(2) > 5) { sk_FragColor = vec4(0.75); } else { discard; }"
         "int i = 0;"
         "while (i < 10) sk_FragColor *= 0.5;"
         "do { sk_FragColor += 0.01; } while (sk_FragColor.x < 0.75);"
         "for (int i = 0; i < 10; i++) {"
         "if (i % 0 == 1) break; else continue;"
         "}"
         "return;"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    if (sqrt(2.0) > 5.0) {\n"
         "        sk_FragColor = vec4(0.75);\n"
         "    } else {\n"
         "        discard;\n"
         "    }\n"
         "    int i = 0;\n"
         "    while (i < 10) sk_FragColor *= 0.5;\n"
         "    do {\n"
         "        sk_FragColor += 0.01;\n"
         "    } while (sk_FragColor.x < 0.75);\n"
         "    for (int i = 0;i < 10; i++) {\n"
         "        if (i % 0 == 1) break; else continue;\n"
         "    }\n"
         "    return;\n"
         "}\n");
}

DEF_TEST(SkSLFunctions, r) {
    test(r,
         "float foo(float v[2]) { return v[0] * v[1]; }"
         "void bar(inout float x) { float y[2], z; y[0] = x; y[1] = x * 2; z = foo(y); x = z; }"
         "void main() { float x = 10; bar(x); sk_FragColor = vec4(x); }",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "float foo(in float v[2]) {\n"
         "    return v[0] * v[1];\n"
         "}\n"
         "void bar(inout float x) {\n"
         "    float y[2], z;\n"
         "    y[0] = x;\n"
         "    y[1] = x * 2.0;\n"
         "    z = foo(y);\n"
         "    x = z;\n"
         "}\n"
         "void main() {\n"
         "    float x = 10.0;\n"
         "    bar(x);\n"
         "    sk_FragColor = vec4(x);\n"
         "}\n");
}

DEF_TEST(SkSLOperators, r) {
    test(r,
         "void main() {"
         "float x = 1, y = 2;"
         "int z = 3;"
         "x = x + y * z * x * (y - z);"
         "y = x / y / z;"
         "z = (z / 2 % 3 << 4) >> 2 << 1;"
         "bool b = (x > 4) == x < 2 || 2 >= sqrt(2) && y <= z;"
         "x += 12;"
         "x -= 12;"
         "x *= y /= z = 10;"
         "b ||= false;"
         "b &&= true;"
         "b ^^= false;"
         "z |= 0;"
         "z &= -1;"
         "z ^= 0;"
         "z >>= 2;"
         "z <<= 4;"
         "z %= 5;"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    float x = 1.0, y = 2.0;\n"
         "    int z = 3;\n"
         "    x = x + ((y * float(z)) * x) * (y - float(z));\n"
         "    y = (x / y) / float(z);\n"
         "    z = (((z / 2) % 3 << 4) >> 2) << 1;\n"
         "    bool b = x > 4.0 == x < 2.0 || 2.0 >= sqrt(2.0) && y <= float(z);\n"
         "    x += 12.0;\n"
         "    x -= 12.0;\n"
         "    x *= (y /= float(z = 10));\n"
         "    b ||= false;\n"
         "    b &&= true;\n"
         "    b ^^= false;\n"
         "    z |= 0;\n"
         "    z &= -1;\n"
         "    z ^= 0;\n"
         "    z >>= 2;\n"
         "    z <<= 4;\n"
         "    z %= 5;\n"
         "}\n");
}

DEF_TEST(SkSLMatrices, r) {
    test(r,
         "void main() {"
         "mat2x4 x = mat2x4(1);"
         "mat3x2 y = mat3x2(1, 0, 0, 1, vec2(2, 2));"
         "mat3x4 z = x * y;"
         "vec3 v1 = mat3(1) * vec3(1);"
         "vec3 v2 = vec3(1) * mat3(1);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    mat2x4 x = mat2x4(1.0);\n"
         "    mat3x2 y = mat3x2(1.0, 0.0, 0.0, 1.0, vec2(2.0, 2.0));\n"
         "    mat3x4 z = x * y;\n"
         "    vec3 v1 = mat3(1.0) * vec3(1.0);\n"
         "    vec3 v2 = vec3(1.0) * mat3(1.0);\n"
         "}\n");
}

DEF_TEST(SkSLInterfaceBlock, r) {
    test(r,
         "uniform testBlock {"
         "float x;"
         "float y[2];"
         "layout(binding=12) mat3x2 z;"
         "bool w;"
         "};"
         "void main() {"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "uniform testBlock {\n"
         "    float x;\n"
         "    float[2] y;\n"
         "    layout (binding = 12) mat3x2 z;\n"
         "    bool w;\n"
         "};\n"
         "void main() {\n"
         "}\n");
}

DEF_TEST(SkSLStructs, r) {
    test(r,
         "struct A {"
         "int x;"
         "int y;"
         "} a1, a2;"
         "A a3;"
         "struct B {"
         "float x;"
         "float y[2];"
         "layout(binding=1) A z;"
         "};"
         "B b1, b2, b3;"
         "void main() {"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "struct A {\n"
         "    int x;\n"
         "    int y;\n"
         "} a1, a2;\n"
         "A a3;\n"
         "struct B {\n"
         "    float x;\n"
         "    float[2] y;\n"
         "    layout (binding = 1) A z;\n"
         "} b1, b2, b3;\n"
         "void main() {\n"
         "}\n");
}

DEF_TEST(SkSLVersion, r) {
    test(r,
         "in float test; void main() { sk_FragColor = vec4(0.75); }",
         *SkSL::ShaderCapsFactory::Version450Core(),
         "#version 450 core\n"
         "out vec4 sk_FragColor;\n"
         "in float test;\n"
         "void main() {\n"
         "    sk_FragColor = vec4(0.75);\n"
         "}\n");
    test(r,
         "in float test; void main() { sk_FragColor = vec4(0.75); }",
         *SkSL::ShaderCapsFactory::Version110(),
         "#version 110\n"
         "varying float test;\n"
         "void main() {\n"
         "    gl_FragColor = vec4(0.75);\n"
         "}\n");
}

DEF_TEST(SkSLUsesPrecisionModifiers, r) {
    test(r,
         "void main() { float x = 0.75; highp float y = 1; }",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    float x = 0.75;\n"
         "    float y = 1.0;\n"
         "}\n");    
    test(r,
         "void main() { float x = 0.75; highp float y = 1; }",
         *SkSL::ShaderCapsFactory::UsesPrecisionModifiers(),
         "#version 400\n"
         "precision highp float;\n"
         "out mediump vec4 sk_FragColor;\n"
         "void main() {\n"
         "    float x = 0.75;\n"
         "    highp float y = 1.0;\n"
         "}\n");    
}

DEF_TEST(SkSLMinAbs, r) {
    test(r,
         "void main() {"
         "float x = -5;"
         "x = min(abs(x), 6);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    float x = -5.0;\n"
         "    x = min(abs(x), 6.0);\n"
         "}\n");

    test(r,
         "void main() {"
         "float x = -5.0;"
         "x = min(abs(x), 6.0);"
         "}",
         *SkSL::ShaderCapsFactory::CannotUseMinAndAbsTogether(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    float minAbsHackVar0;\n"
         "    float minAbsHackVar1;\n"
         "    float x = -5.0;\n"
         "    x = ((minAbsHackVar0 = abs(x)) < (minAbsHackVar1 = 6.0) ? minAbsHackVar0 : "
                                                                                "minAbsHackVar1);\n"
         "}\n");
}

DEF_TEST(SkSLNegatedAtan, r) {
    test(r,
         "void main() { vec2 x = vec2(1, 2); float y = atan(x.x, -(2 * x.y)); }",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    vec2 x = vec2(1.0, 2.0);\n"
         "    float y = atan(x.x, -(2.0 * x.y));\n"
         "}\n");
    test(r,
         "void main() { vec2 x = vec2(1, 2); float y = atan(x.x, -(2 * x.y)); }",
         *SkSL::ShaderCapsFactory::MustForceNegatedAtanParamToFloat(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    vec2 x = vec2(1.0, 2.0);\n"
         "    float y = atan(x.x, -1.0 * (2.0 * x.y));\n"
         "}\n");
}

DEF_TEST(SkSLModifiersDeclaration, r) {
    test(r,
         "layout(blend_support_all_equations) out;"
         "void main() { }",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "layout (blend_support_all_equations) out ;\n"
         "void main() {\n"
         "}\n");
}

DEF_TEST(SkSLHex, r) {
    test(r,
         "void main() {"
         "int i1 = 0x0;"
         "int i2 = 0x1234abcd;"
         "int i3 = 0x7fffffff;"
         "int i4 = 0xffffffff;"
         "int i5 = -0xbeef;"
         "uint u1 = 0x0;"
         "uint u2 = 0x1234abcd;"
         "uint u3 = 0x7fffffff;"
         "uint u4 = 0xffffffff;"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    int i1 = 0;\n"
         "    int i2 = 305441741;\n"
         "    int i3 = 2147483647;\n"
         "    int i4 = -1;\n"
         "    int i5 = -48879;\n"
         "    uint u1 = 0u;\n"
         "    uint u2 = 305441741u;\n"
         "    uint u3 = 2147483647u;\n"
         "    uint u4 = 4294967295u;\n"
         "}\n");
}

DEF_TEST(SkSLVectorConstructors, r) {
    test(r,
         "vec2 v1 = vec2(1);"
         "vec2 v2 = vec2(1, 2);"
         "vec2 v3 = vec2(vec2(1));"
         "vec2 v4 = vec2(vec3(1));"
         "vec3 v5 = vec3(vec2(1), 1.0);"
         "vec3 v6 = vec3(vec4(1, 2, 3, 4));"
         "ivec2 v7 = ivec2(1);"
         "ivec2 v8 = ivec2(vec2(1, 2));"
         "vec2 v9 = vec2(ivec2(1, 2));",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "vec2 v1 = vec2(1.0);\n"
         "vec2 v2 = vec2(1.0, 2.0);\n"
         "vec2 v3 = vec2(1.0);\n"
         "vec2 v4 = vec2(vec3(1.0));\n"
         "vec3 v5 = vec3(vec2(1.0), 1.0);\n"
         "vec3 v6 = vec3(vec4(1.0, 2.0, 3.0, 4.0));\n"
         "ivec2 v7 = ivec2(1);\n"
         "ivec2 v8 = ivec2(vec2(1.0, 2.0));\n"
         "vec2 v9 = vec2(ivec2(1, 2));\n");
}

DEF_TEST(SkSLArrayConstructors, r) {
    test(r,
         "float test1[] = float[](1, 2, 3, 4);"
         "vec2 test2[] = vec2[](vec2(1, 2), vec2(3, 4));"
         "mat4 test3[] = mat4[]();",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "float test1[] = float[](1.0, 2.0, 3.0, 4.0);\n"
         "vec2 test2[] = vec2[](vec2(1.0, 2.0), vec2(3.0, 4.0));\n"
         "mat4 test3[] = mat4[]();\n");
}

DEF_TEST(SkSLDerivatives, r) {
    test(r,
         "void main() { float x = dFdx(1); }",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    float x = dFdx(1.0);\n"
         "}\n");
    test(r,
         "void main() { float x = 1; }",
         *SkSL::ShaderCapsFactory::ShaderDerivativeExtensionString(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    float x = 1.0;\n"
         "}\n");
    test(r,
         "void main() { float x = dFdx(1); }",
         *SkSL::ShaderCapsFactory::ShaderDerivativeExtensionString(),
         "#version 400\n"
         "#extension GL_OES_standard_derivatives : require\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    float x = dFdx(1.0);\n"
         "}\n");
}

DEF_TEST(SkSLConstantFolding, r) {
    test(r,
         "void main() {"
         "float f_add = 32 + 2;"
         "float f_sub = 32 - 2;"
         "float f_mul = 32 * 2;"
         "float f_div = 32 / 2;"
         "float mixed = (12 > 2.0) ? (10 * 2 / 5 + 18 - 3) : 0;"
         "int i_add = 32 + 2;"
         "int i_sub = 32 - 2;"
         "int i_mul = 32 * 2;"
         "int i_div = 32 / 2;"
         "int i_or = 12 | 6;"
         "int i_and = 254 & 7;"
         "int i_xor = 2 ^ 7;"
         "int i_shl = 1 << 4;"
         "int i_shr = 128 >> 2;"
         "bool gt_it = 6 > 5;"
         "bool gt_if = 6 > 6;"
         "bool gt_ft = 6.0 > 5.0;"
         "bool gt_ff = 6.0 > 6.0;"
         "bool gte_it = 6 >= 6;"
         "bool gte_if = 6 >= 7;"
         "bool gte_ft = 6.0 >= 6.0;"
         "bool gte_ff = 6.0 >= 7.0;"
         "bool lte_it = 6 <= 6;"
         "bool lte_if = 6 <= 5;"
         "bool lte_ft = 6.0 <= 6.0;"
         "bool lte_ff = 6.0 <= 5.0;"
         "bool or_t = 1 == 1 || 2 == 8;"
         "bool or_f = 1 > 1 || 2 == 8;"
         "bool and_t = 1 == 1 && 2 <= 8;"
         "bool and_f = 1 == 2 && 2 == 8;"
         "bool xor_t = 1 == 1 ^^ 1 != 1;"
         "bool xor_f = 1 == 1 ^^ 1 == 1;"
         "int ternary = 10 > 5 ? 10 : 5;"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    float f_add = 34.0;\n"
         "    float f_sub = 30.0;\n"
         "    float f_mul = 64.0;\n"
         "    float f_div = 16.0;\n"
         "    float mixed = 19.0;\n"
         "    int i_add = 34;\n"
         "    int i_sub = 30;\n"
         "    int i_mul = 64;\n"
         "    int i_div = 16;\n"
         "    int i_or = 14;\n"
         "    int i_and = 6;\n"
         "    int i_xor = 5;\n"
         "    int i_shl = 16;\n"
         "    int i_shr = 32;\n"
         "    bool gt_it = true;\n"
         "    bool gt_if = false;\n"
         "    bool gt_ft = true;\n"
         "    bool gt_ff = false;\n"
         "    bool gte_it = true;\n"
         "    bool gte_if = false;\n"
         "    bool gte_ft = true;\n"
         "    bool gte_ff = false;\n"
         "    bool lte_it = true;\n"
         "    bool lte_if = false;\n"
         "    bool lte_ft = true;\n"
         "    bool lte_ff = false;\n"
         "    bool or_t = true;\n"
         "    bool or_f = false;\n"
         "    bool and_t = true;\n"
         "    bool and_f = false;\n"
         "    bool xor_t = true;\n"
         "    bool xor_f = false;\n"
         "    int ternary = 10;\n"
         "}\n");
}

DEF_TEST(SkSLStaticIf, r) {
    test(r,
         "void main() {"
         "int x;"
         "if (true) x = 1;"
         "if (2 > 1) x = 2; else x = 3;"
         "if (1 > 2) x = 4; else x = 5;"
         "if (false) x = 6;"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    int x;\n"
         "    x = 1;\n"
         "    x = 2;\n"
         "    x = 5;\n"
         "    {\n"
         "    }\n"
         "}\n");
}

DEF_TEST(SkSLCaps, r) {
    test(r,
         "void main() {"
         "int x;"
         "if (sk_Caps.externalTextureSupport) x = 1;"
         "if (sk_Caps.fbFetchSupport) x = 2;"
         "if (sk_Caps.dropsTileOnZeroDivide && sk_Caps.texelFetchSupport) x = 3;"
         "if (sk_Caps.dropsTileOnZeroDivide && sk_Caps.canUseAnyFunctionInShader) x = 4;"
         "}",
         *SkSL::ShaderCapsFactory::VariousCaps(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    int x;\n"
         "    x = 1;\n"
         "    {\n"
         "    }\n"
         "    x = 3;\n"
         "    {\n"
         "    }\n"
         "}\n");
}

DEF_TEST(SkSLTexture, r) {
    test(r,
         "uniform sampler1D one;"
         "uniform sampler2D two;"
         "void main() {"
         "vec4 a = texture(one, 0);"
         "vec4 b = texture(two, vec2(0));"
         "vec4 c = texture(one, vec2(0));"
         "vec4 d = texture(two, vec3(0));"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "uniform sampler1D one;\n"
         "uniform sampler2D two;\n"
         "void main() {\n"
         "    vec4 a = texture(one, 0.0);\n"
         "    vec4 b = texture(two, vec2(0.0));\n"
         "    vec4 c = textureProj(one, vec2(0.0));\n"
         "    vec4 d = textureProj(two, vec3(0.0));\n"
         "}\n");
    test(r,
         "uniform sampler1D one;"
         "uniform sampler2D two;"
         "void main() {"
         "vec4 a = texture(one, 0);"
         "vec4 b = texture(two, vec2(0));"
         "vec4 c = texture(one, vec2(0));"
         "vec4 d = texture(two, vec3(0));"
         "}",
         *SkSL::ShaderCapsFactory::Version110(),
         "#version 110\n"
         "uniform sampler1D one;\n"
         "uniform sampler2D two;\n"
         "void main() {\n"
         "    vec4 a = texture1D(one, 0.0);\n"
         "    vec4 b = texture2D(two, vec2(0.0));\n"
         "    vec4 c = texture1DProj(one, vec2(0.0));\n"
         "    vec4 d = texture2DProj(two, vec3(0.0));\n"
         "}\n");
}

DEF_TEST(SkSLOffset, r) {
    test(r,
         "struct Test {"
         "layout(offset = 0) int x;"
         "layout(offset = 4) int y;"
         "int z;"
         "} test;",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "struct Test {\n"
         "    layout (offset = 0) int x;\n"
         "    layout (offset = 4) int y;\n"
         "    int z;\n"
         "} test;\n");
}

DEF_TEST(SkSLFragCoord, r) {
    SkSL::Program::Settings settings;
    settings.fFlipY = true;
    sk_sp<GrShaderCaps> caps = SkSL::ShaderCapsFactory::FragCoordsOld();
    settings.fCaps = caps.get();
    SkSL::Program::Inputs inputs;
    test(r,
         "void main() { sk_FragColor.xy = sk_FragCoord.xy; }",
         settings,
         "#version 110\n"
         "#extension GL_ARB_fragment_coord_conventions : require\n"
         "layout(origin_upper_left) in vec4 gl_FragCoord;\n"
         "void main() {\n"
         "    gl_FragColor.xy = gl_FragCoord.xy;\n"
         "}\n",
         &inputs);
    REPORTER_ASSERT(r, !inputs.fRTHeight);

    caps = SkSL::ShaderCapsFactory::FragCoordsNew();
    settings.fCaps = caps.get();
    test(r,
         "void main() { sk_FragColor.xy = sk_FragCoord.xy; }",
         settings,
         "#version 400\n"
         "layout(origin_upper_left) in vec4 gl_FragCoord;\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    sk_FragColor.xy = gl_FragCoord.xy;\n"
         "}\n",
         &inputs);
    REPORTER_ASSERT(r, !inputs.fRTHeight);

    caps = SkSL::ShaderCapsFactory::Default();
    settings.fCaps = caps.get();
    test(r,
         "void main() { sk_FragColor.xy = sk_FragCoord.xy; }",
         settings,
         "#version 400\n"
         "uniform float u_skRTHeight;\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    vec2 _sktmpCoord = gl_FragCoord.xy;\n"
         "    vec4 sk_FragCoord = vec4(_sktmpCoord.x, u_skRTHeight - _sktmpCoord.y, 1.0, 1.0);\n"
         "    sk_FragColor.xy = sk_FragCoord.xy;\n"
         "}\n",
         &inputs);
    REPORTER_ASSERT(r, inputs.fRTHeight);

    settings.fFlipY = false;
    test(r,
         "void main() { sk_FragColor.xy = sk_FragCoord.xy; }",
         settings,
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    sk_FragColor.xy = gl_FragCoord.xy;\n"
         "}\n",
         &inputs);
    REPORTER_ASSERT(r, !inputs.fRTHeight);
}

#endif
