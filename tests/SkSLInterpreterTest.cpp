/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPoint3.h"
#include "src/sksl/SkSLByteCode.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLExternalValue.h"
#include "src/utils/SkJSON.h"

#include "tests/Test.h"

static bool nearly_equal(const float a[], const float b[], int count) {
    for (int i = 0; i < count; ++i) {
        if (!SkScalarNearlyEqual(a[i], b[i])) {
            return false;
        }
    }
    return true;
}

void test(skiatest::Reporter* r, const char* src, float* in, int expectedCount, float* expected,
          bool exactCompare = true) {
    SkSL::Compiler compiler;
    SkSL::Program::Settings settings;
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(
                                                             SkSL::Program::kGeneric_Kind,
                                                             SkSL::String(src), settings);
    REPORTER_ASSERT(r, program);
    if (program) {
        std::unique_ptr<SkSL::ByteCode> byteCode = compiler.toByteCode(*program);
        program.reset();
        REPORTER_ASSERT(r, !compiler.errorCount());
        if (compiler.errorCount() > 0) {
            printf("%s\n%s", src, compiler.errorText().c_str());
            return;
        }
        const SkSL::ByteCodeFunction* main = byteCode->getFunction("main");
        std::unique_ptr<float[]> out = std::unique_ptr<float[]>(new float[expectedCount]);
        SkAssertResult(byteCode->run(main, in, out.get(), 1, nullptr, 0));
        bool valid = exactCompare ? !memcmp(out.get(), expected, sizeof(float) * expectedCount)
                                  : nearly_equal(out.get(), expected, expectedCount);
        if (!valid) {
            printf("for program: %s\n", src);
            printf("    expected (");
            const char* separator = "";
            for (int i = 0; i < expectedCount; ++i) {
                printf("%s%f", separator, expected[i]);
                separator = ", ";
            }
            printf("), but received (");
            separator = "";
            for (int i = 0; i < expectedCount; ++i) {
                printf("%s%f", separator, out.get()[i]);
                separator = ", ";
            }
            printf(")\n");
            main->disassemble();
        }
        REPORTER_ASSERT(r, valid);
    } else {
        printf("%s\n%s", src, compiler.errorText().c_str());
    }
}

void vec_test(skiatest::Reporter* r, const char* src) {
    // Test on four different vectors (with varying orderings to get divergent control flow)
    const float input[16] = { 1, 2, 3, 4,
                              4, 3, 2, 1,
                              7, 5, 8, 6,
                              6, 8, 5, 7 };

    SkSL::Compiler compiler;
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(
            SkSL::Program::kGeneric_Kind, SkSL::String(src), SkSL::Program::Settings());
    if (!program) {
        REPORT_FAILURE(r, "!program", SkString(compiler.errorText().c_str()));
        return;
    }

    std::unique_ptr<SkSL::ByteCode> byteCode = compiler.toByteCode(*program);
    if (compiler.errorCount() > 0) {
        REPORT_FAILURE(r, "!toByteCode", SkString(compiler.errorText().c_str()));
        return;
    }

    const SkSL::ByteCodeFunction* main = byteCode->getFunction("main");

    float out_s[16], out_v[16];
    memcpy(out_s, input, sizeof(out_s));
    memcpy(out_v, input, sizeof(out_v));

    // First run in scalar mode to determine the expected output
    for (int i = 0; i < 4; ++i) {
        SkAssertResult(byteCode->run(main, out_s + i * 4, nullptr, 1, nullptr, 0));
    }

    // Now run in parallel and compare results
    SkAssertResult(byteCode->run(main, out_v, nullptr, 4, nullptr, 0));
    if (memcmp(out_s, out_v, sizeof(out_s)) != 0) {
        printf("for program: %s\n", src);
        for (int i = 0; i < 4; ++i) {
            printf("(%g %g %g %g) -> (%g %g %g %g), expected (%g %g %g %g)\n",
                    input[4*i + 0], input[4*i + 1], input[4*i + 2], input[4*i + 3],
                    out_v[4*i + 0], out_v[4*i + 1], out_v[4*i + 2], out_v[4*i + 3],
                    out_s[4*i + 0], out_s[4*i + 1], out_s[4*i + 2], out_s[4*i + 3]);
        }
        main->disassemble();
        REPORT_FAILURE(r, "VecInterpreter mismatch", SkString());
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
        program.reset();
        REPORTER_ASSERT(r, !compiler.errorCount());
        if (compiler.errorCount() > 0) {
            printf("%s\n%s", src, compiler.errorText().c_str());
            return;
        }
        const SkSL::ByteCodeFunction* main = byteCode->getFunction("main");
        float inoutColor[4] = { inR, inG, inB, inA };
        SkAssertResult(byteCode->run(main, inoutColor, nullptr, 1, nullptr, 0));
        if (inoutColor[0] != expectedR || inoutColor[1] != expectedG ||
            inoutColor[2] != expectedB || inoutColor[3] != expectedA) {
            printf("for program: %s\n", src);
            printf("    expected (%f, %f, %f, %f), but received (%f, %f, %f, %f)\n", expectedR,
                   expectedG, expectedB, expectedA, inoutColor[0], inoutColor[1], inoutColor[2],
                   inoutColor[3]);
            main->disassemble();
        }
        REPORTER_ASSERT(r, inoutColor[0] == expectedR);
        REPORTER_ASSERT(r, inoutColor[1] == expectedG);
        REPORTER_ASSERT(r, inoutColor[2] == expectedB);
        REPORTER_ASSERT(r, inoutColor[3] == expectedA);
    } else {
        printf("%s\n%s", src, compiler.errorText().c_str());
    }

    // Do additional testing of 4x1 vs 1x4 to stress divergent control flow, etc.
    vec_test(r, src);
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

DEF_TEST(SkSLInterpreterAnd, r) {
    test(r, "void main(inout half4 color) { if (color.r > color.g && color.g > color.b) "
            "color = half4(color.a); }", 2, 1, 0, 3, 3, 3, 3, 3);
    test(r, "void main(inout half4 color) { if (color.r > color.g && color.g > color.b) "
            "color = half4(color.a); }", 1, 1, 0, 3, 1, 1, 0, 3);
    test(r, "void main(inout half4 color) { if (color.r > color.g && color.g > color.b) "
            "color = half4(color.a); }", 2, 1, 1, 3, 2, 1, 1, 3);
    test(r, "int global; bool update() { global = 123; return true; }"
            "void main(inout half4 color) { global = 0;  if (color.r > color.g && update()) "
            "color = half4(color.a); color.a = global; }", 2, 1, 1, 3, 3, 3, 3, 123);
    test(r, "int global; bool update() { global = 123; return true; }"
            "void main(inout half4 color) { global = 0;  if (color.r > color.g && update()) "
            "color = half4(color.a); color.a = global; }", 1, 1, 1, 3, 1, 1, 1, 0);
}

DEF_TEST(SkSLInterpreterOr, r) {
    test(r, "void main(inout half4 color) { if (color.r > color.g || color.g > color.b) "
            "color = half4(color.a); }", 2, 1, 0, 3, 3, 3, 3, 3);
    test(r, "void main(inout half4 color) { if (color.r > color.g || color.g > color.b) "
            "color = half4(color.a); }", 1, 1, 0, 3, 3, 3, 3, 3);
    test(r, "void main(inout half4 color) { if (color.r > color.g || color.g > color.b) "
            "color = half4(color.a); }", 1, 1, 1, 3, 1, 1, 1, 3);
    test(r, "int global; bool update() { global = 123; return true; }"
            "void main(inout half4 color) { global = 0;  if (color.r > color.g || update()) "
            "color = half4(color.a); color.a = global; }", 1, 1, 1, 3, 3, 3, 3, 123);
    test(r, "int global; bool update() { global = 123; return true; }"
            "void main(inout half4 color) { global = 0;  if (color.r > color.g || update()) "
            "color = half4(color.a); color.a = global; }", 2, 1, 1, 3, 3, 3, 3, 0);
}

DEF_TEST(SkSLInterpreterMatrix, r) {
    float in[16];
    float expected[16];

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

    // Initialize 16 values to be used as inputs to matrix tests
    for (int i = 0; i < 16; ++i) { in[i] = (float)i; }

    // M+M, M-S, S-M
    for (int i = 0; i < 16; ++i) { expected[i] = (float)(2 * i); }
    test(r, "float4x4 main(float4x4 m) { return m + m; }", in, 16, expected);
    for (int i = 0; i < 16; ++i) { expected[i] = (float)(i + 3); }
    test(r, "float4x4 main(float4x4 m) { return m + 3.0; }", in, 16, expected);
    test(r, "float4x4 main(float4x4 m) { return 3.0 + m; }", in, 16, expected);

    // M-M, M-S, S-M
    for (int i = 0; i < 8; ++i) { expected[i] = 8.0f; }
    test(r, "float4x2 main(float4x2 m1, float4x2 m2) { return m2 - m1; }", in, 8, expected);
    for (int i = 0; i < 16; ++i) { expected[i] = (float)(i - 3); }
    test(r, "float4x4 main(float4x4 m) { return m - 3.0; }", in, 16, expected);
    for (int i = 0; i < 16; ++i) { expected[i] = (float)(3 - i); }
    test(r, "float4x4 main(float4x4 m) { return 3.0 - m; }", in, 16, expected);

    // M*S, S*M, M/S, S/M
    for (int i = 0; i < 16; ++i) { expected[i] = (float)(i * 3); }
    test(r, "float4x4 main(float4x4 m) { return m * 3.0; }", in, 16, expected);
    test(r, "float4x4 main(float4x4 m) { return 3.0 * m; }", in, 16, expected);
    for (int i = 0; i < 16; ++i) { expected[i] = (float)(i) / 2.0f; }
    test(r, "float4x4 main(float4x4 m) { return m / 2.0; }", in, 16, expected);
    for (int i = 0; i < 16; ++i) { expected[i] = 1.0f / (float)(i + 1); }
    test(r, "float4x4 main(float4x4 m) { return 1.0 / (m + 1); }", in, 16, expected);

#if 0
    // Matrix negation - legal in GLSL, not in SkSL?
    for (int i = 0; i < 16; ++i) { expected[i] = (float)(-i); }
    test(r, "float4x4 main(float4x4 m) { return -m; }", in, 16, expected);
#endif

    // M*V, V*M
    for (int i = 0; i < 4; ++i) {
        expected[i] = 12.0f*i + 13.0f*(i+4) + 14.0f*(i+8);
    }
    test(r, "float4 main(float3x4 m, float3 v) { return m * v; }", in, 4, expected);
    for (int i = 0; i < 4; ++i) {
        expected[i] = 12.0f*(3*i) + 13.0f*(3*i+1) + 14.0f*(3*i+2);
    }
    test(r, "float4 main(float4x3 m, float3 v) { return v * m; }", in, 4, expected);

    // M*M
    {
        SkMatrix44 m;
        m.setColMajorf(in);
        SkMatrix44 m2;
        for (int i = 0; i < 16; ++i) {
            m2.set(i % 4, i / 4, (i + 4) % 16);
        }
        m.setConcat(m, m2);
        // Rearrange the columns on the RHS so we detect left-hand/right-hand errors
        test(r, "float4x4 main(float4x4 m) { return m * float4x4(m[1], m[2], m[3], m[0]); }",
             in, 16, (float*)&m);
    }
}

DEF_TEST(SkSLInterpreterTernary, r) {
    test(r, "void main(inout half4 color) { color.r = color.g > color.b ? color.g : color.b; }",
         0, 1, 2, 0, 2, 1, 2, 0);
    test(r, "void main(inout half4 color) { color.r = color.g > color.b ? color.g : color.b; }",
         0, 3, 2, 0, 3, 3, 2, 0);
}

DEF_TEST(SkSLInterpreterCast, r) {
    union Val {
        float    f;
        uint32_t u;
        int32_t  s;
    };

    Val input[2];
    Val expected[2];

    input[0].s = 3;
    input[1].s = -5;
    expected[0].f = 3.0f;
    expected[1].f = -5.0f;
    test(r, "float  main(int  x) { return float (x); }", (float*)input, 1, (float*)expected);
    test(r, "float2 main(int2 x) { return float2(x); }", (float*)input, 2, (float*)expected);

    input[0].u = 3;
    input[1].u = 5;
    expected[0].f = 3.0f;
    expected[1].f = 5.0f;
    test(r, "float  main(uint  x) { return float (x); }", (float*)input, 1, (float*)expected);
    test(r, "float2 main(uint2 x) { return float2(x); }", (float*)input, 2, (float*)expected);

    input[0].f = 3.0f;
    input[1].f = -5.0f;
    expected[0].s = 3;
    expected[1].s = -5;
    test(r, "int  main(float  x) { return int (x); }", (float*)input, 1, (float*)expected);
    test(r, "int2 main(float2 x) { return int2(x); }", (float*)input, 2, (float*)expected);

    input[0].s = 3;
    expected[0].f = 3.0f;
    expected[1].f = 3.0f;
    test(r, "float2 main(int x) { return float2(x); }", (float*)input, 2, (float*)expected);
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
    test(r, "void main(inout half4 color) { while (color.r < 8) { color.r++; } }",
         1, 2, 3, 4, 8, 2, 3, 4);
    test(r, "void main(inout half4 color) { while (color.r < 1) color.r += 0.25; }", 0, 0, 0, 0, 1,
         0, 0, 0);
    test(r, "void main(inout half4 color) { while (color.r > 1) color.r -= 0.25; }", 0, 0, 0, 0, 0,
         0, 0, 0);
    test(r, "void main(inout half4 color) { while (true) { color.r += 0.5; "
         "if (color.r > 5) break; } }", 0, 0, 0, 0, 5.5, 0, 0, 0);
    test(r, "void main(inout half4 color) { while (color.r < 10) { color.r += 0.5; "
            "if (color.r < 5) continue; break; } }", 0, 0, 0, 0, 5, 0, 0, 0);
    test(r,
         "void main(inout half4 color) {"
         "    while (true) {"
         "        if (color.r > 4) { break; }"
         "        while (true) { color.a = 1; break; }"
         "        break;"
         "    }"
         "}",
         6, 5, 4, 3, 6, 5, 4, 3);
}

DEF_TEST(SkSLInterpreterDo, r) {
    test(r, "void main(inout half4 color) { do color.r += 0.25; while (color.r < 1); }", 0, 0, 0, 0,
         1, 0, 0, 0);
    test(r, "void main(inout half4 color) { do color.r -= 0.25; while (color.r > 1); }", 0, 0, 0, 0,
         -0.25, 0, 0, 0);
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
    test(r, "float main(float x) { return x * x; }", &value1, 1, &expected1);
    float value2[2] = { 5, 25 };
    float expected2[2] = { 25, 625 };
    test(r, "float2 main(float x, float y) { return float2(x * x, y * y); }", value2, 2, expected2);
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

    {
        SkIRect in = SkIRect::MakeXYWH(10, 10, 20, 30);
        int out = 0;
        SkAssertResult(byteCode->run(rect_height, (float*)&in, (float*)&out, 1, (float*)gRects, 16));
        REPORTER_ASSERT(r, out == 30);
    }

    {
        int in[2] = { 15, 25 };
        RectAndColor out;
        SkAssertResult(byteCode->run(make_blue_rect, (float*)in, (float*)&out, 1, (float*)gRects, 16));
        REPORTER_ASSERT(r, out.fRect.width() == 15);
        REPORTER_ASSERT(r, out.fRect.height() == 25);
        SkColor4f blue = { 0.0f, 1.0f, 0.0f, 1.0f };
        REPORTER_ASSERT(r, out.fColor == blue);
    }

    {
        int in[15] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
        int out = 0;
        SkAssertResult(byteCode->run(median, (float*)in, (float*)&out, 1, (float*)gRects, 16));
        REPORTER_ASSERT(r, out == 8);
    }

    {
        float in[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
        float out[8] = { 0 };
        SkAssertResult(byteCode->run(sums, in, out, 1, (float*)gRects, 16));
        for (int i = 0; i < 8; ++i) {
            REPORTER_ASSERT(r, out[i] == static_cast<float>((i + 1) * (i + 2) / 2));
        }
    }

    {
        int in = 2;
        SkIRect out = SkIRect::MakeEmpty();
        SkAssertResult(byteCode->run(get_rect, (float*)&in, (float*)&out, 1, (float*)gRects, 16));
        REPORTER_ASSERT(r, out == gRects[2]);
    }

    {
        ManyRects in;
        memset(&in, 0, sizeof(in));
        in.fNumRects = 2;
        SkAssertResult(byteCode->run(fill_rects, (float*)&in, nullptr, 1, (float*)gRects, 16));
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

static void expect_failure(skiatest::Reporter* r, const char* src) {
    SkSL::Compiler compiler;
    auto program = compiler.convertProgram(SkSL::Program::kGeneric_Kind, SkSL::String(src),
                                           SkSL::Program::Settings());
    REPORTER_ASSERT(r, program);

    auto byteCode = compiler.toByteCode(*program);
    REPORTER_ASSERT(r, compiler.errorCount() > 0);
    REPORTER_ASSERT(r, !byteCode);
}

static void expect_run_failure(skiatest::Reporter* r, const char* src, float* in) {
    SkSL::Compiler compiler;
    auto program = compiler.convertProgram(SkSL::Program::kGeneric_Kind, SkSL::String(src),
                                           SkSL::Program::Settings());
    REPORTER_ASSERT(r, program);

    auto byteCode = compiler.toByteCode(*program);
    REPORTER_ASSERT(r, byteCode);

    bool result = byteCode->run(byteCode->getFunction("main"), in, nullptr, 1, nullptr, 0);
    REPORTER_ASSERT(r, !result);
}

DEF_TEST(SkSLInterpreterRestrictFunctionCalls, r) {
    // Ensure that simple recursion is not allowed
    expect_failure(r, "float main() { return main() + 1; }");

    // Ensure that calls to undefined functions are not allowed (to prevent mutual recursion)
    expect_failure(r, "float foo(); float bar() { return foo(); } float foo() { return bar(); }");

    // returns are not allowed inside conditionals (or loops, which are effectively the same thing)
    expect_failure(r, "float main(float x, float y) { if (x < y) { return x; } return y; }");
    expect_failure(r, "float main(float x) { while (x > 1) { return x; } return 0; }");
}

DEF_TEST(SkSLInterpreterArrayBounds, r) {
    // Out of bounds array access at compile time
    expect_failure(r, "float main(float x[4]) { return x[-1]; }");
    expect_failure(r, "float2 main(float2 x[2]) { return x[2]; }");

    // Out of bounds array access at runtime is pinned, and we don't update any inout data
    float in[3] = { -1.0f, 1.0f, 2.0f };
    expect_run_failure(r, "void main(inout float data[3]) { data[int(data[0])] = 0; }", in);
    REPORTER_ASSERT(r, in[0] == -1.0f && in[1] == 1.0f && in[2] == 2.0f);

    in[0] = 3.0f;
    expect_run_failure(r, "void main(inout float data[3]) { data[int(data[0])] = 0; }", in);
    REPORTER_ASSERT(r, in[0] == 3.0f && in[1] == 1.0f && in[2] == 2.0f);
}

DEF_TEST(SkSLInterpreterFunctions, r) {
    const char* src =
        "float sqr(float x) { return x * x; }\n"
        "float sub(float x, float y) { return x - y; }\n"
        "float main(float x) { return sub(sqr(x), x); }\n"

        // Different signatures
        "float dot(float2 a, float2 b) { return a.x*b.x + a.y*b.y; }\n"
        "float dot(float3 a, float3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }\n"
        "float dot3_test(float x) { return dot(float3(x, x + 1, x + 2), float3(1, -1, 2)); }\n"
        "float dot2_test(float x) { return dot(float2(x, x + 1), float2(1, -1)); }\n";

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

    REPORTER_ASSERT(r, sub);
    REPORTER_ASSERT(r, sqr);
    REPORTER_ASSERT(r, main);
    REPORTER_ASSERT(r, !tan);
    REPORTER_ASSERT(r, dot3);
    REPORTER_ASSERT(r, dot2);

    float out = 0.0f;
    float in = 3.0f;
    SkAssertResult(byteCode->run(main, &in, &out, 1, nullptr, 0));
    REPORTER_ASSERT(r, out = 6.0f);

    SkAssertResult(byteCode->run(dot3, &in, &out, 1, nullptr, 0));
    REPORTER_ASSERT(r, out = 9.0f);

    SkAssertResult(byteCode->run(dot2, &in, &out, 1, nullptr, 0));
    REPORTER_ASSERT(r, out = -1.0f);
}

DEF_TEST(SkSLInterpreterOutParams, r) {
    test(r,
         "void oneAlpha(inout half4 color) { color.a = 1; }"
         "void main(inout half4 color) { oneAlpha(color); }",
         0, 0, 0, 0, 0, 0, 0, 1);
    test(r,
         "half2 tricky(half x, half y, inout half2 color, half z) {"
         "    color.xy = color.yx;"
         "    return half2(x + y, z);"
         "}"
         "void main(inout half4 color) {"
         "    half2 t = tricky(1, 2, color.rb, 5);"
         "    color.ga = t;"
         "}",
         1, 2, 3, 4, 3, 3, 1, 5);
}

DEF_TEST(SkSLInterpreterMathFunctions, r) {
    float value[4], expected[4];

    value[0] = 0.0f; expected[0] = 0.0f;
    test(r, "float main(float x) { return sin(x); }", value, 1, expected);
    test(r, "float main(float x) { return tan(x); }", value, 1, expected);

    value[0] = 0.0f; expected[0] = 1.0f;
    test(r, "float main(float x) { return cos(x); }", value, 1, expected);

    value[0] = 25.0f; expected[0] = 5.0f;
    test(r, "float main(float x) { return sqrt(x); }", value, 1, expected);

    value[0] = 90.0f; expected[0] = sk_float_degrees_to_radians(value[0]);
    test(r, "float main(float x) { return radians(x); }", value, 1, expected);

    value[0] = 1.0f; value[1] = -1.0f;
    expected[0] = 1.0f / SK_FloatSqrt2; expected[1] = -1.0f / SK_FloatSqrt2;
    test(r, "float2 main(float2 x) { return normalize(x); }", value, 2, expected);
}

DEF_TEST(SkSLInterpreterVoidFunction, r) {
    test(r,
         "half x; void foo() { x = 1.0; }"
         "void main(inout half4 color) { foo(); color.r = x; }",
         0, 0, 0, 0, 1, 0, 0, 0);
}

DEF_TEST(SkSLInterpreterMix, r) {
    float value, expected;

    value = 0.5f; expected = 0.0f;
    test(r, "float main(float x) { return mix(-10, 10, x); }", &value, 1, &expected);
    value = 0.75f; expected = 5.0f;
    test(r, "float main(float x) { return mix(-10, 10, x); }", &value, 1, &expected);
    value = 2.0f; expected = 30.0f;
    test(r, "float main(float x) { return mix(-10, 10, x); }", &value, 1, &expected);

    float valueVectors[]   = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f },
          expectedVector[] = { 3.0f, 4.0f, 5.0f, 6.0f };
    test(r, "float4 main(float4 x, float4 y) { return mix(x, y, 0.5); }", valueVectors, 4,
         expectedVector);
}

DEF_TEST(SkSLInterpreterCross, r) {
    float args[] = { 1.0f, 4.0f, -6.0f, -2.0f, 7.0f, -3.0f };
    SkPoint3 cross = SkPoint3::CrossProduct(SkPoint3::Make(args[0], args[1], args[2]),
                                            SkPoint3::Make(args[3], args[4], args[5]));
    float expected[] = { cross.fX, cross.fY, cross.fZ };
    test(r, "float3 main(float3 x, float3 y) { return cross(x, y); }", args, 3, expected);
}

DEF_TEST(SkSLInterpreterInverse, r) {
    {
        SkMatrix m;
        m.setRotate(30).postScale(1, 2);
        float args[4] = { m[0], m[3], m[1], m[4] };
        SkAssertResult(m.invert(&m));
        float expt[4] = { m[0], m[3], m[1], m[4] };
        test(r, "float2x2 main(float2x2 m) { return inverse(m); }", args, 4, expt, false);
    }
    {
        SkMatrix m;
        m.setRotate(30).postScale(1, 2).postTranslate(1, 2);
        float args[9] = { m[0], m[3], m[6], m[1], m[4], m[7], m[2], m[5], m[8] };
        SkAssertResult(m.invert(&m));
        float expt[9] = { m[0], m[3], m[6], m[1], m[4], m[7], m[2], m[5], m[8] };
        test(r, "float3x3 main(float3x3 m) { return inverse(m); }", args, 9, expt, false);
    }
    {
        float args[16], expt[16];
        SkMatrix44 m;
        // just some crazy thing that is invertible
        m.set4x4(1, 2, 3, 4, 1, 2, 0, 3, 1, 0, 1, 4, 1, 3, 2, 0);
        m.asColMajorf(args);
        SkAssertResult(m.invert(&m));
        m.asColMajorf(expt);
        test(r, "float4x4 main(float4x4 m) { return inverse(m); }", args, 16, expt, false);
    }
}

DEF_TEST(SkSLInterpreterDot, r) {
    float args[] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f };
    float expected = args[0] * args[2] +
                     args[1] * args[3];
    test(r, "float main(float2 x, float2 y) { return dot(x, y); }", args, 1, &expected);

    expected = args[0] * args[3] +
               args[1] * args[4] +
               args[2] * args[5];
    test(r, "float main(float3 x, float3 y) { return dot(x, y); }", args, 1, &expected);

    expected = args[0] * args[4] +
               args[1] * args[5] +
               args[2] * args[6] +
               args[3] * args[7];
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

    void read(int /*unusedIndex*/, float* target) override {
        if (type() == *fCompiler.context().fInt_Type) {
            *(int*) target = *fValue.as<skjson::NumberValue>();
        } else if (type() == *fCompiler.context().fFloat_Type) {
            *(float*) target = *fValue.as<skjson::NumberValue>();
        } else if (type() == *fCompiler.context().fBool_Type) {
            // ByteCode "booleans" are actually bit-masks
            *(int*) target = *fValue.as<skjson::BoolValue>() ? ~0 : 0;
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

    void read(int /*unusedIndex*/, float* target) override {
        memcpy(target, fData, fSize);
    }

    void write(int /*unusedIndex*/, float* src) override {
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
                      "    return root.child.value2 ? root.value1 * root.child.value3 : -1;"
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
        const SkSL::ByteCodeFunction* main = byteCode->getFunction("main");
        float out;
        SkAssertResult(byteCode->run(main, nullptr, &out, 1, nullptr, 0));
        REPORTER_ASSERT(r, out == 66.0);
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
        const SkSL::ByteCodeFunction* main = byteCode->getFunction("main");
        SkAssertResult(byteCode->run(main, nullptr, nullptr, 1, nullptr, 0));
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

    void call(int /*unusedIndex*/, float* arguments, float* outReturn) override {
        outReturn[0] = fFunction(arguments[0]);
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
        const SkSL::ByteCodeFunction* main = byteCode->getFunction("main");
        float out;
        SkAssertResult(byteCode->run(main, nullptr, &out, 1, nullptr, 0));
        REPORTER_ASSERT(r, out == 5.0);
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

    void call(int /*unusedIndex*/, float* arguments, float* outReturn) override {
        fFunction(arguments, outReturn);
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
        const SkSL::ByteCodeFunction* main = byteCode->getFunction("main");
        float out[4];
        SkAssertResult(byteCode->run(main, nullptr, out, 1, nullptr, 0));
        REPORTER_ASSERT(r, out[0] == 1.0);
        REPORTER_ASSERT(r, out[1] == 2.0);
        REPORTER_ASSERT(r, out[2] == 3.0);
        REPORTER_ASSERT(r, out[3] == 4.0);
    } else {
        printf("%s\n%s", src, compiler.errorText().c_str());
    }
}
