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

DEF_TEST(SkSLInterfaceBlockStorageModifiers, r) {
    test_failure(r,
                 "uniform foo { out int x; };",
                 "error: 1: 'out' is not permitted here\n1 error\n");
}

DEF_TEST(SkSLUseWithoutInitialize, r) {
    test_failure(r,
                 "void main() { int x; if (5 == 2) x = 3; x++; }",
                 "error: 1: 'x' has not been assigned\n1 error\n");
    test_failure(r,
                 "void main() { int x[2][2]; int i; x[i][1] = 4; }",
                 "error: 1: 'i' has not been assigned\n1 error\n");
    test_failure(r,
                 "int main() { int r; return r; }",
                 "error: 1: 'r' has not been assigned\n1 error\n");
    test_failure(r,
                 "void main() { int x; int y = x; }",
                 "error: 1: 'x' has not been assigned\n1 error\n");
    test_failure(r,
                 "void main() { bool x; if (true && (false || x)) return; }",
                 "error: 1: 'x' has not been assigned\n1 error\n");
    test_failure(r,
                 "void main() { int x; switch (3) { case 0: x = 0; case 1: x = 1; }"
                               "sk_FragColor = half4(x); }",
                 "error: 1: 'x' has not been assigned\n1 error\n");
}

DEF_TEST(SkSLUnreachable, r) {
    test_failure(r,
                 "void main() { return; return; }",
                 "error: 1: unreachable\n1 error\n");
    test_failure(r,
                 "void main() { for (;;) { continue; int x = 1; } }",
                 "error: 1: unreachable\n1 error\n");
/*    test_failure(r,
                 "void main() { for (;;) { } return; }",
                 "error: 1: unreachable\n1 error\n");*/
    test_failure(r,
                 "void main() { if (true) return; else discard; return; }",
                 "error: 1: unreachable\n1 error\n");
    test_failure(r,
                 "void main() { return; main(); }",
                 "error: 1: unreachable\n1 error\n");
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

DEF_TEST(SkSLContinueOutsideLoop, r) {
    test_failure(r,
                 "void foo() { for(;;); continue; }",
                 "error: 1: continue statement must be inside a loop\n1 error\n");
    test_failure(r,
                 "void foo() { switch (1) { default: continue; } }",
                 "error: 1: continue statement must be inside a loop\n1 error\n");
}

DEF_TEST(SkSLStaticIfError, r) {
    // ensure eliminated branch of static if / ternary is still checked for errors
    test_failure(r,
                 "void foo() { if (true); else x = 5; }",
                 "error: 1: unknown identifier 'x'\n1 error\n");
    test_failure(r,
                 "void foo() { if (false) x = 5; }",
                 "error: 1: unknown identifier 'x'\n1 error\n");
    test_failure(r,
                 "void foo() { true ? 5 : x; }",
                 "error: 1: unknown identifier 'x'\n1 error\n");
    test_failure(r,
                 "void foo() { false ? x : 5; }",
                 "error: 1: unknown identifier 'x'\n1 error\n");
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

DEF_TEST(SkSLUnsupportedGLSLIdentifiers, r) {
    test_failure(r,
                 "void main() { float x = gl_FragCoord.x; }",
                 "error: 1: unknown identifier 'gl_FragCoord'\n1 error\n");
    test_failure(r,
                 "void main() { float r = gl_FragColor.r; }",
                 "error: 1: unknown identifier 'gl_FragColor'\n1 error\n");
}

DEF_TEST(SkSLWrongSwitchTypes, r) {
    test_failure(r,
                 "void main() { switch (float2(1)) { case 1: break; } }",
                 "error: 1: expected 'int', but found 'float2'\n1 error\n");
    test_failure(r,
                 "void main() { switch (1) { case float2(1): break; } }",
                 "error: 1: expected 'int', but found 'float2'\n1 error\n");
    test_failure(r,
                 "void main() { switch (1) { case 0.5: break; } }",
                 "error: 1: expected 'int', but found 'float'\n1 error\n");
    test_failure(r,
                 "void main() { switch (1) { case 1.0: break; } }",
                 "error: 1: expected 'int', but found 'float'\n1 error\n");
    test_failure(r,
                 "uniform float x = 1; void main() { switch (1) { case x: break; } }",
                 "error: 1: expected 'int', but found 'float'\n1 error\n");
    test_failure(r,
                 "const float x = 1; void main() { switch (1) { case x: break; } }",
                 "error: 1: expected 'int', but found 'float'\n1 error\n");
    test_failure(r,
                 "const float x = 1; void main() { switch (x) { case 1: break; } }",
                 "error: 1: expected 'int', but found 'float'\n1 error\n");
    test_success(r,
                 "const int x = 1; void main() { switch (x) { case 1: break; } }");
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

DEF_TEST(SkSLDuplicateCase, r) {
    test_failure(r,
                 "void main() { switch (1) { case 0: case 1: case 0: break; } }",
                 "error: 1: duplicate case value\n1 error\n");
}

DEF_TEST(SkSLFieldAfterRuntimeArray, r) {
    test_failure(r,
                 "buffer broken { float x[]; float y; };",
                 "error: 1: only the last entry in an interface block may be a runtime-sized "
                 "array\n1 error\n");
}

DEF_TEST(SkSLStaticIf, r) {
    test_success(r,
                 "void main() { float x = 5; float y = 10;"
                 "@if (x < y) { sk_FragColor = half4(1); } }");
    test_failure(r,
                 "void main() { float x = sqrt(25); float y = 10;"
                 "@if (x < y) { sk_FragColor = half4(1); } }",
                 "error: 1: static if has non-static test\n1 error\n");
}

DEF_TEST(SkSLInterfaceBlockScope, r) {
    test_failure(r,
                 "uniform testBlock {"
                 "float x;"
                 "} test[x];",
                 "error: 1: unknown identifier 'x'\n1 error\n");
}

DEF_TEST(SkSLDuplicateOutput, r) {
    test_failure(r,
                 "layout (location=0, index=0) out half4 duplicateOutput;",
                 "error: 1: out location=0, index=0 is reserved for sk_FragColor\n1 error\n");
}
