/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkM44.h"
#include "src/sksl/SkSLByteCode.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLExternalValue.h"
#include "src/sksl/SkSLInterpreter.h"
#include "src/utils/SkJSON.h"

#include "tests/Test.h"

void test(skiatest::Reporter* r, const char* src, float* in, float* expected,
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
        SkSL::Interpreter<1> interpreter(std::move(byteCode));
        SkSL::ByteCode::Vector<1>* result;
        bool success = interpreter.run(main, (SkSL::ByteCode::Vector<1>*) in, &result);
        REPORTER_ASSERT(r, success);
        for (int i = 0; i < main->getReturnSlotCount(); ++i) {
            printf("%d: expected %f, received %f\n", i, result[i].fFloat[0], expected[i]);
        }
        REPORTER_ASSERT(r, !memcmp(&result->fFloat, expected,
                                   sizeof(float) * main->getReturnSlotCount()));
    } else {
        printf("%s\n%s", src, compiler.errorText().c_str());
    }
}

void vec_test(skiatest::Reporter* r, const char* src) {
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

    const SkSL::ByteCodeFunction* main1 = byteCode->getFunction("main");
    SkSL::Interpreter<1> interpreter1(std::move(byteCode));

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
        SkAssertResult(interpreter1.run(main1, (SkSL::ByteCode::Vector<1>*) (out_s + i * 4),
                       nullptr));
    }

    byteCode = compiler.toByteCode(*program);
    SkASSERT(compiler.errorCount() == 0);

    const SkSL::ByteCodeFunction* main4 = byteCode->getFunction("main");
    SkSL::Interpreter<4> interpreter4(std::move(byteCode));

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
    SkAssertResult(interpreter4.runStriped(main4, 4, (float**) args));

    // Transpose striped outputs back
    transpose(out_v);

    if (memcmp(out_s, out_v, sizeof(out_s)) != 0) {
        printf("for program: %s\n", src);
        for (int i = 0; i < 4; ++i) {
            printf("(%g %g %g %g) -> (%g %g %g %g), expected (%g %g %g %g)\n",
                    input[4*i + 0], input[4*i + 1], input[4*i + 2], input[4*i + 3],
                    out_v[4*i + 0], out_v[4*i + 1], out_v[4*i + 2], out_v[4*i + 3],
                    out_s[4*i + 0], out_s[4*i + 1], out_s[4*i + 2], out_s[4*i + 3]);
        }
        main4->disassemble();
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
        SkSL::ByteCode::Vector<1> inoutColor[4];
        inoutColor[0].fFloat[0] = inR;
        inoutColor[1].fFloat[0] = inG;
        inoutColor[2].fFloat[0] = inB;
        inoutColor[3].fFloat[0] = inA;
        SkSL::Interpreter<1> interpreter(std::move(byteCode));
        bool success = interpreter.run(main, inoutColor, nullptr);
        REPORTER_ASSERT(r, success);
        if (inoutColor[0].fFloat[0] != expectedR || inoutColor[1].fFloat[0] != expectedG ||
            inoutColor[2].fFloat[0] != expectedB || inoutColor[3].fFloat[0] != expectedA) {
            printf("for program: %s\n", src);
            printf("    expected (%f, %f, %f, %f), but received (%f, %f, %f, %f)\n", expectedR,
                   expectedG, expectedB, expectedA, inoutColor[0].fFloat[0],
                   inoutColor[1].fFloat[0], inoutColor[2].fFloat[0], inoutColor[3].fFloat[0]);
            main->disassemble();
        }
        REPORTER_ASSERT(r, inoutColor[0].fFloat[0] == expectedR);
        REPORTER_ASSERT(r, inoutColor[1].fFloat[0] == expectedG);
        REPORTER_ASSERT(r, inoutColor[2].fFloat[0] == expectedB);
        REPORTER_ASSERT(r, inoutColor[3].fFloat[0] == expectedA);
    } else {
        printf("%s\n%s", src, compiler.errorText().c_str());
    }

    // Do additional testing of 4x1 vs 1x4 to stress divergent control flow, etc.
    vec_test(r, src);
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
