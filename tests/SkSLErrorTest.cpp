/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLCompiler.h"

#include "tests/Test.h"

static void test_failure(skiatest::Reporter* r, const char* src, const char* error) {
    SkSL::Compiler compiler;
    SkSL::Program::Settings settings;
    sk_sp<GrShaderCaps> caps = SkSL::ShaderCapsFactory::Default();
    settings.fCaps = caps.get();
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(SkSL::Program::kFragment_Kind,
                                                                     SkSL::String(src), settings);
    SkSL::String skError(error);
    if (compiler.errorText() != skError) {
        SkDebugf("SKSL ERROR:\n    source: %s\n    expected: %s    received: %s", src, error,
                 compiler.errorText().c_str());
    }
    REPORTER_ASSERT(r, compiler.errorText() == skError);
}

static void test_success(skiatest::Reporter* r, const char* src) {
    SkSL::Compiler compiler;
    SkSL::Program::Settings settings;
    sk_sp<GrShaderCaps> caps = SkSL::ShaderCapsFactory::Default();
    settings.fCaps = caps.get();
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(SkSL::Program::kFragment_Kind,
                                                                     SkSL::String(src), settings);
    REPORTER_ASSERT(r, program);
    if (!program) {
        SkDebugf("ERROR:\n%s\n", compiler.errorText().c_str());
    }
}

DEF_TEST(SkSLNoReturn, r) {
    test_failure(r,
                 "int foo() { if (2 > 5) return 3; }",
                 "error: 1: function 'foo' can exit without returning a value\n1 error\n");
}

DEF_TEST(SkSLBreakOutsideLoop, r) {
    test_failure(r,
                 "void foo() { while(true) {} if (true) break; }",
                 "error: 1: break statement must be inside a loop or switch\n1 error\n");
}

DEF_TEST(SkSLBadCap, r) {
    test_failure(r,
                 "bool b = sk_Caps.bugFreeDriver;",
                 "error: 1: unknown capability flag 'bugFreeDriver'\n1 error\n");
}

DEF_TEST(SkSLDivByZero, r) {
    test_failure(r,
                 "int x = 1 / 0;",
                 "error: 1: division by zero\n1 error\n");
    test_failure(r,
                 "float x = 1 / 0;",
                 "error: 1: division by zero\n1 error\n");
    test_failure(r,
                 "float x = 1.0 / 0.0;",
                 "error: 1: division by zero\n1 error\n");
    test_failure(r,
                 "float x = -67.0 / (3.0 - 3);",
                 "error: 1: division by zero\n1 error\n");
}

DEF_TEST(SkSLNonConstantCase, r) {
    test_failure(r,
                 "uniform int x = 1; void main() { switch (1) { case x: break; } }",
                 "error: 1: case value must be a constant integer\n1 error\n");
    test_failure(r,
                 "void main() { int x = 1; switch (1) { case x: break; } }",
                 "error: 1: case value must be a constant integer\n1 error\n");
    test_success(r,
                 "uniform int x = 1; void main() { switch (x) { case 1: break; } }");
    test_success(r,
                 "void main() { const int x = 1; switch (1) { case x: break; } }");
}

DEF_TEST(SkSLFieldAfterRuntimeArray, r) {
    test_failure(r,
                 "buffer broken { float x[]; float y; };",
                 "error: 1: only the last entry in an interface block may be a runtime-sized "
                 "array\n1 error\n");
}
