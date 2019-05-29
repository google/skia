/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPoint3.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLExternalValue.h"
#include "src/sksl/SkSLInterpreter.h"
#include "src/utils/SkJSON.h"

#include "tests/Test.h"

void test(skiatest::Reporter* r, const char* src, SkSL::Interpreter::Value* in, int expectedCount,
          SkSL::Interpreter::Value* expected) {
    SkSL::Compiler compiler;
    SkSL::Program::Settings settings;
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(
                                                             SkSL::Program::kGeneric_Kind,
                                                             SkSL::String(src), settings);
    REPORTER_ASSERT(r, program);
    if (program) {
        std::unique_ptr<SkSL::ByteCode> byteCode = compiler.toByteCode(*program);
        REPORTER_ASSERT(r, !compiler.errorCount());
        if (compiler.errorCount() > 0) {
            printf("%s\n%s", src, compiler.errorText().c_str());
            return;
        }
        SkSL::ByteCodeFunction* main = byteCode->fFunctions[0].get();
        SkSL::Interpreter interpreter(std::move(program), std::move(byteCode));
        std::unique_ptr<SkSL::Interpreter::Value[]> out =
           std::unique_ptr<SkSL::Interpreter::Value[]>(new SkSL::Interpreter::Value[expectedCount]);
        interpreter.run(*main, in, out.get());
        bool valid = !memcmp(out.get(), expected, sizeof(SkSL::Interpreter::Value) * expectedCount);
        if (!valid) {
            printf("for program: %s\n", src);
            printf("    expected (");
            const char* separator = "";
            for (int i = 0; i < expectedCount; ++i) {
                printf("%s%f", separator, expected[i].fFloat);
                separator = ", ";
            }
            printf("), but received (");
            separator = "";
            for (int i = 0; i < expectedCount; ++i) {
                printf("%s%f", separator, out.get()[i].fFloat);
                separator = ", ";
            }
            printf(")\n");
            interpreter.disassemble(*main);
        }
        REPORTER_ASSERT(r, valid);
    } else {
        printf("%s\n%s", src, compiler.errorText().c_str());
    }
}

void test(skiatest::Reporter* r, const char* src, float inR, float inG, float inB, float inA,
        float expectedR, float expectedG, float expectedB, float expectedA) {
    SkSL::Compiler compiler;
    SkSL::Program::Settings settings;
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(
                                                             SkSL::Program::kGeneric_Kind,
                                                             SkSL::String(src), settings);
    REPORTER_ASSERT(r, program);
    if (program) {
        std::unique_ptr<SkSL::ByteCode> byteCode = compiler.toByteCode(*program);
        REPORTER_ASSERT(r, !compiler.errorCount());
        if (compiler.errorCount() > 0) {
            printf("%s\n%s", src, compiler.errorText().c_str());
            return;
        }
        SkSL::ByteCodeFunction* main = byteCode->fFunctions[0].get();
        SkSL::Interpreter interpreter(std::move(program), std::move(byteCode));
        float inoutColor[4] = { inR, inG, inB, inA };
        interpreter.run(*main, (SkSL::Interpreter::Value*) inoutColor, nullptr);
        if (inoutColor[0] != expectedR || inoutColor[1] != expectedG ||
            inoutColor[2] != expectedB || inoutColor[3] != expectedA) {
            printf("for program: %s\n", src);
            printf("    expected (%f, %f, %f, %f), but received (%f, %f, %f, %f)\n", expectedR,
                   expectedG, expectedB, expectedA, inoutColor[0], inoutColor[1], inoutColor[2],
                   inoutColor[3]);
            interpreter.disassemble(*main);
        }
        REPORTER_ASSERT(r, inoutColor[0] == expectedR);
        REPORTER_ASSERT(r, inoutColor[1] == expectedG);
        REPORTER_ASSERT(r, inoutColor[2] == expectedB);
        REPORTER_ASSERT(r, inoutColor[3] == expectedA);
    } else {
        printf("%s\n%s", src, compiler.errorText().c_str());
    }
}

DEF_TEST(SkSLInterpreterAdd, r) {
    test(r, "void main(inout half4 color) { color.r = color.r + color.g; }", 0.25, 0.75, 0, 0, 1,
         0.75, 0, 0);
    test(r, "void main(inout half4 color) { color += half4(1, 2, 3, 4); }", 4, 3, 2, 1, 5, 5, 5, 5);
    test(r, "void main(inout half4 color) { half4 c = color; color += c; }", 0.25, 0.5, 0.75, 1,
         0.5, 1, 1.5, 2);
    test(r, "void main(inout half4 color) { int a = 1; int b = 3; color.r = a + b; }", 1, 2, 3, 4,
         4, 2, 3, 4);
}

DEF_TEST(SkSLInterpreterSubtract, r) {
    test(r, "void main(inout half4 color) { color.r = color.r - color.g; }", 1, 0.75, 0, 0, 0.25,
         0.75, 0, 0);
    test(r, "void main(inout half4 color) { color -= half4(1, 2, 3, 4); }", 5, 5, 5, 5, 4, 3, 2, 1);
    test(r, "void main(inout half4 color) { half4 c = color; color -= c; }", 4, 3, 2, 1,
         0, 0, 0, 0);
    test(r, "void main(inout half4 color) { color.x = -color.x; }", 4, 3, 2, 1, -4, 3, 2, 1);
    test(r, "void main(inout half4 color) { color = -color; }", 4, 3, 2, 1, -4, -3, -2, -1);
    test(r, "void main(inout half4 color) { int a = 3; int b = 1; color.r = a - b; }", 0, 0, 0, 0,
         2, 0, 0, 0);
}

DEF_TEST(SkSLInterpreterMultiply, r) {
    test(r, "void main(inout half4 color) { color.r = color.r * color.g; }", 2, 3, 0, 0, 6, 3, 0,
         0);
    test(r, "void main(inout half4 color) { color *= half4(1, 2, 3, 4); }", 2, 3, 4, 5, 2, 6, 12,
         20);
    test(r, "void main(inout half4 color) { half4 c = color; color *= c; }", 4, 3, 2, 1,
         16, 9, 4, 1);
    test(r, "void main(inout half4 color) { int a = 3; int b = -2; color.r = a * b; }", 0, 0, 0, 0,
         -6, 0, 0, 0);
}

DEF_TEST(SkSLInterpreterDivide, r) {
    test(r, "void main(inout half4 color) { color.r = color.r / color.g; }", 1, 2, 0, 0, 0.5, 2, 0,
         0);
    test(r, "void main(inout half4 color) { color /= half4(1, 2, 3, 4); }", 12, 12, 12, 12, 12, 6,
         4, 3);
    test(r, "void main(inout half4 color) { half4 c = color; color /= c; }", 4, 3, 2, 1,
         1, 1, 1, 1);
    test(r, "void main(inout half4 color) { int a = 8; int b = -2; color.r = a / b; }", 0, 0, 0, 0,
         -4, 0, 0, 0);
}

DEF_TEST(SkSLInterpreterRemainder, r) {
    test(r, "void main(inout half4 color) { color.r = color.r % color.g; }", 3.125, 2, 0, 0,
         1.125, 2, 0, 0);
    test(r, "void main(inout half4 color) { color %= half4(1, 2, 3, 4); }", 9.5, 9.5, 9.5, 9.5,
         0.5, 1.5, 0.5, 1.5);
    test(r, "void main(inout half4 color) { int a = 8; int b = 3; a %= b; color.r = a; }", 0, 0, 0,
         0, 2, 0, 0, 0);
    test(r, "void main(inout half4 color) { int a = 8; int b = 3; color.r = a % b; }", 0, 0, 0, 0,
         2, 0, 0, 0);
    test(r, "void main(inout half4 color) { int2 a = int2(8, 10); a %= 6; color.rg = a; }", 0, 0, 0,
         0, 2, 4, 0, 0);
}

DEF_TEST(SkSLInterpreterMatrix, r) {
    SkSL::Interpreter::Value in[4];
    SkSL::Interpreter::Value expected[1];

    // Constructing matrix from scalar produces a diagonal matrix
    in[0] = 1.0f;
    expected[0] = 2.0f;
    test(r, "float main(float x) { float4x4 m = float4x4(x); return m[1][1] + m[1][2] + m[2][2]; }",
         in, 1, expected);

    // With non-square matrix
    test(r, "float main(float x) { float3x2 m = float3x2(x); return m[0][0] + m[1][1] + m[2][1]; }",
         in, 1, expected);

    // Constructing from a different-sized matrix fills the remaining space with the identity matrix
    test(r, "float main(float x) {"
         "float3x2 m = float3x2(x);"
         "float4x4 m2 = float4x4(m);"
         "return m2[0][0] + m2[3][3]; }",
         in, 1, expected);

    // Constructing a matrix from vectors or scalars fills in values in column-major order
    in[0] = 1.0f;
    in[1] = 2.0f;
    in[2] = 4.0f;
    in[3] = 8.0f;
    expected[0] = 6.0f;
    test(r, "float main(float4 v) { float2x2 m = float2x2(v); return m[0][1] + m[1][0]; }",
         in, 1, expected);

    expected[0] = 10.0f;
    test(r, "float main(float4 v) {"
         "float2x2 m = float2x2(v.x, v.y, v.w, v.z);"
         "return m[0][1] + m[1][0]; }",
         in, 1, expected);

#if 0
    // Addition of matrices
    test(r, "void main(inout half4 color) {"
         "half4x4 m = half4x4(color, color, color, color);"
         "m += m; color = m[0]; }",
         1, 2, 3, 4, 2, 4, 6, 8);

    // Matrix * Vector multiplication
    test(r, "void main(inout half4 color) {"
         "half4x4 m = half4x4(color, color, color, color);"
         "color = m * color; }",
         1, 2, 3, 4, 10, 20, 30, 40);

    // Matrix * Matrix multiplication
    test(r, "void main(inout half4 color) {"
         "half4x4 m = half4x4(color, color, color, color);"
         "m = m * m; color = m[2]; }",
         1, 2, 3, 4, 10, 20, 30, 40);
#endif
}

DEF_TEST(SkSLInterpreterTernary, r) {
    test(r, "void main(inout half4 color) { color.r = color.g > color.b ? color.g : color.b; }",
         0, 1, 2, 0, 2, 1, 2, 0);
    test(r, "void main(inout half4 color) { color.r = color.g > color.b ? color.g : color.b; }",
         0, 3, 2, 0, 3, 3, 2, 0);
}

DEF_TEST(SkSLInterpreterCast, r) {
    SkSL::Interpreter::Value input[2];
    SkSL::Interpreter::Value expected[2];

    input[0].fSigned = 3;
    input[1].fSigned = -5;
    expected[0].fFloat = 3.0f;
    expected[1].fFloat = -5.0f;
    test(r, "float  main(int  x) { return float (x); }", input, 1, expected);
    test(r, "float2 main(int2 x) { return float2(x); }", input, 2, expected);

    input[0].fUnsigned = 3;
    input[1].fUnsigned = 5;
    expected[0].fFloat = 3.0f;
    expected[1].fFloat = 5.0f;
    test(r, "float  main(uint  x) { return float (x); }", input, 1, expected);
    test(r, "float2 main(uint2 x) { return float2(x); }", input, 2, expected);

    input[0].fFloat = 3.0f;
    input[1].fFloat = -5.0f;
    expected[0].fSigned = 3;
    expected[1].fSigned = -5;
    test(r, "int  main(float  x) { return int (x); }", input, 1, expected);
    test(r, "int2 main(float2 x) { return int2(x); }", input, 2, expected);

    input[0].fSigned = 3;
    expected[0].fFloat = 3.0f;
    expected[1].fFloat = 3.0f;
    test(r, "float2 main(int x) { return float2(x); }", input, 2, expected);
}

DEF_TEST(SkSLInterpreterIf, r) {
    test(r, "void main(inout half4 color) { if (color.r > color.g) color.a = 1; }", 5, 3, 0, 0,
         5, 3, 0, 1);
    test(r, "void main(inout half4 color) { if (color.r > color.g) color.a = 1; }", 5, 5, 0, 0,
         5, 5, 0, 0);
    test(r, "void main(inout half4 color) { if (color.r > color.g) color.a = 1; }", 5, 6, 0, 0,
         5, 6, 0, 0);
    test(r, "void main(inout half4 color) { if (color.r < color.g) color.a = 1; }", 3, 5, 0, 0,
         3, 5, 0, 1);
    test(r, "void main(inout half4 color) { if (color.r < color.g) color.a = 1; }", 5, 5, 0, 0,
         5, 5, 0, 0);
    test(r, "void main(inout half4 color) { if (color.r < color.g) color.a = 1; }", 6, 5, 0, 0,
         6, 5, 0, 0);
    test(r, "void main(inout half4 color) { if (color.r >= color.g) color.a = 1; }", 5, 3, 0, 0,
         5, 3, 0, 1);
    test(r, "void main(inout half4 color) { if (color.r >= color.g) color.a = 1; }", 5, 5, 0, 0,
         5, 5, 0, 1);
    test(r, "void main(inout half4 color) { if (color.r >= color.g) color.a = 1; }", 5, 6, 0, 0,
         5, 6, 0, 0);
    test(r, "void main(inout half4 color) { if (color.r <= color.g) color.a = 1; }", 3, 5, 0, 0,
         3, 5, 0, 1);
    test(r, "void main(inout half4 color) { if (color.r <= color.g) color.a = 1; }", 5, 5, 0, 0,
         5, 5, 0, 1);
    test(r, "void main(inout half4 color) { if (color.r <= color.g) color.a = 1; }", 6, 5, 0, 0,
         6, 5, 0, 0);
    test(r, "void main(inout half4 color) { if (color.r == color.g) color.a = 1; }", 2, 2, 0, 0,
         2, 2, 0, 1);
    test(r, "void main(inout half4 color) { if (color.r == color.g) color.a = 1; }", 2, -2, 0, 0,
         2, -2, 0, 0);
    test(r, "void main(inout half4 color) { if (color.r != color.g) color.a = 1; }", 2, 2, 0, 0,
         2, 2, 0, 0);
    test(r, "void main(inout half4 color) { if (color.r != color.g) color.a = 1; }", 2, -2, 0, 0,
         2, -2, 0, 1);
    test(r, "void main(inout half4 color) { if (color.r == color.g) color.a = 1; else "
         "color.a = 2; }", 1, 1, 0, 0, 1, 1, 0, 1);
    test(r, "void main(inout half4 color) { if (color.r == color.g) color.a = 1; else "
         "color.a = 2; }", 2, -2, 0, 0, 2, -2, 0, 2);
}

DEF_TEST(SkSLInterpreterIfVector, r) {
    test(r, "void main(inout half4 color) { if (color.rg == color.ba) color.a = 1; }",
         1, 2, 1, 2, 1, 2, 1, 1);
    test(r, "void main(inout half4 color) { if (color.rg == color.ba) color.a = 1; }",
         1, 2, 3, 2, 1, 2, 3, 2);
    test(r, "void main(inout half4 color) { if (color.rg != color.ba) color.a = 1; }",
         1, 2, 1, 2, 1, 2, 1, 2);
    test(r, "void main(inout half4 color) { if (color.rg != color.ba) color.a = 1; }",
         1, 2, 3, 2, 1, 2, 3, 1);
}

DEF_TEST(SkSLInterpreterWhile, r) {
    test(r, "void main(inout half4 color) { while (color.r < 1) color.r += 0.25; }", 0, 0, 0, 0, 1,
         0, 0, 0);
    test(r, "void main(inout half4 color) { while (color.r > 1) color.r += 0.25; }", 0, 0, 0, 0, 0,
         0, 0, 0);
    test(r, "void main(inout half4 color) { while (true) { color.r += 0.5; "
         "if (color.r > 1) break; } }", 0, 0, 0, 0, 1.5, 0, 0, 0);
    test(r, "void main(inout half4 color) { while (color.r < 10) { color.r += 0.5; "
            "if (color.r < 5) continue; break; } }", 0, 0, 0, 0, 5, 0, 0, 0);
}

DEF_TEST(SkSLInterpreterDo, r) {
    test(r, "void main(inout half4 color) { do color.r += 0.25; while (color.r < 1); }", 0, 0, 0, 0,
         1, 0, 0, 0);
    test(r, "void main(inout half4 color) { do color.r += 0.25; while (color.r > 1); }", 0, 0, 0, 0,
         0.25, 0, 0, 0);
    test(r, "void main(inout half4 color) { do { color.r += 0.5; if (color.r > 1) break; } while "
            "(true); }", 0, 0, 0, 0, 1.5, 0, 0, 0);
    test(r, "void main(inout half4 color) {do { color.r += 0.5; if (color.r < 5) "
            "continue; if (color.r >= 5) break; } while (true); }", 0, 0, 0, 0, 5, 0, 0, 0);
    test(r, "void main(inout half4 color) { do { color.r += 0.5; } while (false); }",
         0, 0, 0, 0, 0.5, 0, 0, 0);
}

DEF_TEST(SkSLInterpreterFor, r) {
    test(r, "void main(inout half4 color) { for (int i = 1; i <= 10; ++i) color.r += i; }", 0, 0, 0,
         0, 55, 0, 0, 0);
    test(r,
         "void main(inout half4 color) {"
         "    for (int i = 1; i <= 10; ++i)"
         "        for (int j = i; j <= 10; ++j)"
         "            color.r += j;"
         "}",
         0, 0, 0, 0,
         385, 0, 0, 0);
    test(r,
         "void main(inout half4 color) {"
         "    for (int i = 1; i <= 10; ++i)"
         "        for (int j = 1; ; ++j) {"
         "            if (i == j) continue;"
         "            if (j > 10) break;"
         "            color.r += j;"
         "        }"
         "}",
         0, 0, 0, 0,
         495, 0, 0, 0);
}

DEF_TEST(SkSLInterpreterPrefixPostfix, r) {
    test(r, "void main(inout half4 color) { color.r = ++color.g; }", 1, 2, 3, 4, 3, 3, 3, 4);
    test(r, "void main(inout half4 color) { color.r = color.g++; }", 1, 2, 3, 4, 2, 3, 3, 4);
}

DEF_TEST(SkSLInterpreterSwizzle, r) {
    test(r, "void main(inout half4 color) { color = color.abgr; }", 1, 2, 3, 4, 4, 3, 2, 1);
    test(r, "void main(inout half4 color) { color.rgb = half4(5, 6, 7, 8).bbg; }", 1, 2, 3, 4, 7, 7,
         6, 4);
    test(r, "void main(inout half4 color) { color.bgr = int3(5, 6, 7); }", 1, 2, 3, 4, 7, 6,
         5, 4);
}

DEF_TEST(SkSLInterpreterGlobal, r) {
    test(r, "int x; void main(inout half4 color) { x = 10; color.b = x; }", 1, 2, 3, 4, 1, 2, 10,
         4);
    test(r, "float4 x; void main(inout float4 color) { x = color * 2; color = x; }",
         1, 2, 3, 4, 2, 4, 6, 8);
    test(r, "float4 x; void main(inout float4 color) { x = float4(5, 6, 7, 8); color = x.wzyx; }",
         1, 2, 3, 4, 8, 7, 6, 5);
    test(r, "float4 x; void main(inout float4 color) { x.wzyx = float4(5, 6, 7, 8); color = x; }",
         1, 2, 3, 4, 8, 7, 6, 5);
}

DEF_TEST(SkSLInterpreterGeneric, r) {
    float value1 = 5;
    float expected1 = 25;
    test(r, "float main(float x) { return x * x; }", (SkSL::Interpreter::Value*) &value1, 1,
         (SkSL::Interpreter::Value*) &expected1);
    float value2[2] = { 5, 25 };
    float expected2[2] = { 25, 625 };
    test(r, "float2 main(float x, float y) { return float2(x * x, y * y); }",
         (SkSL::Interpreter::Value*) &value2, 2,
         (SkSL::Interpreter::Value*) expected2);
}

DEF_TEST(SkSLInterpreterSetInputs, r) {
    const char* src = R"(
        layout(ctype=float) in uniform float x;
        float main(float y) { return x + y; }
    )";

    SkSL::Compiler compiler;
    SkSL::Program::Settings settings;
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(
                                                             SkSL::Program::kGeneric_Kind,
                                                             SkSL::String(src), settings);
    REPORTER_ASSERT(r, program);

    std::unique_ptr<SkSL::ByteCode> byteCode = compiler.toByteCode(*program);
    REPORTER_ASSERT(r, !compiler.errorCount());

    SkSL::ByteCodeFunction* main = byteCode->fFunctions[0].get();

    float x = 1.0f;
    SkSL::Interpreter interpreter(std::move(program), std::move(byteCode),
                                    (SkSL::Interpreter::Value*)&x);
    float out = 0.0f;
    float in  = 2.0f;
    interpreter.run(*main, (SkSL::Interpreter::Value*)&in, (SkSL::Interpreter::Value*)&out);
    REPORTER_ASSERT(r, out == 3.0f);

    // External updates should be ignored
    x = 3.0f;
    out = 0.0f;
    interpreter.run(*main, (SkSL::Interpreter::Value*)&in, (SkSL::Interpreter::Value*)&out);
    REPORTER_ASSERT(r, out == 3.0f);

    // Updating inputs should affect subsequent calls to run
    out = 0.0f;
    interpreter.setInputs((SkSL::Interpreter::Value*)&x);
    interpreter.run(*main, (SkSL::Interpreter::Value*)&in, (SkSL::Interpreter::Value*)&out);
    REPORTER_ASSERT(r, out == 5.0f);
}

DEF_TEST(SkSLInterpreterCompound, r) {
    struct RectAndColor { SkIRect fRect; SkColor4f fColor; };
    struct ManyRects { int fNumRects; RectAndColor fRects[4]; };

    const char* src =
        // Some struct definitions
        "struct Point { int x; int y; };\n"
        "struct Rect {  Point p0; Point p1; };\n"
        "struct RectAndColor { Rect r; float4 color; };\n"

        // Structs as globals, parameters, return values
        "RectAndColor temp;\n"
        "int rect_height(Rect r) { return r.p1.y - r.p0.y; }\n"
        "RectAndColor make_blue_rect(int w, int h) {\n"
        "  temp.r.p0.x = temp.r.p0.y = 0;\n"
        "  temp.r.p1.x = w; temp.r.p1.y = h;\n"
        "  temp.color = float4(0, 1, 0, 1);\n"
        "  return temp;\n"
        "}\n"

        // Initialization and assignment of types larger than 4 slots
        "RectAndColor init_big(RectAndColor r) { RectAndColor s = r; return s; }\n"
        "RectAndColor copy_big(RectAndColor r) { RectAndColor s; s = r; return s; }\n"

        // Same for arrays, including some non-constant indexing
        "float tempFloats[8];\n"
        "int median(int a[15]) { return a[7]; }\n"
        "float[8] sums(float a[8]) {\n"
        "  float tempFloats[8];\n"
        "  tempFloats[0] = a[0];\n"
        "  for (int i = 1; i < 8; ++i) { tempFloats[i] = tempFloats[i - 1] + a[i]; }\n"
        "  return tempFloats;\n"
        "}\n"

        // Uniforms, array-of-structs, dynamic indices
        "in uniform Rect gRects[4];\n"
        "Rect get_rect(int i) { return gRects[i]; }\n"

        // Kitchen sink (swizzles, inout, SoAoS)
        "struct ManyRects { int numRects; RectAndColor rects[4]; };\n"
        "void fill_rects(inout ManyRects mr) {\n"
        "  for (int i = 0; i < mr.numRects; ++i) {\n"
        "    mr.rects[i].r = gRects[i];\n"
        "    float b = mr.rects[i].r.p1.y;\n"
        "    mr.rects[i].color = float4(b, b, b, b);\n"
        "  }\n"
        "}\n";

    SkSL::Compiler compiler;
    SkSL::Program::Settings settings;
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(
                                                             SkSL::Program::kGeneric_Kind,
                                                             SkSL::String(src), settings);
    REPORTER_ASSERT(r, program);

    std::unique_ptr<SkSL::ByteCode> byteCode = compiler.toByteCode(*program);
    REPORTER_ASSERT(r, !compiler.errorCount());

    auto rect_height    = byteCode->getFunction("rect_height"),
         make_blue_rect = byteCode->getFunction("make_blue_rect"),
         median         = byteCode->getFunction("median"),
         sums           = byteCode->getFunction("sums"),
         get_rect       = byteCode->getFunction("get_rect"),
         fill_rects     = byteCode->getFunction("fill_rects");

    SkIRect gRects[4] = { { 1,2,3,4 }, { 5,6,7,8 }, { 9,10,11,12 }, { 13,14,15,16 } };

    SkSL::Interpreter interpreter(std::move(program), std::move(byteCode),
                                  (SkSL::Interpreter::Value*)gRects);

    {
        SkIRect in = SkIRect::MakeXYWH(10, 10, 20, 30);
        int out = 0;
        interpreter.run(*rect_height,
                        (SkSL::Interpreter::Value*)&in,
                        (SkSL::Interpreter::Value*)&out);
        REPORTER_ASSERT(r, out == 30);
    }

    {
        int in[2] = { 15, 25 };
        RectAndColor out;
        interpreter.run(*make_blue_rect,
                        (SkSL::Interpreter::Value*)in,
                        (SkSL::Interpreter::Value*)&out);
        REPORTER_ASSERT(r, out.fRect.width() == 15);
        REPORTER_ASSERT(r, out.fRect.height() == 25);
        SkColor4f blue = { 0.0f, 1.0f, 0.0f, 1.0f };
        REPORTER_ASSERT(r, out.fColor == blue);
    }

    {
        int in[15] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
        int out = 0;
        interpreter.run(*median,
                        (SkSL::Interpreter::Value*)in,
                        (SkSL::Interpreter::Value*)&out);
        REPORTER_ASSERT(r, out == 8);
    }

    {
        float in[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
        float out[8] = { 0 };
        interpreter.run(*sums,
                        (SkSL::Interpreter::Value*)in,
                        (SkSL::Interpreter::Value*)out);
        for (int i = 0; i < 8; ++i) {
            REPORTER_ASSERT(r, out[i] == static_cast<float>((i + 1) * (i + 2) / 2));
        }
    }

    {
        int in = 2;
        SkIRect out = SkIRect::MakeEmpty();
        interpreter.run(*get_rect,
                        (SkSL::Interpreter::Value*)&in,
                        (SkSL::Interpreter::Value*)&out);
        REPORTER_ASSERT(r, out == gRects[2]);
    }

    {
        ManyRects in;
        memset(&in, 0, sizeof(in));
        in.fNumRects = 2;
        interpreter.run(*fill_rects,
                        (SkSL::Interpreter::Value*)&in,
                        nullptr);
        ManyRects expected;
        memset(&expected, 0, sizeof(expected));
        expected.fNumRects = 2;
        for (int i = 0; i < 2; ++i) {
            expected.fRects[i].fRect = gRects[i];
            float c = gRects[i].fBottom;
            expected.fRects[i].fColor = { c, c, c, c };
        }
        REPORTER_ASSERT(r, memcmp(&in, &expected, sizeof(in)) == 0);
    }
}

DEF_TEST(SkSLInterpreterFunctions, r) {
    const char* src =
        "float sqr(float x) { return x * x; }\n"
        // Forward declared
        "float sub(float x, float y);\n"
        "float main(float x) { return sub(sqr(x), x); }\n"
        "float sub(float x, float y) { return x - y; }\n"

        // Different signatures
        "float dot(float2 a, float2 b) { return a.x*b.x + a.y*b.y; }\n"
        "float dot(float3 a, float3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }\n"
        "float dot3_test(float x) { return dot(float3(x, x + 1, x + 2), float3(1, -1, 2)); }\n"
        "float dot2_test(float x) { return dot(float2(x, x + 1), float2(1, -1)); }\n"
        "int fib(int i) { return (i < 2) ? 1 : fib(i - 1) + fib(i - 2); }";

    SkSL::Compiler compiler;
    SkSL::Program::Settings settings;
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(
                                                             SkSL::Program::kGeneric_Kind,
                                                             SkSL::String(src), settings);
    REPORTER_ASSERT(r, program);

    std::unique_ptr<SkSL::ByteCode> byteCode = compiler.toByteCode(*program);
    REPORTER_ASSERT(r, !compiler.errorCount());

    auto sub = byteCode->getFunction("sub");
    auto sqr = byteCode->getFunction("sqr");
    auto main = byteCode->getFunction("main");
    auto tan = byteCode->getFunction("tan");
    auto dot3 = byteCode->getFunction("dot3_test");
    auto dot2 = byteCode->getFunction("dot2_test");
    auto fib = byteCode->getFunction("fib");

    REPORTER_ASSERT(r, sub);
    REPORTER_ASSERT(r, sqr);
    REPORTER_ASSERT(r, main);
    REPORTER_ASSERT(r, !tan);
    REPORTER_ASSERT(r, dot3);
    REPORTER_ASSERT(r, dot2);
    REPORTER_ASSERT(r, fib);

    SkSL::Interpreter interpreter(std::move(program), std::move(byteCode), nullptr);
    float out = 0.0f;
    float in = 3.0f;
    interpreter.run(*main, (SkSL::Interpreter::Value*)&in, (SkSL::Interpreter::Value*)&out);
    REPORTER_ASSERT(r, out = 6.0f);

    interpreter.run(*dot3, (SkSL::Interpreter::Value*)&in, (SkSL::Interpreter::Value*)&out);
    REPORTER_ASSERT(r, out = 9.0f);

    interpreter.run(*dot2, (SkSL::Interpreter::Value*)&in, (SkSL::Interpreter::Value*)&out);
    REPORTER_ASSERT(r, out = -1.0f);

    int fibIn = 6;
    int fibOut = 0;
    interpreter.run(*fib, (SkSL::Interpreter::Value*)&fibIn, (SkSL::Interpreter::Value*)&fibOut);
    REPORTER_ASSERT(r, fibOut == 13);
}

DEF_TEST(SkSLInterpreterMathFunctions, r) {
    SkSL::Interpreter::Value value, expected;

    value = 0.0f; expected = 0.0f;
    test(r, "float main(float x) { return sin(x); }", &value, 1, &expected);
    test(r, "float main(float x) { return tan(x); }", &value, 1, &expected);

    value = 0.0f; expected = 1.0f;
    test(r, "float main(float x) { return cos(x); }", &value, 1, &expected);

    value = 25.0f; expected = 5.0f;
    test(r, "float main(float x) { return sqrt(x); }", &value, 1, &expected);
}

DEF_TEST(SkSLInterpreterMix, r) {
    SkSL::Interpreter::Value value, expected;

    value = 0.5f; expected = 0.0f;
    test(r, "float main(float x) { return mix(-10, 10, x); }", &value, 1, &expected);
    value = 0.75f; expected = 5.0f;
    test(r, "float main(float x) { return mix(-10, 10, x); }", &value, 1, &expected);
    value = 2.0f; expected = 30.0f;
    test(r, "float main(float x) { return mix(-10, 10, x); }", &value, 1, &expected);

    SkSL::Interpreter::Value valueVectors[]   = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f },
                             expectedVector[] = { 3.0f, 4.0f, 5.0f, 6.0f };
    test(r, "float4 main(float4 x, float4 y) { return mix(x, y, 0.5); }", valueVectors, 4,
         expectedVector);
}

DEF_TEST(SkSLInterpreterCross, r) {
    SkSL::Interpreter::Value args[] = { 1.0f, 4.0f, -6.0f, -2.0f, 7.0f, -3.0f };
    SkPoint3 cross = SkPoint3::CrossProduct(SkPoint3::Make(args[0].fFloat,
                                                           args[1].fFloat,
                                                           args[2].fFloat),
                                            SkPoint3::Make(args[3].fFloat,
                                                           args[4].fFloat,
                                                           args[5].fFloat));
    SkSL::Interpreter::Value expected[] = { cross.fX, cross.fY, cross.fZ };
    test(r, "float3 main(float3 x, float3 y) { return cross(x, y); }", args, 3, expected);
}

DEF_TEST(SkSLInterpreterDot, r) {
    SkSL::Interpreter::Value args[] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f };
    SkSL::Interpreter::Value expected = args[0].fFloat * args[2].fFloat +
                                        args[1].fFloat * args[3].fFloat;
    test(r, "float main(float2 x, float2 y) { return dot(x, y); }", args, 1, &expected);

    expected = args[0].fFloat * args[3].fFloat +
               args[1].fFloat * args[4].fFloat +
               args[2].fFloat * args[5].fFloat;
    test(r, "float main(float3 x, float3 y) { return dot(x, y); }", args, 1, &expected);

    expected = args[0].fFloat * args[4].fFloat +
               args[1].fFloat * args[5].fFloat +
               args[2].fFloat * args[6].fFloat +
               args[3].fFloat * args[7].fFloat;
    test(r, "float main(float4 x, float4 y) { return dot(x, y); }", args, 1, &expected);
}

static const SkSL::Type& type_of(const skjson::Value* value, SkSL::Compiler* compiler) {
    switch (value->getType()) {
        case skjson::Value::Type::kNumber: {
            float f = *value->as<skjson::NumberValue>();
            if (f == (float) (int) f) {
                return *compiler->context().fInt_Type;
            }
            return *compiler->context().fFloat_Type;
        }
        case skjson::Value::Type::kBool:
            return *compiler->context().fBool_Type;
        default:
            return *compiler->context().fVoid_Type;
    }
}

class JSONExternalValue : public SkSL::ExternalValue {
public:
    JSONExternalValue(const char* name, const skjson::Value* value, SkSL::Compiler* compiler)
        : INHERITED(name, type_of(value, compiler))
        , fValue(*value)
        , fCompiler(*compiler) {}

    bool canRead() const override {
        return type() != *fCompiler.context().fVoid_Type;
    }

    void read(void* target) override {
        if (type() == *fCompiler.context().fInt_Type) {
            *(int*) target = *fValue.as<skjson::NumberValue>();
        } else if (type() == *fCompiler.context().fFloat_Type) {
            *(float*) target = *fValue.as<skjson::NumberValue>();
        } else if (type() == *fCompiler.context().fBool_Type) {
            *(bool*) target = *fValue.as<skjson::BoolValue>();
        } else {
            SkASSERT(false);
        }
    }

    SkSL::ExternalValue* getChild(const char* name) const override {
        if (fValue.getType() == skjson::Value::Type::kObject) {
            const skjson::Value& v = fValue.as<skjson::ObjectValue>()[name];
            return (SkSL::ExternalValue*) fCompiler.takeOwnership(std::unique_ptr<Symbol>(
                                                      new JSONExternalValue(name, &v, &fCompiler)));
        }
        return nullptr;
    }

private:
    const skjson::Value& fValue;
    SkSL::Compiler& fCompiler;

    typedef SkSL::ExternalValue INHERITED;
};

class PointerExternalValue : public SkSL::ExternalValue {
public:
    PointerExternalValue(const char* name, const SkSL::Type& type, void* data, size_t size)
        : INHERITED(name, type)
        , fData(data)
        , fSize(size) {}

    bool canRead() const override {
        return true;
    }

    bool canWrite() const override {
        return true;
    }

    void read(void* target) override {
        memcpy(target, fData, fSize);
    }

    void write(void* src) override {
        memcpy(fData, src, fSize);
    }


private:
    void* fData;
    size_t fSize;

    typedef SkSL::ExternalValue INHERITED;
};

DEF_TEST(SkSLInterpreterExternalValues, r) {
    const char* json = "{ \"value1\": 12, \"child\": { \"value2\": true, \"value3\": 5.5 } }";
    skjson::DOM dom(json, strlen(json));
    SkSL::Compiler compiler;
    SkSL::Program::Settings settings;
    const char* src = "float main() {"
                      "    outValue = 152;"
                      "    if (root.child.value2)"
                      "        return root.value1 * root.child.value3;"
                      "    return -1;"
                      "}";
    compiler.registerExternalValue((SkSL::ExternalValue*) compiler.takeOwnership(
             std::unique_ptr<SkSL::Symbol>(new JSONExternalValue("root", &dom.root(), &compiler))));
    int32_t outValue = -1;
    compiler.registerExternalValue((SkSL::ExternalValue*) compiler.takeOwnership(
               std::unique_ptr<SkSL::Symbol>(new PointerExternalValue("outValue",
                                                                      *compiler.context().fInt_Type,
                                                                      &outValue,
                                                                      sizeof(outValue)))));
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(
                                                             SkSL::Program::kGeneric_Kind,
                                                             SkSL::String(src), settings);
    REPORTER_ASSERT(r, program);
    if (program) {
        std::unique_ptr<SkSL::ByteCode> byteCode = compiler.toByteCode(*program);
        REPORTER_ASSERT(r, !compiler.errorCount());
        if (compiler.errorCount() > 0) {
            printf("%s\n%s", src, compiler.errorText().c_str());
            return;
        }
        SkSL::ByteCodeFunction* main = byteCode->fFunctions[0].get();
        SkSL::Interpreter interpreter(std::move(program), std::move(byteCode));
        SkSL::Interpreter::Value out;
        interpreter.run(*main, nullptr, &out);
        REPORTER_ASSERT(r, out.fFloat == 66.0);
        REPORTER_ASSERT(r, outValue == 152);
    } else {
        printf("%s\n%s", src, compiler.errorText().c_str());
    }
}

DEF_TEST(SkSLInterpreterExternalValuesVector, r) {
    SkSL::Compiler compiler;
    SkSL::Program::Settings settings;
    const char* src = "void main() {"
                      "    value *= 2;"
                      "}";
    int32_t value[4] = { 1, 2, 3, 4 };
    compiler.registerExternalValue((SkSL::ExternalValue*) compiler.takeOwnership(
              std::unique_ptr<SkSL::Symbol>(new PointerExternalValue("value",
                                                                     *compiler.context().fInt4_Type,
                                                                     value,
                                                                     sizeof(value)))));
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(SkSL::Program::kGeneric_Kind,
                                                                     SkSL::String(src),
                                                                     settings);
    REPORTER_ASSERT(r, program);
    if (program) {
        std::unique_ptr<SkSL::ByteCode> byteCode = compiler.toByteCode(*program);
        REPORTER_ASSERT(r, !compiler.errorCount());
        if (compiler.errorCount() > 0) {
            printf("%s\n%s", src, compiler.errorText().c_str());
            return;
        }
        SkSL::ByteCodeFunction* main = byteCode->fFunctions[0].get();
        SkSL::Interpreter interpreter(std::move(program), std::move(byteCode));
        SkSL::Interpreter::Value out;
        interpreter.run(*main, nullptr, &out);
        REPORTER_ASSERT(r, value[0] == 2);
        REPORTER_ASSERT(r, value[1] == 4);
        REPORTER_ASSERT(r, value[2] == 6);
        REPORTER_ASSERT(r, value[3] == 8);
    } else {
        printf("%s\n%s", src, compiler.errorText().c_str());
    }
}

class FunctionExternalValue : public SkSL::ExternalValue {
public:
    FunctionExternalValue(const char* name, float(*function)(float), SkSL::Compiler& compiler)
        : INHERITED(name, *compiler.context().fFloat_Type)
        , fCompiler(compiler)
        , fFunction(function) {}

    bool canCall() const override {
        return true;
    }

    int callParameterCount() const override {
        return 1;
    }

    void getCallParameterTypes(const SkSL::Type** outTypes) const override {
        outTypes[0] = fCompiler.context().fFloat_Type.get();
    }

    void call(SkSL::Interpreter::Value* arguments, SkSL::Interpreter::Value* outReturn) override {
        outReturn[0].fFloat = fFunction(arguments[0].fFloat);
    }

private:
    SkSL::Compiler& fCompiler;

    float (*fFunction)(float);

    typedef SkSL::ExternalValue INHERITED;
};

DEF_TEST(SkSLInterpreterExternalValuesCall, r) {
    SkSL::Compiler compiler;
    SkSL::Program::Settings settings;
    const char* src = "float main() {"
                      "    return external(25);"
                      "}";
    compiler.registerExternalValue((SkSL::ExternalValue*) compiler.takeOwnership(
            std::unique_ptr<SkSL::Symbol>(new FunctionExternalValue("external",
                                                                    [] (float x) {
                                                                        return (float) sqrt(x);
                                                                    },
                                                                    compiler))));
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(SkSL::Program::kGeneric_Kind,
                                                                     SkSL::String(src),
                                                                     settings);
    REPORTER_ASSERT(r, program);
    if (program) {
        std::unique_ptr<SkSL::ByteCode> byteCode = compiler.toByteCode(*program);
        REPORTER_ASSERT(r, !compiler.errorCount());
        if (compiler.errorCount() > 0) {
            printf("%s\n%s", src, compiler.errorText().c_str());
            return;
        }
        SkSL::ByteCodeFunction* main = byteCode->fFunctions[0].get();
        SkSL::Interpreter interpreter(std::move(program), std::move(byteCode));
        SkSL::Interpreter::Value out;
        interpreter.run(*main, nullptr, &out);
        REPORTER_ASSERT(r, out.fFloat == 5.0);
    } else {
        printf("%s\n%s", src, compiler.errorText().c_str());
    }
}

class VectorFunctionExternalValue : public SkSL::ExternalValue {
public:
    VectorFunctionExternalValue(const char* name, void(*function)(float[4], float[4]),
                                SkSL::Compiler& compiler)
        : INHERITED(name, *compiler.context().fFloat4_Type)
        , fCompiler(compiler)
        , fFunction(function) {}

    bool canCall() const override {
        return true;
    }

    int callParameterCount() const override {
        return 1;
    }

    void getCallParameterTypes(const SkSL::Type** outTypes) const override {
        outTypes[0] = fCompiler.context().fFloat4_Type.get();
    }

    void call(SkSL::Interpreter::Value* arguments, SkSL::Interpreter::Value* outReturn) override {
        fFunction((float*) arguments, (float*) outReturn);
    }

private:
    SkSL::Compiler& fCompiler;

    void (*fFunction)(float[4], float[4]);

    typedef SkSL::ExternalValue INHERITED;
};


DEF_TEST(SkSLInterpreterExternalValuesVectorCall, r) {
    SkSL::Compiler compiler;
    SkSL::Program::Settings settings;
    const char* src = "float4 main() {"
                      "    return external(float4(1, 4, 9, 16));"
                      "}";
    compiler.registerExternalValue((SkSL::ExternalValue*) compiler.takeOwnership(
            std::unique_ptr<SkSL::Symbol>(new VectorFunctionExternalValue("external",
                                                                    [] (float in[4], float out[4]) {
                                                                        out[0] = sqrt(in[0]);
                                                                        out[1] = sqrt(in[1]);
                                                                        out[2] = sqrt(in[2]);
                                                                        out[3] = sqrt(in[3]);
                                                                    },
                                                                    compiler))));
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(SkSL::Program::kGeneric_Kind,
                                                                     SkSL::String(src),
                                                                     settings);
    REPORTER_ASSERT(r, program);
    if (program) {
        std::unique_ptr<SkSL::ByteCode> byteCode = compiler.toByteCode(*program);
        REPORTER_ASSERT(r, !compiler.errorCount());
        if (compiler.errorCount() > 0) {
            printf("%s\n%s", src, compiler.errorText().c_str());
            return;
        }
        SkSL::ByteCodeFunction* main = byteCode->fFunctions[0].get();
        SkSL::Interpreter interpreter(std::move(program), std::move(byteCode));
        SkSL::Interpreter::Value out[4];
        interpreter.run(*main, nullptr, out);
        REPORTER_ASSERT(r, out[0].fFloat == 1.0);
        REPORTER_ASSERT(r, out[1].fFloat == 2.0);
        REPORTER_ASSERT(r, out[2].fFloat == 3.0);
        REPORTER_ASSERT(r, out[3].fFloat == 4.0);
    } else {
        printf("%s\n%s", src, compiler.errorText().c_str());
    }
}
