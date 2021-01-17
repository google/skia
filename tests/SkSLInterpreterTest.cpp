/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkM44.h"
#include "src/sksl/SkSLByteCode.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLExternalFunction.h"
#include "src/sksl/SkSLVMGenerator.h"
#include "src/utils/SkJSON.h"

#include "tests/Test.h"

#if defined(SK_ENABLE_SKSL_INTERPRETER)

struct ProgramBuilder {
    ProgramBuilder(skiatest::Reporter* r, const char* src)
            : fCaps(GrContextOptions{}), fCompiler(&fCaps) {
        SkSL::Program::Settings settings;
        // The SkSL inliner is well tested in other contexts. Here, we disable inlining entirely,
        // to stress-test the VM generator's handling of function calls with varying signatures.
        settings.fInlineThreshold = 0;
        // For convenience, so we can test functions other than (and not called by) main.
        settings.fRemoveDeadFunctions = false;

        fProgram = fCompiler.convertProgram(SkSL::Program::kGeneric_Kind, SkSL::String(src),
                                            settings);
        if (!fProgram) {
            ERRORF(r, "Program failed to compile:\n%s\n%s\n", src, fCompiler.errorText().c_str());
        }
    }

    operator bool() const { return fProgram != nullptr; }
    SkSL::Program& operator*() { return *fProgram; }

    GrShaderCaps fCaps;
    SkSL::Compiler fCompiler;
    std::unique_ptr<SkSL::Program> fProgram;
};

struct ByteCodeBuilder {
    ByteCodeBuilder(skiatest::Reporter* r, const char* src) : fProgram(r, src), fByteCode(nullptr) {
        if (fProgram) {
            fByteCode = fProgram.fCompiler.toByteCode(*fProgram);
            if (!fByteCode) {
                ERRORF(r, "Program failed to compile:\n%s\n%s\n", src,
                       fProgram.fCompiler.errorText().c_str());
            }
        }
    }

    operator bool() const { return fByteCode != nullptr; }
    SkSL::ByteCode* operator->() { return fByteCode.get(); }

    ProgramBuilder fProgram;
    std::unique_ptr<SkSL::ByteCode> fByteCode;
};

static void verify_values(skiatest::Reporter* r,
                          const char* src,
                          const float* actual,
                          const float* expected,
                          int N,
                          bool exactCompare) {
    auto exact_equiv = [](float x, float y) {
        return x == y
            || (isnan(x) && isnan(y));
    };

    bool valid = true;
    for (int i = 0; i < N; ++i) {
        if (exactCompare && !exact_equiv(actual[i], expected[i])) {
            valid = false;
        }
        if (!exactCompare && !SkScalarNearlyEqual(actual[i], expected[i])) {
            valid = false;
        }
    }

    if (!valid) {
        printf("for program: %s\n", src);
        printf("    expected (");
        const char* separator = "";
        for (int i = 0; i < N; ++i) {
            printf("%s%f", separator, expected[i]);
            separator = ", ";
        }
        printf("), but received (");
        separator = "";
        for (int i = 0; i < N; ++i) {
            printf("%s%f", separator, actual[i]);
            separator = ", ";
        }
        printf(")\n");
    }
    REPORTER_ASSERT(r, valid);
}

void test_skvm(skiatest::Reporter* r, const char* src, float* in, const float* expected,
               bool exactCompare) {
    ProgramBuilder program(r, src);
    if (!program) { return; }

    const SkSL::FunctionDefinition* main = SkSL::Program_GetFunction(*program, "main");
    REPORTER_ASSERT(r, main);

    skvm::Builder b;
    SkSL::SkVMSignature sig;
    SkSL::ProgramToSkVM(*program, *main, &b, /*uniforms=*/{}, &sig);
    skvm::Program p = b.done();

    REPORTER_ASSERT(r, p.nargs() == (int)(sig.fParameterSlots + sig.fReturnSlots));

    auto out = std::make_unique<float[]>(sig.fReturnSlots);
    auto args = std::make_unique<void*[]>(sig.fParameterSlots + sig.fReturnSlots);
    for (size_t i = 0; i < sig.fParameterSlots; ++i) {
        args[i] = in + i;
    }
    for (size_t i = 0; i < sig.fReturnSlots; ++i) {
        args[sig.fParameterSlots + i] = out.get() + i;
    }

    // TODO: Test with and without JIT?
    p.eval(1, args.get());

    verify_values(r, src, out.get(), expected, sig.fReturnSlots, exactCompare);
}

void test(skiatest::Reporter* r, const char* src, float* in, const float* expected,
          bool exactCompare = true) {
    test_skvm(r, src, in, expected, exactCompare);

    ByteCodeBuilder byteCode(r, src);
    if (!byteCode) { return; }

    const SkSL::ByteCodeFunction* main = byteCode->getFunction("main");
    int returnCount = main->getReturnCount();
    std::unique_ptr<float[]> out = std::unique_ptr<float[]>(new float[returnCount]);
    SkAssertResult(byteCode->run(main, in, main->getParameterCount(), out.get(), returnCount,
                                 nullptr, 0));

    verify_values(r, src, out.get(), expected, returnCount, exactCompare);
}

void vec_test(skiatest::Reporter* r, const char* src) {
    ByteCodeBuilder byteCode(r, src);
    if (!byteCode) { return; }

    const SkSL::ByteCodeFunction* main = byteCode->getFunction("main");

    // Test on four different vectors (with varying orderings to get divergent control flow)
    const float input[16] = { 1, 2, 3, 4,
                              4, 3, 2, 1,
                              7, 5, 8, 6,
                              6, 8, 5, 7 };

    float out_s[16], out_v[16];
    memcpy(out_s, input, sizeof(out_s));
    memcpy(out_v, input, sizeof(out_v));

    // First run in scalar mode to determine the expected output
    for (int i = 0; i < 4; ++i) {
        SkAssertResult(byteCode->run(main, out_s + i * 4, 4, nullptr, 0, nullptr, 0));
    }

    // Need to transpose input vectors for striped execution
    auto transpose = [](float* v) {
        for (int r = 0; r < 4; ++r)
        for (int c = 0; c < r; ++c)
            std::swap(v[r*4 + c], v[c*4 + r]);
    };

    // Need to transpose input vectors for striped execution
    transpose(out_v);
    float* args[] = { out_v, out_v + 4, out_v + 8, out_v + 12 };

    // Now run in parallel and compare results
    SkAssertResult(byteCode->runStriped(main, 4, args, 4, nullptr, 0, nullptr, 0));

    // Transpose striped outputs back
    transpose(out_v);

    if (0 != memcmp(out_s, out_v, sizeof(out_s))) {
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

void test_skvm(skiatest::Reporter* r, const char* src,
               float inR, float inG, float inB, float inA,
               float exR, float exG, float exB, float exA) {
    ProgramBuilder program(r, src);
    if (!program) { return; }

    const SkSL::FunctionDefinition* main = SkSL::Program_GetFunction(*program, "main");
    REPORTER_ASSERT(r, main);

    skvm::Builder b;
    SkSL::ProgramToSkVM(*program, *main, &b, /*uniforms=*/{});
    skvm::Program p = b.done();

    // TODO: Test with and without JIT?
    p.eval(1, &inR, &inG, &inB, &inA);

    float actual[4]   = { inR, inG, inB, inA };
    float expected[4] = { exR, exG, exB, exA };

    verify_values(r, src, actual, expected, 4, /*exactCompare=*/true);

    // TODO: vec_test with skvm
}

void test(skiatest::Reporter* r, const char* src,
          float inR, float inG, float inB, float inA,
          float exR, float exG, float exB, float exA) {
    test_skvm(r, src, inR, inG, inB, inA, exR, exG, exB, exA);

    ByteCodeBuilder byteCode(r, src);
    if (!byteCode) { return; }

    float inoutColor[4] = { inR, inG, inB, inA };
    float expected[4]   = { exR, exG, exB, exA };

    const SkSL::ByteCodeFunction* main = byteCode->getFunction("main");
    SkAssertResult(byteCode->run(main, inoutColor, 4, nullptr, 0, nullptr, 0));

    verify_values(r, src, inoutColor, expected, 4, /*exactCompare=*/true);

    // Do additional testing of 4x1 vs 1x4 to stress divergent control flow, etc.
    vec_test(r, src);
}

DEF_TEST(SkSLInterpreterAdd, r) {
    test(r, "void main(inout half4 color) { color.r = color.r + color.g; }", 0.25, 0.75, 0, 0, 1,
         0.75, 0, 0);
    test(r, "void main(inout half4 color) { color += half4(1, 2, 3, 4); }", 4, 3, 2, 1, 5, 5, 5, 5);
    test(r, "void main(inout half4 color) { half4 c = color; color += c; }", 0.25, 0.5, 0.75, 1,
         0.5, 1, 1.5, 2);
    test(r, "void main(inout half4 color) { color.r = half(int(color.r) + int(color.g)); }", 1, 3, 0, 0,
         4, 3, 0, 0);
}

DEF_TEST(SkSLInterpreterSubtract, r) {
    test(r, "void main(inout half4 color) { color.r = color.r - color.g; }", 1, 0.75, 0, 0, 0.25,
         0.75, 0, 0);
    test(r, "void main(inout half4 color) { color -= half4(1, 2, 3, 4); }", 5, 5, 5, 5, 4, 3, 2, 1);
    test(r, "void main(inout half4 color) { half4 c = color; color -= c; }", 4, 3, 2, 1,
         0, 0, 0, 0);
    test(r, "void main(inout half4 color) { color.x = -color.x; }", 4, 3, 2, 1, -4, 3, 2, 1);
    test(r, "void main(inout half4 color) { color = -color; }", 4, 3, 2, 1, -4, -3, -2, -1);
    test(r, "void main(inout half4 color) { color.r = half(int(color.r) - int(color.g)); }", 3, 1, 0, 0,
         2, 1, 0, 0);
}

DEF_TEST(SkSLInterpreterMultiply, r) {
    test(r, "void main(inout half4 color) { color.r = color.r * color.g; }", 2, 3, 0, 0, 6, 3, 0,
         0);
    test(r, "void main(inout half4 color) { color *= half4(1, 2, 3, 4); }", 2, 3, 4, 5, 2, 6, 12,
         20);
    test(r, "void main(inout half4 color) { half4 c = color; color *= c; }", 4, 3, 2, 1,
         16, 9, 4, 1);
    test(r, "void main(inout half4 color) { color.r = half(int(color.r) * int(color.g)); }", 3, -2, 0, 0,
         -6, -2, 0, 0);
}

DEF_TEST(SkSLInterpreterDivide, r) {
    test(r, "void main(inout half4 color) { color.r = color.r / color.g; }", 1, 2, 0, 0, 0.5, 2, 0,
         0);
    test(r, "void main(inout half4 color) { color /= half4(1, 2, 3, 4); }", 12, 12, 12, 12, 12, 6,
         4, 3);
    test(r, "void main(inout half4 color) { half4 c = color; color /= c; }", 4, 3, 2, 1,
         1, 1, 1, 1);
    test(r, "void main(inout half4 color) { color.r = half(int(color.r) / int(color.g)); }", 8, -2, 0, 0,
         -4, -2, 0, 0);
}

DEF_TEST(SkSLInterpreterAnd, r) {
    test(r, "void main(inout half4 color) { if (color.r > color.g && color.g > color.b) "
            "color = half4(color.a); }", 2, 1, 0, 3, 3, 3, 3, 3);
    test(r, "void main(inout half4 color) { if (color.r > color.g && color.g > color.b) "
            "color = half4(color.a); }", 1, 1, 0, 3, 1, 1, 0, 3);
    test(r, "void main(inout half4 color) { if (color.r > color.g && color.g > color.b) "
            "color = half4(color.a); }", 2, 1, 1, 3, 2, 1, 1, 3);
    test(r, "half global; bool update() { global = 123; return true; }"
            "void main(inout half4 color) { global = 0;  if (color.r > color.g && update()) "
            "color = half4(color.a); color.a = global; }", 2, 1, 1, 3, 3, 3, 3, 123);
    test(r, "half global; bool update() { global = 123; return true; }"
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
    test(r, "half global; bool update() { global = 123; return true; }"
            "void main(inout half4 color) { global = 0;  if (color.r > color.g || update()) "
            "color = half4(color.a); color.a = global; }", 1, 1, 1, 3, 3, 3, 3, 123);
    test(r, "half global; bool update() { global = 123; return true; }"
            "void main(inout half4 color) { global = 0;  if (color.r > color.g || update()) "
            "color = half4(color.a); color.a = global; }", 2, 1, 1, 3, 3, 3, 3, 0);
}

DEF_TEST(SkSLInterpreterMatrix, r) {
    float in[16];
    float expected[16];

    // Constructing matrix from scalar produces a diagonal matrix
    in[0] = 2.0f;
    expected[0] = 4.0f;
    test(r, "float main(float x) { float4x4 m = float4x4(x); return m[1][1] + m[1][2] + m[2][2]; }",
         in, expected);

    // Constructing from a different-sized matrix fills the remaining space with the identity matrix
    expected[0] = 3.0f;
    test(r, "float main(float x) {"
         "float2x2 m = float2x2(x);"
         "float4x4 m2 = float4x4(m);"
         "return m2[0][0] + m2[3][3]; }",
         in, expected);

    // Constructing a matrix from vectors or scalars fills in values in column-major order
    in[0] = 1.0f;
    in[1] = 2.0f;
    in[2] = 4.0f;
    in[3] = 8.0f;
    expected[0] = 6.0f;
    test(r, "float main(float4 v) { float2x2 m = float2x2(v); return m[0][1] + m[1][0]; }",
         in, expected);

    expected[0] = 10.0f;
    test(r, "float main(float4 v) {"
         "float2x2 m = float2x2(v.x, v.y, v.w, v.z);"
         "return m[0][1] + m[1][0]; }",
         in, expected);

    // Initialize 16 values to be used as inputs to matrix tests
    for (int i = 0; i < 16; ++i) { in[i] = (float)i; }

    // M+M, M-S, S-M
    for (int i = 0; i < 16; ++i) { expected[i] = (float)(2 * i); }
    test(r, "float4x4 main(float4x4 m) { return m + m; }", in, expected);
    for (int i = 0; i < 16; ++i) { expected[i] = (float)(i + 3); }
    test(r, "float4x4 main(float4x4 m) { return m + 3.0; }", in, expected);
    test(r, "float4x4 main(float4x4 m) { return 3.0 + m; }", in, expected);

    // M-M, M-S, S-M
    for (int i = 0; i < 4; ++i) { expected[i] = 4.0f; }
    test(r, "float2x2 main(float2x2 m1, float2x2 m2) { return m2 - m1; }", in, expected);
    for (int i = 0; i < 16; ++i) { expected[i] = (float)(i - 3); }
    test(r, "float4x4 main(float4x4 m) { return m - 3.0; }", in, expected);
    for (int i = 0; i < 16; ++i) { expected[i] = (float)(3 - i); }
    test(r, "float4x4 main(float4x4 m) { return 3.0 - m; }", in, expected);

    // M*S, S*M, M/S, S/M
    for (int i = 0; i < 16; ++i) { expected[i] = (float)(i * 3); }
    test(r, "float4x4 main(float4x4 m) { return m * 3.0; }", in, expected);
    test(r, "float4x4 main(float4x4 m) { return 3.0 * m; }", in, expected);
    for (int i = 0; i < 16; ++i) { expected[i] = (float)(i) / 2.0f; }
    test(r, "float4x4 main(float4x4 m) { return m / 2.0; }", in, expected);
    for (int i = 0; i < 16; ++i) { expected[i] = 1.0f / (float)(i + 1); }
    test(r, "float4x4 main(float4x4 m) { return 1.0 / (m + 1); }", in, expected);

    // Matrix negation
    for (int i = 0; i < 16; ++i) { expected[i] = (float)(-i); }
    test(r, "float4x4 main(float4x4 m) { return -m; }", in, expected);

    // M*V, V*M
    for (int i = 0; i < 3; ++i) {
        expected[i] = 9.0f*i + 10.0f*(i+3) + 11.0f*(i+6);
    }
    test(r, "float3 main(float3x3 m, float3 v) { return m * v; }", in, expected);
    for (int i = 0; i < 3; ++i) {
        expected[i] = 9.0f*(3*i) + 10.0f*(3*i+1) + 11.0f*(3*i+2);
    }
    test(r, "float3 main(float3x3 m, float3 v) { return v * m; }", in, expected);

    // M*M
    {
        SkM44 m = SkM44::ColMajor(in);
        SkM44 m2;
        float in2[16];
        for (int i = 0; i < 16; ++i) {
            in2[i] = (i + 4) % 16;
        }
        m2 = SkM44::ColMajor(in2);
        m.setConcat(m, m2);
        // Rearrange the columns on the RHS so we detect left-hand/right-hand errors
        test(r, "float4x4 main(float4x4 m) { return m * float4x4(m[1], m[2], m[3], m[0]); }",
             in, (float*)&m);
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
        int32_t  s;
    };

    Val input[2];
    Val expected[2];

    input[0].s = 3;
    input[1].s = -5;
    expected[0].f = 3.0f;
    expected[1].f = -5.0f;
    test(r, "float  main(int  x) { return float (x); }", (float*)input, (float*)expected);
    test(r, "float2 main(int2 x) { return float2(x); }", (float*)input, (float*)expected);

    input[0].f = 3.0f;
    input[1].f = -5.0f;
    expected[0].s = 3;
    expected[1].s = -5;
    test(r, "int  main(float  x) { return int (x); }", (float*)input, (float*)expected);
    test(r, "int2 main(float2 x) { return int2(x); }", (float*)input, (float*)expected);

    input[0].s = 3;
    expected[0].f = 3.0f;
    expected[1].f = 3.0f;
    test(r, "float2 main(int x) { return float2(x); }", (float*)input, (float*)expected);
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
    test(r, "void main(inout half4 color) { if (!(color.r == color.g)) color.a = 1; }", 2, 2, 0, 0,
         2, 2, 0, 0);
    test(r, "void main(inout half4 color) { if (!(color.r == color.g)) color.a = 1; }", 2, -2, 0, 0,
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

DEF_TEST(SkSLInterpreterFor, r) {
    test(r, "void main(inout half4 color) { for (int i = 1; i <= 10; ++i) color.r += half(i); }",
         0, 0, 0, 0,
         55, 0, 0, 0);
    test(r,
         "void main(inout half4 color) {"
         "    for (int i = 1; i <= 10; ++i)"
         "        for (int j = 1; j <= 10; ++j)"
         "            if (j >= i) { color.r += half(j); }"
         "}",
         0, 0, 0, 0,
         385, 0, 0, 0);
    test(r,
         "void main(inout half4 color) {"
         "    for (int i = 1; i <= 10; ++i)"
         "        for (int j = 1; j < 20 ; ++j) {"
         "            if (i == j) continue;"
         "            if (j > 10) break;"
         "            color.r += half(j);"
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
    test(r, "void main(inout half4 color) { color.bgr = half3(5, 6, 7); }", 1, 2, 3, 4, 7, 6,
         5, 4);
}

DEF_TEST(SkSLInterpreterGlobal, r) {
    test(r, "int x; void main(inout half4 color) { x = 10; color.b = half(x); }", 1, 2, 3, 4, 1, 2,
         10, 4);
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
    test(r, "float main(float x) { return x * x; }", &value1, &expected1);
    float value2[2] = { 5, 25 };
    float expected2[2] = { 25, 625 };
    test(r, "float2 main(float x, float y) { return float2(x * x, y * y); }", value2, expected2);
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
        "int median(int a[15]) { return a[7]; }\n"

        "float tempFloats[8];\n"
        "float sums(float a[8]) {\n"
        "  tempFloats[0] = a[0];\n"
        "  for (int i = 1; i < 8; ++i) { tempFloats[i] = tempFloats[i - 1] + a[i]; }\n"
        "  return tempFloats[7];\n"
        "}\n"

        // Uniforms, array-of-structs
        "uniform Rect gRects[4];\n"
        "Rect get_rect_2() { return gRects[2]; }\n"

        // Kitchen sink (swizzles, inout, SoAoS)
        "struct ManyRects { int numRects; RectAndColor rects[4]; };\n"
        "void fill_rects(inout ManyRects mr) {\n"
        "  for (int i = 0; i < 4; ++i) {\n"
        "    if (i >= mr.numRects) { break; }\n"
        "    mr.rects[i].r = gRects[i];\n"
        "    float b = float(mr.rects[i].r.p1.y);\n"
        "    mr.rects[i].color = float4(b, b, b, b);\n"
        "  }\n"
        "}\n";

    ProgramBuilder program(r, src);

    auto rect_height    = SkSL::Program_GetFunction(*program, "rect_height"),
         make_blue_rect = SkSL::Program_GetFunction(*program, "make_blue_rect"),
         median         = SkSL::Program_GetFunction(*program, "median"),
         sums           = SkSL::Program_GetFunction(*program, "sums"),
         get_rect_2     = SkSL::Program_GetFunction(*program, "get_rect_2"),
         fill_rects     = SkSL::Program_GetFunction(*program, "fill_rects");

    SkIRect gRects[4] = { { 1,2,3,4 }, { 5,6,7,8 }, { 9,10,11,12 }, { 13,14,15,16 } };

    auto build = [&](const SkSL::FunctionDefinition* fn) {
        skvm::Builder b;
        skvm::Ptr uniformPtr = b.uniform();
        skvm::Val uniforms[16];
        for (int i = 0; i < 16; ++i) {
            uniforms[i] = b.uniform32(uniformPtr, i * sizeof(int)).id;
        }
        SkSL::ProgramToSkVM(*program, *fn, &b, uniforms);
        return b.done();
    };

    struct Args {
        Args(void* uniformData) { fArgs.push_back(uniformData); }
        void add(void* base, int n) {
            for (int i = 0; i < n; ++i) {
                fArgs.push_back(SkTAddOffset<void>(base, i * sizeof(float)));
            }
        }
        std::vector<void*> fArgs;
    };

    {
        SkIRect in = SkIRect::MakeXYWH(10, 10, 20, 30);
        int out = 0;
        skvm::Program p = build(rect_height);
        Args args(gRects);
        args.add(&in, 4);
        args.add(&out, 1);
        p.eval(1, args.fArgs.data());
        REPORTER_ASSERT(r, out == 30);
    }

    {
        int in[2] = { 15, 25 };
        RectAndColor out;
        skvm::Program p = build(make_blue_rect);
        Args args(gRects);
        args.add(&in, 2);
        args.add(&out, 8);
        p.eval(1, args.fArgs.data());
        REPORTER_ASSERT(r, out.fRect.width() == 15);
        REPORTER_ASSERT(r, out.fRect.height() == 25);
        SkColor4f blue = { 0.0f, 1.0f, 0.0f, 1.0f };
        REPORTER_ASSERT(r, out.fColor == blue);
    }

    {
        int in[15] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
        int out = 0;
        skvm::Program p = build(median);
        Args args(gRects);
        args.add(&in, 15);
        args.add(&out, 1);
        p.eval(1, args.fArgs.data());
        REPORTER_ASSERT(r, out == 8);
    }

    // TODO: Doesn't work until SkVM generator supports indexing-by-loop variable
    if (false) {
        float in[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
        float out = 0;
        skvm::Program p = build(sums);
        Args args(gRects);
        args.add(&in, 8);
        args.add(&out, 1);
        p.eval(1, args.fArgs.data());
        REPORTER_ASSERT(r, out == static_cast<float>((7 + 1) * (7 + 2) / 2));
    }

    {
        SkIRect out = SkIRect::MakeEmpty();
        skvm::Program p = build(get_rect_2);
        Args args(gRects);
        args.add(&out, 4);
        p.eval(1, args.fArgs.data());
        REPORTER_ASSERT(r, out == gRects[2]);
    }

    // TODO: Doesn't work until SkVM generator supports indexing-by-loop variable
    if (false) {
        ManyRects in;
        memset(&in, 0, sizeof(in));
        in.fNumRects = 2;
        skvm::Program p = build(fill_rects);
        Args args(gRects);
        args.add(&in, 33);
        p.eval(1, args.fArgs.data());
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
    GrShaderCaps caps(GrContextOptions{});
    SkSL::Compiler compiler(&caps);
    SkSL::Program::Settings settings;
    auto program = compiler.convertProgram(SkSL::Program::kGeneric_Kind,
                                           SkSL::String(src), settings);
    REPORTER_ASSERT(r, !program);
}

DEF_TEST(SkSLInterpreterRestrictLoops, r) {
    // while and do-while loops are not allowed
    expect_failure(r, "void main(inout float x) { while (x < 1) { x++; } }");
    expect_failure(r, "void main(inout float x) { do { x++; } while (x < 1); }");
}

DEF_TEST(SkSLInterpreterRestrictFunctionCalls, r) {
    // Ensure that simple recursion is not allowed
    expect_failure(r, "float main() { return main() + 1; }");

    // Ensure that calls to undefined functions are not allowed (to prevent mutual recursion)
    expect_failure(r, "float foo(); float bar() { return foo(); } float foo() { return bar(); }");
}

DEF_TEST(SkSLInterpreterReturnThenCall, r) {
    // Test that early returns disable execution in subsequently called functions
    const char* src = R"(
        float y;
        void inc () { ++y; }
        void maybe_inc() { if (y < 0) return; inc(); }
        void main(inout float x) { y = x; maybe_inc(); x = y; }
    )";

    ProgramBuilder program(r, src);
    const SkSL::FunctionDefinition* main = SkSL::Program_GetFunction(*program, "main");
    REPORTER_ASSERT(r, main);

    skvm::Builder b;
    SkSL::ProgramToSkVM(*program, *main, &b, /*uniforms=*/{});
    skvm::Program p = b.done();

    float xs[] = { -2.0f, 0.0f, 3.0f, -1.0f };
    p.eval(4, xs);

    REPORTER_ASSERT(r, xs[0] == -2.0f);
    REPORTER_ASSERT(r, xs[1] ==  1.0f);
    REPORTER_ASSERT(r, xs[2] ==  4.0f);
    REPORTER_ASSERT(r, xs[3] == -1.0f);
}

DEF_TEST(SkSLInterpreterEarlyReturn, r) {
    // Test early returns with divergent control flow
    const char* src = "float main(float x, float y) { if (x < y) { return x; } return y; }";

    ProgramBuilder program(r, src);

    const SkSL::FunctionDefinition* main = SkSL::Program_GetFunction(*program, "main");
    REPORTER_ASSERT(r, main);

    skvm::Builder b;
    SkSL::ProgramToSkVM(*program, *main, &b, /*uniforms=*/{});
    skvm::Program p = b.done();

    float xs[] = { 1.0f, 3.0f },
          ys[] = { 2.0f, 2.0f };
    float rets[2];
    p.eval(2, xs, ys, rets);

    REPORTER_ASSERT(r, rets[0] == 1.0f);
    REPORTER_ASSERT(r, rets[1] == 2.0f);
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

    ProgramBuilder program(r, src);

    auto sub  = SkSL::Program_GetFunction(*program, "sub");
    auto sqr  = SkSL::Program_GetFunction(*program, "sqr");
    auto main = SkSL::Program_GetFunction(*program, "main");
    auto tan  = SkSL::Program_GetFunction(*program, "tan");
    auto dot3 = SkSL::Program_GetFunction(*program, "dot3_test");
    auto dot2 = SkSL::Program_GetFunction(*program, "dot2_test");

    REPORTER_ASSERT(r, sub);
    REPORTER_ASSERT(r, sqr);
    REPORTER_ASSERT(r, main);
    REPORTER_ASSERT(r, !tan);  // Getting a non-existent function should return nullptr
    REPORTER_ASSERT(r, dot3);
    REPORTER_ASSERT(r, dot2);

    auto test_fn = [&](const SkSL::FunctionDefinition* fn, float in, float expected) {
        skvm::Builder b;
        SkSL::ProgramToSkVM(*program, *fn, &b, /*uniforms=*/{});
        skvm::Program p = b.done();

        float out = 0.0f;
        p.eval(1, &in, &out);
        REPORTER_ASSERT(r, out == expected);
    };

    test_fn(main, 3.0f, 6.0f);
    test_fn(dot3, 3.0f, 9.0f);
    test_fn(dot2, 3.0f, -1.0f);
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

DEF_TEST(SkSLInterpreterSwizzleSingleLvalue, r) {
    // Add in your SkSL here.
    test(r,
         "void main(inout half4 color) { color.xywz = half4(1,2,3,4); }",
         0, 0, 0, 0, 1, 2, 4, 3);
}

DEF_TEST(SkSLInterpreterSwizzleDoubleLvalue, r) {
    // Add in your SkSL here.
    test(r,
         "void main(inout half4 color) { color.xywz.yxzw = half4(1,2,3,4); }",
         0, 0, 0, 0, 2, 1, 4, 3);
}

DEF_TEST(SkSLInterpreterMathFunctions, r) {
    float value[4], expected[4];

    value[0] = 0.0f; expected[0] = 0.0f;
    test(r, "float main(float x) { return sin(x); }", value, expected);
    test(r, "float main(float x) { return tan(x); }", value, expected);

    value[0] = 0.0f; expected[0] = 1.0f;
    test(r, "float main(float x) { return cos(x); }", value, expected);

    value[0] = 25.0f; expected[0] = 5.0f;
    test(r, "float main(float x) { return sqrt(x); }", value, expected);

    value[0] = 90.0f; expected[0] = sk_float_degrees_to_radians(value[0]);
    test(r, "float main(float x) { return radians(x); }", value, expected);

    value[0] = 1.0f; value[1] = -1.0f;
    expected[0] = 1.0f / SK_FloatSqrt2; expected[1] = -1.0f / SK_FloatSqrt2;
    test(r, "float2 main(float2 x) { return normalize(x); }", value, expected);
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
    test(r, "float main(float x) { return mix(-10, 10, x); }", &value, &expected);
    value = 0.75f; expected = 5.0f;
    test(r, "float main(float x) { return mix(-10, 10, x); }", &value, &expected);
    value = 2.0f; expected = 30.0f;
    test(r, "float main(float x) { return mix(-10, 10, x); }", &value, &expected);

    float valueVectors[]   = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f },
          expectedVector[] = { 3.0f, 4.0f, 5.0f, 6.0f };
    test(r, "float4 main(float4 x, float4 y) { return mix(x, y, 0.5); }", valueVectors,
         expectedVector);
}

DEF_TEST(SkSLInterpreterCross, r) {
    float args[] = { 1.0f, 4.0f, -6.0f, -2.0f, 7.0f, -3.0f };
    SkV3 cross = SkV3::Cross({args[0], args[1], args[2]},
                             {args[3], args[4], args[5]});
    float expected[] = { cross.x, cross.y, cross.z };
    test(r, "float3 main(float3 x, float3 y) { return cross(x, y); }", args, expected);
}

DEF_TEST(SkSLInterpreterInverse, r) {
    {
        SkMatrix m;
        m.setRotate(30).postScale(1, 2);
        float args[4] = { m[0], m[3], m[1], m[4] };
        SkAssertResult(m.invert(&m));
        float expt[4] = { m[0], m[3], m[1], m[4] };
        test(r, "float2x2 main(float2x2 m) { return inverse(m); }", args, expt, false);
    }
    {
        SkMatrix m;
        m.setRotate(30).postScale(1, 2).postTranslate(1, 2);
        float args[9] = { m[0], m[3], m[6], m[1], m[4], m[7], m[2], m[5], m[8] };
        SkAssertResult(m.invert(&m));
        float expt[9] = { m[0], m[3], m[6], m[1], m[4], m[7], m[2], m[5], m[8] };
        test(r, "float3x3 main(float3x3 m) { return inverse(m); }", args, expt, false);
    }
    {
        float args[16], expt[16];
        // just some crazy thing that is invertible
        SkM44 m = {1, 2, 3, 4, 1, 2, 0, 3, 1, 0, 1, 4, 1, 3, 2, 0};
        m.getColMajor(args);
        SkAssertResult(m.invert(&m));
        m.getColMajor(expt);
        test(r, "float4x4 main(float4x4 m) { return inverse(m); }", args, expt, false);
    }
}

DEF_TEST(SkSLInterpreterDot, r) {
    float args[] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f };
    float expected = args[0] * args[2] +
                     args[1] * args[3];
    test(r, "float main(float2 x, float2 y) { return dot(x, y); }", args, &expected);

    expected = args[0] * args[3] +
               args[1] * args[4] +
               args[2] * args[5];
    test(r, "float main(float3 x, float3 y) { return dot(x, y); }", args, &expected);

    expected = args[0] * args[4] +
               args[1] * args[5] +
               args[2] * args[6] +
               args[3] * args[7];
    test(r, "float main(float4 x, float4 y) { return dot(x, y); }", args, &expected);
}

class ExternalSqrt : public SkSL::ExternalFunction {
public:
    ExternalSqrt(const char* name, SkSL::Compiler& compiler)
        : INHERITED(name, *compiler.context().fTypes.fFloat)
        , fCompiler(compiler) {}

    int callParameterCount() const override { return 1; }

    void getCallParameterTypes(const SkSL::Type** outTypes) const override {
        outTypes[0] = fCompiler.context().fTypes.fFloat.get();
    }

    void call(int /*unusedIndex*/, float* arguments, float* outReturn) const override {
        outReturn[0] = sqrt(arguments[0]);
    }

private:
    SkSL::Compiler& fCompiler;
    using INHERITED = SkSL::ExternalFunction;
};

DEF_TEST(SkSLInterpreterExternalFunction, r) {
    GrShaderCaps caps(GrContextOptions{});
    SkSL::Compiler compiler(&caps);
    SkSL::Program::Settings settings;
    const char* src = "float main() {"
                      "    return external(25);"
                      "}";
    std::vector<std::unique_ptr<SkSL::ExternalFunction>> externalFunctions;
    externalFunctions.push_back(std::make_unique<ExternalSqrt>("external", compiler));
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(
            SkSL::Program::kGeneric_Kind, SkSL::String(src), settings, &externalFunctions);
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
        SkAssertResult(byteCode->run(main, nullptr, 0, &out, 1, nullptr, 0));
        REPORTER_ASSERT(r, out == 5.0);
    } else {
        printf("%s\n%s", src, compiler.errorText().c_str());
    }
}

class ExternalSqrt4 : public SkSL::ExternalFunction {
public:
    ExternalSqrt4(const char* name, SkSL::Compiler& compiler)
        : INHERITED(name, *compiler.context().fTypes.fFloat4)
        , fCompiler(compiler) {}

    int callParameterCount() const override { return 1; }

    void getCallParameterTypes(const SkSL::Type** outTypes) const override {
        outTypes[0] = fCompiler.context().fTypes.fFloat4.get();
    }

    void call(int /*unusedIndex*/, float* arguments, float* outReturn) const override {
        outReturn[0] = sqrt(arguments[0]);
        outReturn[1] = sqrt(arguments[1]);
        outReturn[2] = sqrt(arguments[2]);
        outReturn[3] = sqrt(arguments[3]);
    }

private:
    SkSL::Compiler& fCompiler;
    using INHERITED = SkSL::ExternalFunction;
};


DEF_TEST(SkSLInterpreterExternalFunctionVector, r) {
    GrShaderCaps caps(GrContextOptions{});
    SkSL::Compiler compiler(&caps);
    SkSL::Program::Settings settings;
    const char* src =
            "float4 main() {"
            "    return external(float4(1, 4, 9, 16));"
            "}";
    std::vector<std::unique_ptr<SkSL::ExternalFunction>> externalFunctions;
    externalFunctions.push_back(std::make_unique<ExternalSqrt4>("external", compiler));
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(
            SkSL::Program::kGeneric_Kind, SkSL::String(src), settings, &externalFunctions);
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
        SkAssertResult(byteCode->run(main, nullptr, 0, out, 4, nullptr, 0));
        REPORTER_ASSERT(r, out[0] == 1.0);
        REPORTER_ASSERT(r, out[1] == 2.0);
        REPORTER_ASSERT(r, out[2] == 3.0);
        REPORTER_ASSERT(r, out[3] == 4.0);
    } else {
        printf("%s\n%s", src, compiler.errorText().c_str());
    }
}

#endif // SK_ENABLE_SKSL_INTERPRETER
