/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSLCompiler.h"

#include "Test.h"

#if SK_SUPPORT_GPU

// Note that the optimizer will aggressively kill dead code and substitute constants in place of
// variables, so we have to jump through a few hoops to ensure that the code in these tests has the
// necessary side-effects to remain live. In some cases we rely on the optimizer not (yet) being
// smart enough to optimize around certain constructs; as the optimizer gets smarter it will
// undoubtedly end up breaking some of these tests. That is a good thing, as long as the new code is
// equivalent!

static void test(skiatest::Reporter* r, const char* src, const SkSL::Program::Settings& settings,
                 const char* expected, SkSL::Program::Inputs* inputs,
                 SkSL::Program::Kind kind = SkSL::Program::kFragment_Kind) {
    SkSL::Compiler compiler;
    SkSL::String output;
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(kind, SkString(src), settings);
    if (!program) {
        SkDebugf("Unexpected error compiling %s\n%s", src, compiler.errorText().c_str());
    }
    REPORTER_ASSERT(r, program);
    *inputs = program->fInputs;
    REPORTER_ASSERT(r, compiler.toGLSL(*program, &output));
    if (program) {
        SkSL::String skExpected(expected);
        if (output != skExpected) {
            SkDebugf("GLSL MISMATCH:\nsource:\n%s\n\nexpected:\n'%s'\n\nreceived:\n'%s'", src,
                     expected, output.c_str());
        }
        REPORTER_ASSERT(r, output == skExpected);
    }
}

static void test(skiatest::Reporter* r, const char* src, const GrShaderCaps& caps,
                 const char* expected, SkSL::Program::Kind kind = SkSL::Program::kFragment_Kind) {
    SkSL::Program::Settings settings;
    settings.fCaps = &caps;
    SkSL::Program::Inputs inputs;
    test(r, src, settings, expected, &inputs, kind);
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
         "while (i < 10) { sk_FragColor *= 0.5; i++; }"
         "do { sk_FragColor += 0.01; } while (sk_FragColor.x < 0.75);"
         "for (int i = 0; i < 10; i++) {"
         "if (i % 2 == 1) break; else continue;"
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
         "    while (i < 10) {\n"
         "        sk_FragColor *= 0.5;\n"
         "        i++;\n"
         "    }\n"
         "    do {\n"
         "        sk_FragColor += 0.01;\n"
         "    } while (sk_FragColor.x < 0.75);\n"
         "    for (int i = 0;i < 10; i++) {\n"
         "        if (i % 2 == 1) break; else continue;\n"
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
         "float foo(float v[2]) {\n"
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
         "x = x - x + y * z * x * (y - z);"
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
         "x = (vec2(sqrt(1)) , 6);"
         "z = (vec2(sqrt(1)) , 6);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    float x = 1.0, y = 2.0;\n"
         "    int z = 3;\n"
         "    x = -6.0;\n"
         "    y = -1.0;\n"
         "    z = 8;\n"
         "    bool b = false == true || 2.0 >= sqrt(2.0) && true;\n"
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
         "    x = float((vec2(sqrt(1.0)) , 6));\n"
         "    z = (vec2(sqrt(1.0)) , 6);\n"
         "}\n");
}

DEF_TEST(SkSLMatrices, r) {
    test(r,
         "void main() {"
         "mat2x4 x = mat2x4(1);"
         "mat3x2 y = mat3x2(1, 0, 0, 1, vec2(2, 2));"
         "mat3x4 z = x * y;"
         "vec3 v1 = mat3(1) * vec3(2);"
         "vec3 v2 = vec3(2) * mat3(1);"
         "sk_FragColor = vec4(z[0].x, v1 + v2);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    mat3x4 z = mat2x4(1.0) * mat3x2(1.0, 0.0, 0.0, 1.0, vec2(2.0, 2.0));\n"
         "    vec3 v1 = mat3(1.0) * vec3(2.0);\n"
         "    vec3 v2 = vec3(2.0) * mat3(1.0);\n"
         "    sk_FragColor = vec4(z[0].x, v1 + v2);\n"
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
         "    sk_FragColor = vec4(x, y[0], y[1], 0);"
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
         "    sk_FragColor = vec4(x, y[0], y[1], 0.0);\n"
         "}\n");
    test(r,
         "uniform testBlock {"
         "float x;"
         "} test;"
         "void main() {"
         "    sk_FragColor = vec4(test.x);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "uniform testBlock {\n"
         "    float x;\n"
         "} test;\n"
         "void main() {\n"
         "    sk_FragColor = vec4(test.x);\n"
         "}\n");
    test(r,
         "uniform testBlock {"
         "float x;"
         "} test[2];"
         "void main() {"
         "    sk_FragColor = vec4(test[1].x);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "uniform testBlock {\n"
         "    float x;\n"
         "} test[2];\n"
         "void main() {\n"
         "    sk_FragColor = vec4(test[1].x);\n"
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
         "void main() { float x = 0.75; highp float y = 1; x++; y++;"
         "sk_FragColor.rg = vec2(x, y); }",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    float x = 0.75;\n"
         "    float y = 1.0;\n"
         "    x++;\n"
         "    y++;\n"
         "    sk_FragColor.xy = vec2(x, y);\n"
         "}\n");
    test(r,
         "void main() { float x = 0.75; highp float y = 1; x++; y++;"
         "sk_FragColor.rg = vec2(x, y); }",
         *SkSL::ShaderCapsFactory::UsesPrecisionModifiers(),
         "#version 400\n"
         "precision highp float;\n"
         "out mediump vec4 sk_FragColor;\n"
         "void main() {\n"
         "    float x = 0.75;\n"
         "    highp float y = 1.0;\n"
         "    x++;\n"
         "    y++;\n"
         "    sk_FragColor.xy = vec2(x, y);\n"
         "}\n");
}

DEF_TEST(SkSLMinAbs, r) {
    test(r,
         "void main() {"
         "float x = -5;"
         "sk_FragColor.r = min(abs(x), 6);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    sk_FragColor.x = min(abs(-5.0), 6.0);\n"
         "}\n");

    test(r,
         "void main() {"
         "float x = -5.0;"
         "sk_FragColor.r = min(abs(x), 6.0);"
         "}",
         *SkSL::ShaderCapsFactory::CannotUseMinAndAbsTogether(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    float minAbsHackVar0;\n"
         "    float minAbsHackVar1;\n"
         "    sk_FragColor.x = ((minAbsHackVar0 = abs(-5.0)) < (minAbsHackVar1 = 6.0) ? "
                                                               "minAbsHackVar0 : minAbsHackVar1);\n"
         "}\n");
}

DEF_TEST(SkSLNegatedAtan, r) {
    test(r,
         "void main() { vec2 x = vec2(sqrt(2)); sk_FragColor.r = atan(x.x, -x.y); }",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    vec2 x = vec2(sqrt(2.0));\n"
         "    sk_FragColor.x = atan(x.x, -x.y);\n"
         "}\n");
    test(r,
         "void main() { vec2 x = vec2(sqrt(2)); sk_FragColor.r = atan(x.x, -x.y); }",
         *SkSL::ShaderCapsFactory::MustForceNegatedAtanParamToFloat(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    vec2 x = vec2(sqrt(2.0));\n"
         "    sk_FragColor.x = atan(x.x, -1.0 * x.y);\n"
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
         "i1++;"
         "int i2 = 0x1234abcd;"
         "i2++;"
         "int i3 = 0x7fffffff;"
         "i3++;"
         "int i4 = 0xffffffff;"
         "i4++;"
         "int i5 = -0xbeef;"
         "i5++;"
         "uint u1 = 0x0;"
         "u1++;"
         "uint u2 = 0x1234abcd;"
         "u2++;"
         "uint u3 = 0x7fffffff;"
         "u3++;"
         "uint u4 = 0xffffffff;"
         "u4++;"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    int i1 = 0;\n"
         "    i1++;\n"
         "    int i2 = 305441741;\n"
         "    i2++;\n"
         "    int i3 = 2147483647;\n"
         "    i3++;\n"
         "    int i4 = -1;\n"
         "    i4++;\n"
         "    int i5 = -48879;\n"
         "    i5++;\n"
         "    uint u1 = 0u;\n"
         "    u1++;\n"
         "    uint u2 = 305441741u;\n"
         "    u2++;\n"
         "    uint u3 = 2147483647u;\n"
         "    u3++;\n"
         "    uint u4 = 4294967295u;\n"
         "    u4++;\n"
         "}\n");
}

DEF_TEST(SkSLVectorConstructors, r) {
    test(r,
         "vec2 v1 = vec2(1);"
         "vec2 v2 = vec2(1, 2);"
         "vec2 v3 = vec2(vec2(1));"
         "vec3 v4 = vec3(vec2(1), 1.0);"
         "ivec2 v5 = ivec2(1);"
         "ivec2 v6 = ivec2(vec2(1, 2));"
         "vec2 v7 = vec2(ivec2(1, 2));",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "vec2 v1 = vec2(1.0);\n"
         "vec2 v2 = vec2(1.0, 2.0);\n"
         "vec2 v3 = vec2(1.0);\n"
         "vec3 v4 = vec3(vec2(1.0), 1.0);\n"
         "ivec2 v5 = ivec2(1);\n"
         "ivec2 v6 = ivec2(vec2(1.0, 2.0));\n"
         "vec2 v7 = vec2(ivec2(1, 2));\n");
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
         "void main() { sk_FragColor.r = dFdx(1); }",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    sk_FragColor.x = dFdx(1.0);\n"
         "}\n");
    test(r,
         "void main() { sk_FragColor.r = 1; }",
         *SkSL::ShaderCapsFactory::ShaderDerivativeExtensionString(),
         "#version 400\n"
         "precision highp float;\n"
         "out mediump vec4 sk_FragColor;\n"
         "void main() {\n"
         "    sk_FragColor.x = 1.0;\n"
         "}\n");
    test(r,
         "void main() { sk_FragColor.r = dFdx(1); }",
         *SkSL::ShaderCapsFactory::ShaderDerivativeExtensionString(),
         "#version 400\n"
         "#extension GL_OES_standard_derivatives : require\n"
         "precision highp float;\n"
         "out mediump vec4 sk_FragColor;\n"
         "void main() {\n"
         "    sk_FragColor.x = dFdx(1.0);\n"
         "}\n");
}


DEF_TEST(SkSLIntFolding, r) {
    test(r,
         "void main() {"
         "sk_FragColor.r = 32 + 2;"
         "sk_FragColor.r = 32 - 2;"
         "sk_FragColor.r = 32 * 2;"
         "sk_FragColor.r = 32 / 2;"
         "sk_FragColor.r = 12 | 6;"
         "sk_FragColor.r = 254 & 7;"
         "sk_FragColor.r = 2 ^ 7;"
         "sk_FragColor.r = 1 << 4;"
         "sk_FragColor.r = 128 >> 2;"
         "sk_FragColor.r = -1 == -1 ? 1 : -1;"
         "sk_FragColor.r = -1 == -2 ? 2 : -2;"
         "sk_FragColor.r = 0 != 1 ? 3 : -3;"
         "sk_FragColor.r = 0 != 0 ? 4 : -4;"
         "sk_FragColor.r = 6 > 5 ? 5 : -5;"
         "sk_FragColor.r = 6 > 6 ? 6 : -6;"
         "sk_FragColor.r = -1 < 0 ? 7 : -7;"
         "sk_FragColor.r = 1 < 0 ? 8 : -8;"
         "sk_FragColor.r = 6 >= 6 ? 9 : -9;"
         "sk_FragColor.r = 6 >= 7 ? 10 : -10;"
         "sk_FragColor.r = 6 <= 6 ? 11 : -11;"
         "sk_FragColor.r = 6 <= 5 ? 12 : -12;"
         "sk_FragColor.r = int(sqrt(1)) + 0;"
         "sk_FragColor.r = 0 + int(sqrt(2));"
         "sk_FragColor.r = int(sqrt(3)) - 0;"
         "sk_FragColor.r = int(sqrt(4)) * 0;"
         "sk_FragColor.r = int(sqrt(5)) * 1;"
         "sk_FragColor.r = 1 * int(sqrt(6));"
         "sk_FragColor.r = 0 * int(sqrt(7));"
         "sk_FragColor.r = int(sqrt(8)) / 1;"
         "sk_FragColor.r = 0 / int(sqrt(9));"
         "int x = int(sqrt(2));"
         "x += 1;"
         "x += 0;"
         "x -= 1;"
         "x -= 0;"
         "x *= 1;"
         "x *= 2;"
         "x /= 1;"
         "x /= 2;"
         "sk_FragColor.r = x;"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    sk_FragColor.x = 34.0;\n"
         "    sk_FragColor.x = 30.0;\n"
         "    sk_FragColor.x = 64.0;\n"
         "    sk_FragColor.x = 16.0;\n"
         "    sk_FragColor.x = 14.0;\n"
         "    sk_FragColor.x = 6.0;\n"
         "    sk_FragColor.x = 5.0;\n"
         "    sk_FragColor.x = 16.0;\n"
         "    sk_FragColor.x = 32.0;\n"
         "    sk_FragColor.x = 1.0;\n"
         "    sk_FragColor.x = -2.0;\n"
         "    sk_FragColor.x = 3.0;\n"
         "    sk_FragColor.x = -4.0;\n"
         "    sk_FragColor.x = 5.0;\n"
         "    sk_FragColor.x = -6.0;\n"
         "    sk_FragColor.x = 7.0;\n"
         "    sk_FragColor.x = -8.0;\n"
         "    sk_FragColor.x = 9.0;\n"
         "    sk_FragColor.x = -10.0;\n"
         "    sk_FragColor.x = 11.0;\n"
         "    sk_FragColor.x = -12.0;\n"
         "    sk_FragColor.x = float(int(sqrt(1.0)));\n"
         "    sk_FragColor.x = float(int(sqrt(2.0)));\n"
         "    sk_FragColor.x = float(int(sqrt(3.0)));\n"
         "    sk_FragColor.x = 0.0;\n"
         "    sk_FragColor.x = float(int(sqrt(5.0)));\n"
         "    sk_FragColor.x = float(int(sqrt(6.0)));\n"
         "    sk_FragColor.x = 0.0;\n"
         "    sk_FragColor.x = float(int(sqrt(8.0)));\n"
         "    sk_FragColor.x = 0.0;\n"
         "    int x = int(sqrt(2.0));\n"
         "    x += 1;\n"
         "    x -= 1;\n"
         "    x *= 2;\n"
         "    x /= 2;\n"
         "    sk_FragColor.x = float(x);\n"
         "}\n");
}

DEF_TEST(SkSLFloatFolding, r) {
    test(r,
         "void main() {"
         "sk_FragColor.r = 32.0 + 2.0;"
         "sk_FragColor.r = 32.0 - 2.0;"
         "sk_FragColor.r = 32.0 * 2.0;"
         "sk_FragColor.r = 32.0 / 2.0;"
         "sk_FragColor.r = (12 > 2.0) ? (10 * 2 / 5 + 18 - 3) : 0;"
         "sk_FragColor.r = 0.0 == 0.0 ? 1 : -1;"
         "sk_FragColor.r = 0.0 == 1.0 ? 2 : -2;"
         "sk_FragColor.r = 0.0 != 1.0 ? 3 : -3;"
         "sk_FragColor.r = 0.0 != 0.0 ? 4 : -4;"
         "sk_FragColor.r = 6.0 > 5.0 ? 5 : -5;"
         "sk_FragColor.r = 6.0 > 6.0 ? 6 : -6;"
         "sk_FragColor.r = 6.0 >= 6.0 ? 7 : -7;"
         "sk_FragColor.r = 6.0 >= 7.0 ? 8 : -8;"
         "sk_FragColor.r = 5.0 < 6.0 ? 9 : -9;"
         "sk_FragColor.r = 6.0 < 6.0 ? 10 : -10;"
         "sk_FragColor.r = 6.0 <= 6.0 ? 11 : -11;"
         "sk_FragColor.r = 6.0 <= 5.0 ? 12 : -12;"
         "sk_FragColor.r = sqrt(1) + 0;"
         "sk_FragColor.r = 0 + sqrt(2);"
         "sk_FragColor.r = sqrt(3) - 0;"
         "sk_FragColor.r = sqrt(4) * 0;"
         "sk_FragColor.r = sqrt(5) * 1;"
         "sk_FragColor.r = 1 * sqrt(6);"
         "sk_FragColor.r = 0 * sqrt(7);"
         "sk_FragColor.r = sqrt(8) / 1;"
         "sk_FragColor.r = 0 / sqrt(9);"
         "sk_FragColor.r += 1;"
         "sk_FragColor.r += 0;"
         "sk_FragColor.r -= 1;"
         "sk_FragColor.r -= 0;"
         "sk_FragColor.r *= 1;"
         "sk_FragColor.r *= 2;"
         "sk_FragColor.r /= 1;"
         "sk_FragColor.r /= 2;"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    sk_FragColor.x = 34.0;\n"
         "    sk_FragColor.x = 30.0;\n"
         "    sk_FragColor.x = 64.0;\n"
         "    sk_FragColor.x = 16.0;\n"
         "    sk_FragColor.x = 19.0;\n"
         "    sk_FragColor.x = 1.0;\n"
         "    sk_FragColor.x = -2.0;\n"
         "    sk_FragColor.x = 3.0;\n"
         "    sk_FragColor.x = -4.0;\n"
         "    sk_FragColor.x = 5.0;\n"
         "    sk_FragColor.x = -6.0;\n"
         "    sk_FragColor.x = 7.0;\n"
         "    sk_FragColor.x = -8.0;\n"
         "    sk_FragColor.x = 9.0;\n"
         "    sk_FragColor.x = -10.0;\n"
         "    sk_FragColor.x = 11.0;\n"
         "    sk_FragColor.x = -12.0;\n"
         "    sk_FragColor.x = sqrt(1.0);\n"
         "    sk_FragColor.x = sqrt(2.0);\n"
         "    sk_FragColor.x = sqrt(3.0);\n"
         "    sk_FragColor.x = 0.0;\n"
         "    sk_FragColor.x = sqrt(5.0);\n"
         "    sk_FragColor.x = sqrt(6.0);\n"
         "    sk_FragColor.x = 0.0;\n"
         "    sk_FragColor.x = sqrt(8.0);\n"
         "    sk_FragColor.x = 0.0;\n"
         "    sk_FragColor.x += 1.0;\n"
         "    sk_FragColor.x -= 1.0;\n"
         "    sk_FragColor.x *= 2.0;\n"
         "    sk_FragColor.x /= 2.0;\n"
         "}\n");
}

DEF_TEST(SkSLBoolFolding, r) {
    test(r,
         "void main() {"
         "sk_FragColor.r = 1 == 1 || 2 == 8 ? 1 : -1;"
         "sk_FragColor.r = 1 > 1 || 2 == 8 ? 2 : -2;"
         "sk_FragColor.r = 1 == 1 && 2 <= 8 ? 3 : -3;"
         "sk_FragColor.r = 1 == 2 && 2 == 8 ? 4 : -4;"
         "sk_FragColor.r = 1 == 1 ^^ 1 != 1 ? 5 : -5;"
         "sk_FragColor.r = 1 == 1 ^^ 1 == 1 ? 6 : -6;"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    sk_FragColor.x = 1.0;\n"
         "    sk_FragColor.x = -2.0;\n"
         "    sk_FragColor.x = 3.0;\n"
         "    sk_FragColor.x = -4.0;\n"
         "    sk_FragColor.x = 5.0;\n"
         "    sk_FragColor.x = -6.0;\n"
         "}\n");
}

DEF_TEST(SkSLVecFolding, r) {
    test(r,
         "void main() {"
         "sk_FragColor.r = vec4(0.5, 1, 1, 1).x;"
         "sk_FragColor = vec4(vec2(1), vec2(2, 3)) + vec4(5, 6, 7, 8);"
         "sk_FragColor = vec4(8, vec3(10)) - vec4(1);"
         "sk_FragColor = vec4(2) * vec4(1, 2, 3, 4);"
         "sk_FragColor = vec4(12) / vec4(1, 2, 3, 4);"
         "sk_FragColor.r = (vec4(12) / vec4(1, 2, 3, 4)).y;"
         "sk_FragColor.x = vec4(1) == vec4(1) ? 1.0 : -1.0;"
         "sk_FragColor.x = vec4(1) == vec4(2) ? 2.0 : -2.0;"
         "sk_FragColor.x = vec2(1) == vec2(1, 1) ? 3.0 : -3.0;"
         "sk_FragColor.x = vec2(1, 1) == vec2(1, 1) ? 4.0 : -4.0;"
         "sk_FragColor.x = vec2(1) == vec2(1, 0) ? 5.0 : -5.0;"
         "sk_FragColor.x = vec4(1) == vec4(vec2(1), vec2(1)) ? 6.0 : -6.0;"
         "sk_FragColor.x = vec4(vec3(1), 1) == vec4(vec2(1), vec2(1)) ? 7.0 : -7.0;"
         "sk_FragColor.x = vec4(vec3(1), 1) == vec4(vec2(1), 1, 0) ? 8.0 : -8.0;"
         "sk_FragColor.x = vec2(1) != vec2(1, 0) ? 9.0 : -9.0;"
         "sk_FragColor.x = vec4(1) != vec4(vec2(1), vec2(1)) ? 10.0 : -10.0;"
         "sk_FragColor = vec4(sqrt(1)) * vec4(1);"
         "sk_FragColor = vec4(1) * vec4(sqrt(2));"
         "sk_FragColor = vec4(0) * vec4(sqrt(3));"
         "sk_FragColor = vec4(sqrt(4)) * vec4(0);"
         "sk_FragColor = vec4(0) / vec4(sqrt(5));"
         "sk_FragColor = vec4(0) + vec4(sqrt(6));"
         "sk_FragColor = vec4(sqrt(7)) + vec4(0);"
         "sk_FragColor = vec4(sqrt(8)) - vec4(0);"
         "sk_FragColor = vec4(0) + sqrt(9);"
         "sk_FragColor = vec4(0) * sqrt(10);"
         "sk_FragColor = vec4(0) / sqrt(11);"
         "sk_FragColor = vec4(1) * sqrt(12);"
         "sk_FragColor = 0 + vec4(sqrt(13));"
         "sk_FragColor = 0 * vec4(sqrt(14));"
         "sk_FragColor = 0 / vec4(sqrt(15));"
         "sk_FragColor = 1 * vec4(sqrt(16));"
         "sk_FragColor = vec4(sqrt(17)) + 0;"
         "sk_FragColor = vec4(sqrt(18)) * 0;"
         "sk_FragColor = vec4(sqrt(19)) * 1;"
         "sk_FragColor = vec4(sqrt(19.5)) - 0;"
         "sk_FragColor = sqrt(20) * vec4(1);"
         "sk_FragColor = sqrt(21) + vec4(0);"
         "sk_FragColor = sqrt(22) - vec4(0);"
         "sk_FragColor = sqrt(23) / vec4(1);"
         "sk_FragColor = vec4(sqrt(24)) / 1;"
         "sk_FragColor += vec4(1);"
         "sk_FragColor += vec4(0);"
         "sk_FragColor -= vec4(1);"
         "sk_FragColor -= vec4(0);"
         "sk_FragColor *= vec4(1);"
         "sk_FragColor *= vec4(2);"
         "sk_FragColor /= vec4(1);"
         "sk_FragColor /= vec4(2);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    sk_FragColor.x = 0.5;\n"
         "    sk_FragColor = vec4(6.0, 7.0, 9.0, 11.0);\n"
         "    sk_FragColor = vec4(7.0, 9.0, 9.0, 9.0);\n"
         "    sk_FragColor = vec4(2.0, 4.0, 6.0, 8.0);\n"
         "    sk_FragColor = vec4(12.0, 6.0, 4.0, 3.0);\n"
         "    sk_FragColor.x = 6.0;\n"
         "    sk_FragColor.x = 1.0;\n"
         "    sk_FragColor.x = -2.0;\n"
         "    sk_FragColor.x = 3.0;\n"
         "    sk_FragColor.x = 4.0;\n"
         "    sk_FragColor.x = -5.0;\n"
         "    sk_FragColor.x = 6.0;\n"
         "    sk_FragColor.x = 7.0;\n"
         "    sk_FragColor.x = -8.0;\n"
         "    sk_FragColor.x = 9.0;\n"
         "    sk_FragColor.x = -10.0;\n"
         "    sk_FragColor = vec4(sqrt(1.0));\n"
         "    sk_FragColor = vec4(sqrt(2.0));\n"
         "    sk_FragColor = vec4(0.0);\n"
         "    sk_FragColor = vec4(0.0);\n"
         "    sk_FragColor = vec4(0.0);\n"
         "    sk_FragColor = vec4(sqrt(6.0));\n"
         "    sk_FragColor = vec4(sqrt(7.0));\n"
         "    sk_FragColor = vec4(sqrt(8.0));\n"
         "    sk_FragColor = vec4(sqrt(9.0));\n"
         "    sk_FragColor = vec4(0.0);\n"
         "    sk_FragColor = vec4(0.0);\n"
         "    sk_FragColor = vec4(sqrt(12.0));\n"
         "    sk_FragColor = vec4(sqrt(13.0));\n"
         "    sk_FragColor = vec4(0.0);\n"
         "    sk_FragColor = vec4(0.0);\n"
         "    sk_FragColor = vec4(sqrt(16.0));\n"
         "    sk_FragColor = vec4(sqrt(17.0));\n"
         "    sk_FragColor = vec4(0.0);\n"
         "    sk_FragColor = vec4(sqrt(19.0));\n"
         "    sk_FragColor = vec4(sqrt(19.5));\n"
         "    sk_FragColor = vec4(sqrt(20.0));\n"
         "    sk_FragColor = vec4(sqrt(21.0));\n"
         "    sk_FragColor = vec4(sqrt(22.0));\n"
         "    sk_FragColor = vec4(sqrt(23.0));\n"
         "    sk_FragColor = vec4(sqrt(24.0));\n"
         "    sk_FragColor += vec4(1.0);\n"
         "    sk_FragColor -= vec4(1.0);\n"
         "    sk_FragColor *= vec4(2.0);\n"
         "    sk_FragColor /= vec4(2.0);\n"
         "}\n");
}

DEF_TEST(SkSLMatFolding, r) {
    test(r,
         "void main() {"
         "sk_FragColor.x = mat2(vec2(1.0, 0.0), vec2(0.0, 1.0)) == "
                          "mat2(vec2(1.0, 0.0), vec2(0.0, 1.0)) ? 1 : -1;"
         "sk_FragColor.x = mat2(vec2(1.0, 0.0), vec2(1.0, 1.0)) == "
                          "mat2(vec2(1.0, 0.0), vec2(0.0, 1.0)) ? 2 : -2;"
         "sk_FragColor.x = mat2(1) == mat2(1) ? 3 : -3;"
         "sk_FragColor.x = mat2(1) == mat2(0) ? 4 : -4;"
         "sk_FragColor.x = mat2(1) == mat2(vec2(1.0, 0.0), vec2(0.0, 1.0)) ? 5 : -5;"
         "sk_FragColor.x = mat2(2) == mat2(vec2(1.0, 0.0), vec2(0.0, 1.0)) ? 6 : -6;"
         "sk_FragColor.x = mat3x2(2) == mat3x2(vec2(2.0, 0.0), vec2(0.0, 2.0), vec2(0.0)) ? 7 : -7;"
         "sk_FragColor.x = mat2(1) != mat2(1) ? 8 : -8;"
         "sk_FragColor.x = mat2(1) != mat2(0) ? 9 : -9;"
         "sk_FragColor.x = mat3(vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 0.0)) == "
                          "mat3(mat2(1.0)) ? 10 : -10;"
         "sk_FragColor.x = mat2(mat3(1.0)) == mat2(1.0) ? 11 : -11;"
         "sk_FragColor.x = mat2(vec4(1.0, 0.0, 0.0, 1.0)) == mat2(1.0) ? 12 : -12;"
         "sk_FragColor.x = mat2(1.0, 0.0, vec2(0.0, 1.0)) == mat2(1.0) ? 13 : -13;"
         "sk_FragColor.x = mat2(vec2(1.0, 0.0), 0.0, 1.0) == mat2(1.0) ? 14 : -14;"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    sk_FragColor.x = 1.0;\n"
         "    sk_FragColor.x = -2.0;\n"
         "    sk_FragColor.x = 3.0;\n"
         "    sk_FragColor.x = -4.0;\n"
         "    sk_FragColor.x = 5.0;\n"
         "    sk_FragColor.x = -6.0;\n"
         "    sk_FragColor.x = 7.0;\n"
         "    sk_FragColor.x = -8.0;\n"
         "    sk_FragColor.x = 9.0;\n"
         "    sk_FragColor.x = 10.0;\n"
         "    sk_FragColor.x = 11.0;\n"
         "    sk_FragColor.x = 12.0;\n"
         "    sk_FragColor.x = 13.0;\n"
         "    sk_FragColor.x = 14.0;\n"
         "}\n");
}

DEF_TEST(SkSLConstantIf, r) {
    test(r,
         "void main() {"
         "int x;"
         "if (true) x = 1;"
         "if (2 > 1) x = 2; else x = 3;"
         "if (1 > 2) x = 4; else x = 5;"
         "if (false) x = 6;"
         "sk_FragColor.r = x;"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    sk_FragColor.x = 5.0;\n"
         "}\n");
}

DEF_TEST(SkSLCaps, r) {
    test(r,
         "void main() {"
         "int x = 0;"
         "int y = 0;"
         "int z = 0;"
         "int w = 0;"
         "if (sk_Caps.externalTextureSupport) x = 1;"
         "if (sk_Caps.fbFetchSupport) y = 1;"
         "if (sk_Caps.dropsTileOnZeroDivide && sk_Caps.texelFetchSupport) z = 1;"
         "if (sk_Caps.dropsTileOnZeroDivide && sk_Caps.canUseAnyFunctionInShader) w = 1;"
         "sk_FragColor = vec4(x, y, z, w);"
         "}",
         *SkSL::ShaderCapsFactory::VariousCaps(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    sk_FragColor = vec4(1.0, 0.0, 1.0, 0.0);\n"
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
         "sk_FragColor = vec4(a.x, b.x, c.x, d.x);"
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
         "    sk_FragColor = vec4(a.x, b.x, c.x, d.x);\n"
         "}\n");
    test(r,
         "uniform sampler1D one;"
         "uniform sampler2D two;"
         "void main() {"
         "vec4 a = texture(one, 0);"
         "vec4 b = texture(two, vec2(0));"
         "vec4 c = texture(one, vec2(0));"
         "vec4 d = texture(two, vec3(0));"
         "sk_FragColor = vec4(a.x, b.x, c.x, d.x);"
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
         "    gl_FragColor = vec4(a.x, b.x, c.x, d.x);\n"
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

DEF_TEST(SkSLVertexID, r) {
    test(r,
         "out int id; void main() { id = sk_VertexID; }",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out int id;\n"
         "void main() {\n"
         "    id = gl_VertexID;\n"
         "}\n",
         SkSL::Program::kVertex_Kind);
}

DEF_TEST(SkSLClipDistance, r) {
    test(r,
         "void main() { sk_ClipDistance[0] = 0; }",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "void main() {\n"
         "    gl_ClipDistance[0] = 0.0;\n"
         "}\n",
         SkSL::Program::kVertex_Kind);
    test(r,
         "void main() { sk_FragColor = vec4(sk_ClipDistance[0]); }",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    sk_FragColor = vec4(gl_ClipDistance[0]);\n"
         "}\n");
}

DEF_TEST(SkSLArrayTypes, r) {
    test(r,
         "void main() { vec2 x[2] = vec2[2](vec2(1), vec2(2));"
         "vec2[2] y = vec2[2](vec2(3), vec2(4));"
         "sk_FragColor = vec4(x[0], y[1]); }",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    sk_FragColor = vec4(vec2[2](vec2(1.0), vec2(2.0))[0], "
                                 "vec2[2](vec2(3.0), vec2(4.0))[1]);\n"
         "}\n");
}

DEF_TEST(SkSLGeometry, r) {
    test(r,
         "layout(points) in;"
         "layout(invocations = 2) in;"
         "layout(line_strip, max_vertices = 2) out;"
         "void main() {"
         "gl_Position = sk_in[0].gl_Position + vec4(-0.5, 0, 0, sk_InvocationID);"
         "EmitVertex();"
         "gl_Position = sk_in[0].gl_Position + vec4(0.5, 0, 0, sk_InvocationID);"
         "EmitVertex();"
         "EndPrimitive();"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "layout (points) in ;\n"
         "layout (invocations = 2) in ;\n"
         "layout (line_strip, max_vertices = 2) out ;\n"
         "void main() {\n"
         "    gl_Position = gl_in[0].gl_Position + vec4(-0.5, 0.0, 0.0, float(gl_InvocationID));\n"
         "    EmitVertex();\n"
         "    gl_Position = gl_in[0].gl_Position + vec4(0.5, 0.0, 0.0, float(gl_InvocationID));\n"
         "    EmitVertex();\n"
         "    EndPrimitive();\n"
         "}\n",
         SkSL::Program::kGeometry_Kind);
}

DEF_TEST(SkSLSwitch, r) {
    // basic "does a switch even work" test
    test(r,
         "void main() {"
         "    float x;"
         "    switch (int(sqrt(1))) {"
         "        case 0:"
         "            x = 0.0;"
         "            break;"
         "        case 1:"
         "            x = 1.0;"
         "            break;"
         "        default:"
         "            x = 2.0;"
         "    }"
         "    sk_FragColor = vec4(x);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    float x;\n"
         "    switch (int(sqrt(1.0))) {\n"
         "        case 0:\n"
         "            x = 0.0;\n"
         "            break;\n"
         "        case 1:\n"
         "            x = 1.0;\n"
         "            break;\n"
         "        default:\n"
         "            x = 2.0;\n"
         "    }\n"
         "    sk_FragColor = vec4(x);\n"
         "}\n");
    // dead code inside of switch
    test(r,
         "void main() {"
         "    float x;"
         "    switch (int(sqrt(2))) {"
         "        case 0:"
         "            x = 0.0;"
         "        case 1:"
         "            x = 1.0;"
         "        default:"
         "            x = 2.0;"
         "    }"
         "    sk_FragColor = vec4(x);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    switch (int(sqrt(2.0))) {\n"
         "        case 0:\n"
         "            ;\n"
         "        case 1:\n"
         "            ;\n"
         "        default:\n"
         "            ;\n"
         "    }\n"
         "    sk_FragColor = vec4(2.0);\n"
         "}\n");
    // non-static test w/ fallthrough
    test(r,
         "void main() {"
         "    float x = 0.0;"
         "    switch (int(sqrt(3))) {"
         "        case 0:"
         "            x = 0.0;"
         "        case 1:"
         "            x = 1.0;"
         "    }"
         "    sk_FragColor = vec4(x);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    float x = 0.0;\n"
         "    switch (int(sqrt(3.0))) {\n"
         "        case 0:\n"
         "            x = 0.0;\n"
         "        case 1:\n"
         "            x = 1.0;\n"
         "    }\n"
         "    sk_FragColor = vec4(x);\n"
         "}\n");
    // static test w/ fallthrough
    test(r,
         "void main() {"
         "    float x = 0.0;"
         "    switch (0) {"
         "        case 0:"
         "            x = 0.0;"
         "        case 1:"
         "            x = 1.0;"
         "    }"
         "    sk_FragColor = vec4(x);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    sk_FragColor = vec4(1.0);\n"
         "}\n");
    // static test w/ fallthrough, different entry point
    test(r,
         "void main() {"
         "    float x = 0.0;"
         "    switch (1) {"
         "        case 0:"
         "            x = 0.0;"
         "        case 1:"
         "            x = 1.0;"
         "    }"
         "    sk_FragColor = vec4(x);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    sk_FragColor = vec4(1.0);\n"
         "}\n");
    // static test w/ break
    test(r,
         "void main() {"
         "    float x = 0.0;"
         "    switch (0) {"
         "        case 0:"
         "            x = 0.0;"
         "            break;"
         "        case 1:"
         "            x = 1.0;"
         "    }"
         "    sk_FragColor = vec4(x);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    sk_FragColor = vec4(0.0);\n"
         "}\n");
    // static test w/ static conditional break
    test(r,
         "void main() {"
         "    float x = 0.0;"
         "    switch (0) {"
         "        case 0:"
         "            x = 0.0;"
         "            if (x < 1) break;"
         "        case 1:"
         "            x = 1.0;"
         "    }"
         "    sk_FragColor = vec4(x);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    sk_FragColor = vec4(0.0);\n"
         "}\n");
    // static test w/ non-static conditional break
    test(r,
         "void main() {"
         "    float x = 0.0;"
         "    switch (0) {"
         "        case 0:"
         "            x = 0.0;"
         "            if (x < sqrt(1)) break;"
         "        case 1:"
         "            x = 1.0;"
         "    }"
         "    sk_FragColor = vec4(x);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    float x = 0.0;\n"
         "    switch (0) {\n"
         "        case 0:\n"
         "            x = 0.0;\n"
         "            if (0.0 < sqrt(1.0)) break;\n"
         "        case 1:\n"
         "            x = 1.0;\n"
         "    }\n"
         "    sk_FragColor = vec4(x);\n"
         "}\n");
}

DEF_TEST(SkSLRectangleTexture, r) {
    test(r,
         "uniform sampler2D test;"
         "void main() {"
         "    sk_FragColor = texture(test, vec2(0.5));"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "uniform sampler2D test;\n"
         "void main() {\n"
         "    sk_FragColor = texture(test, vec2(0.5));\n"
         "}\n");
    test(r,
         "uniform sampler2DRect test;"
         "void main() {"
         "    sk_FragColor = texture(test, vec2(0.5));"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "uniform sampler2DRect test;\n"
         "void main() {\n"
         "    sk_FragColor = texture(test, textureSize(test) * vec2(0.5));\n"
         "}\n");
    test(r,
         "uniform sampler2DRect test;"
         "void main() {"
         "    sk_FragColor = texture(test, vec3(0.5));"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "uniform sampler2DRect test;\n"
         "void main() {\n"
         "    sk_FragColor = texture(test, vec3(textureSize(test), 1.0) * vec3(0.5));\n"
         "}\n");
}

DEF_TEST(SkSLUnusedVars, r) {
    test(r,
         "void main() {"
         "float a = 1, b = 2, c = 3;"
         "float d = c;"
         "float e = d;"
         "b++;"
         "d++;"
         "sk_FragColor = vec4(b, b, d, d);"
         "}",
        *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    float b = 2.0;\n"
         "    float d = 3.0;\n"
         "    b++;\n"
         "    d++;\n"
         "    sk_FragColor = vec4(b, b, d, d);\n"
         "}\n");
}

DEF_TEST(SkSLMultipleAssignments, r) {
    test(r,
         "void main() {"
         "float x;"
         "float y;"
         "int z;"
         "x = y = z = 1;"
         "sk_FragColor = vec4(z);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    sk_FragColor = vec4(1.0);\n"
         "}\n");
}

DEF_TEST(SkSLComplexDelete, r) {
    test(r,
         "uniform mat4 colorXform;"
         "uniform sampler2D sampler;"
         "void main() {"
         "vec4 tmpColor;"
         "sk_FragColor = vec4(1.0) * (tmpColor = texture(sampler, vec2(1)) , "
         "colorXform != mat4(1.0) ? vec4(clamp((mat4(colorXform) * vec4(tmpColor.xyz, 1.0)).xyz, "
         "0.0, tmpColor.w), tmpColor.w) : tmpColor);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "uniform mat4 colorXform;\n"
         "uniform sampler2D sampler;\n"
         "void main() {\n"
         "    vec4 tmpColor;\n"
         "    sk_FragColor = (tmpColor = texture(sampler, vec2(1.0)) , colorXform != mat4(1.0) ? "
         "vec4(clamp((colorXform * vec4(tmpColor.xyz, 1.0)).xyz, 0.0, tmpColor.w), tmpColor.w) : "
         "tmpColor);\n"
         "}\n");
}

DEF_TEST(SkSLDependentInitializers, r) {
    test(r,
         "void main() {"
         "float x = 0.5, y = x * 2;"
         "sk_FragColor = vec4(y);"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    sk_FragColor = vec4(1.0);\n"
         "}\n");
}

DEF_TEST(SkSLDeadLoopVar, r) {
    test(r,
         "void main() {"
         "for (int x = 0; x < 4; ) {"
         "break;"
         "}"
         "}",
         *SkSL::ShaderCapsFactory::Default(),
         "#version 400\n"
         "out vec4 sk_FragColor;\n"
         "void main() {\n"
         "    for (; true; ) {\n"
         "        break;\n"
         "    }\n"
         "}\n"
         );
}

DEF_TEST(SkSLInvocations, r) {
    test(r,
         "layout(points) in;"
         "layout(invocations = 2) in;"
         "layout(line_strip, max_vertices = 2) out;"
         "void test() {"
         "gl_Position = sk_in[0].gl_Position + vec4(0.5, 0, 0, sk_InvocationID);"
         "EmitVertex();"
         "}"
         "void main() {"
         "gl_Position = sk_in[0].gl_Position + vec4(-0.5, 0, 0, sk_InvocationID);"
         "EmitVertex();"
         "}",
         *SkSL::ShaderCapsFactory::MustImplementGSInvocationsWithLoop(),
         "#version 400\n"
         "int sk_InvocationID;\n"
         "layout (points) in ;\n"
         "layout (line_strip, max_vertices = 4) out ;\n"
         "void test() {\n"
         "    gl_Position = gl_in[0].gl_Position + vec4(0.5, 0.0, 0.0, float(sk_InvocationID));\n"
         "    EmitVertex();\n"
         "}\n"
         "void _invoke() {\n"
         "    gl_Position = gl_in[0].gl_Position + vec4(-0.5, 0.0, 0.0, float(sk_InvocationID));\n"
         "    EmitVertex();\n"
         "}\n"
         "void main() {\n"
         "    for (sk_InvocationID = 0;sk_InvocationID < 2; sk_InvocationID++) {\n"
         "        _invoke();\n"
         "        EndPrimitive();\n"
         "    }\n"
         "}\n",
         SkSL::Program::kGeometry_Kind);
}

#endif
