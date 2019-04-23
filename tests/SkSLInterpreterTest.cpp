/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSLCompiler.h"
#include "SkSLInterpreter.h"

#include "Test.h"

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
        SkSL::Interpreter::Value* out = interpreter.run(*main, in, nullptr);
        bool valid = !memcmp(out, expected, sizeof(SkSL::Interpreter::Value) * expectedCount);
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
                printf("%s%f", separator, out[i].fFloat);
                separator = ", ";
            }
            printf(")\n");
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
    test(r, "void main(inout half4 color) { int a = 8; int b = 3; a %= b; color.r = a; }", 0, 0, 0,
         0, 2, 0, 0, 0);
    test(r, "void main(inout half4 color) { int a = 8; int b = 3; color.r = a % b; }", 0, 0, 0, 0,
         2, 0, 0, 0);
    test(r, "void main(inout half4 color) { int2 a = int2(8, 10); a %= 6; color.rg = a; }", 0, 0, 0,
         0, 2, 4, 0, 0);
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
