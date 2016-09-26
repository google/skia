/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSLCompiler.h"

#include "Test.h"

static void test(skiatest::Reporter* r, const char* src, SkSL::GLCaps caps, const char* expected) {
    SkSL::Compiler compiler;
    std::string output;
    bool result = compiler.toGLSL(SkSL::Program::kFragment_Kind, src, caps, &output);
    if (!result) {
        SkDebugf("Unexpected error compiling %s\n%s", src, compiler.errorText().c_str());
    }
    REPORTER_ASSERT(r, result);
    if (result) {
        if (output != expected) {
            SkDebugf("GLSL MISMATCH:\nsource:\n%s\n\nexpected:\n'%s'\n\nreceived:\n'%s'", src, 
                     expected, output.c_str());
        }
        REPORTER_ASSERT(r, output == expected);
    }
}

DEF_TEST(SkSLHelloWorld, r) {
    SkSL::GLCaps caps = { 400, SkSL::GLCaps::kGL_Standard };
    test(r,
         "out vec4 fragColor; void main() { fragColor = vec4(0.75); }",
         caps,
         "#version 400\n"
         "out vec4 fragColor;\n"
         "void main() {\n"
         "    fragColor = vec4(0.75);\n"
         "}\n");
}

DEF_TEST(SkSLControl, r) {
    SkSL::GLCaps caps = { 400, SkSL::GLCaps::kGL_Standard };
    test(r,
         "out vec4 fragColor;"
         "void main() {"
         "if (1 + 2 + 3 > 5) { fragColor = vec4(0.75); } else { discard; }"
         "int i = 0;"
         "while (i < 10) fragColor *= 0.5;"
         "do { fragColor += 0.01; } while (fragColor.x < 0.7);"
         "for (int i = 0; i < 10; i++) {"
         "if (i % 0 == 1) break; else continue;"
         "}"
         "return;"
         "}",
         caps,
         "#version 400\n"
         "out vec4 fragColor;\n"
         "void main() {\n"
         "    if ((1 + 2) + 3 > 5) {\n"
         "        fragColor = vec4(0.75);\n"
         "    } else {\n"
         "        discard;\n"
         "    }\n"
         "    int i = 0;\n"
         "    while (i < 10) fragColor *= 0.5;\n"
         "    do {\n"
         "        fragColor += 0.01;\n"
         "    } while (fragColor.x < 0.7);\n"
         "    for (int i = 0;i < 10; i++) {\n"
         "        if (i % 0 == 1) break; else continue;\n"
         "    }\n"
         "    return;\n"
         "}\n");
}

DEF_TEST(SkSLFunctions, r) {
    SkSL::GLCaps caps = { 400, SkSL::GLCaps::kGL_Standard };
    test(r,
         "out vec4 fragColor;"
         "float foo(float v[2]) { return v[0] * v[1]; }"
         "void bar(inout float x) { float y[2], z; y[0] = x; y[1] = x * 2; z = foo(y); x = z; }"
         "void main() { float x = 10; bar(x); fragColor = vec4(x); }",
         caps,
         "#version 400\n"
         "out vec4 fragColor;\n"
         "float foo(in float[2] v) {\n"
         "    return v[0] * v[1];\n"
         "}\n"
         "void bar(inout float x) {\n"
         "    float y[2], z;\n"
         "    y[0] = x;\n"
         "    y[1] = x * 2;\n"
         "    z = foo(y);\n"
         "    x = z;\n"
         "}\n"
         "void main() {\n"
         "    float x = 10;\n"
         "    bar(x);\n"
         "    fragColor = vec4(x);\n"
         "}\n");
}

DEF_TEST(SkSLOperators, r) {
    SkSL::GLCaps caps = { 400, SkSL::GLCaps::kGL_Standard };
    test(r,
         "void main() {"
         "float x = 1, y = 2;"
         "int z = 3;"
         "x = x + y * z * x * (y - z);"
         "y = x / y / z;"
         "z = (z / 2 % 3 << 4) >> 2 << 1;"
         "bool b = (x > 4) == x < 2 || 2 >= 5 && y <= z && 12 != 11;"
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
         caps,
         "#version 400\n"
         "void main() {\n"
         "    float x = 1, y = 2;\n"
         "    int z = 3;\n"
         "    x = x + ((y * float(z)) * x) * (y - float(z));\n"
         "    y = (x / y) / float(z);\n"
         "    z = (((z / 2) % 3 << 4) >> 2) << 1;\n"
         "    bool b = x > 4 == x < 2 || (2 >= 5 && y <= float(z)) && 12 != 11;\n"
         "    x += 12;\n"
         "    x -= 12;\n"
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
    SkSL::GLCaps caps = { 400, SkSL::GLCaps::kGL_Standard };
    test(r,
         "void main() {"
         "mat2x4 x = mat2x4(1);"
         "mat3x2 y = mat3x2(1, 0, 0, 1, vec2(2, 2));"
         "mat3x4 z = x * y;"
         "vec3 v1 = mat3(1) * vec3(1);"
         "vec3 v2 = vec3(1) * mat3(1);"
         "}",
         caps,
         "#version 400\n"
         "void main() {\n"
         "    mat2x4 x = mat2x4(1);\n"
         "    mat3x2 y = mat3x2(1, 0, 0, 1, vec2(2, 2));\n"
         "    mat3x4 z = x * y;\n"
         "    vec3 v1 = mat3(1) * vec3(1);\n"
         "    vec3 v2 = vec3(1) * mat3(1);\n"
         "}\n");
}

DEF_TEST(SkSLInterfaceBlock, r) {
    SkSL::GLCaps caps = { 400, SkSL::GLCaps::kGL_Standard };
    test(r,
         "uniform testBlock {"
         "float x;"
         "float y[2];"
         "layout(binding=12) mat3x2 z;"
         "bool w;"
         "};"
         "void main() {"
         "}",
         caps,
         "#version 400\n"
         "uniform testBlock {\n"
         "    float x;\n"
         "    float[2] y;\n"
         "    layout (binding = 12)mat3x2 z;\n"
         "    bool w;\n"
         "};\n"
         "void main() {\n"
         "}\n");
}

DEF_TEST(SkSLStructs, r) {
    SkSL::GLCaps caps = { 400, SkSL::GLCaps::kGL_Standard };
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
         caps,
         "#version 400\n"
         "struct A {\n"
         "    int x;\n"
         "    int y;\n"
         "}\n"
         " a1, a2;\n"
         "A a3;\n"
         "struct B {\n"
         "    float x;\n"
         "    float[2] y;\n"
         "    layout (binding = 1)A z;\n"
         "}\n"
         " b1, b2, b3;\n"
         "void main() {\n"
         "}\n");

}
