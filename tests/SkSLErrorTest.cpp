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
    if (!compiler.errorCount()) {
        compiler.optimize(*program);
    }
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
}

DEF_TEST(SkSLUndefinedSymbol, r) {
    test_failure(r,
                 "void main() { x = float2(1); }",
                 "error: 1: unknown identifier 'x'\n1 error\n");
}

DEF_TEST(SkSLUndefinedFunction, r) {
    test_failure(r,
                 "void main() { int x = foo(1); }",
                 "error: 1: unknown identifier 'foo'\n1 error\n");
}

DEF_TEST(SkSLGenericArgumentMismatch, r) {
    test_failure(r,
                 "void main() { float x = sin(1, 2); }",
                 "error: 1: no match for sin(int, int)\n1 error\n");
    test_failure(r,
                 "void main() { float x = sin(true); }",
                 "error: 1: no match for sin(bool)\n1 error\n");
    test_success(r,
                 "void main() { float x = sin(1); }");
}

DEF_TEST(SkSLArgumentCountMismatch, r) {
    test_failure(r,
                 "float foo(float x) { return x * x; }"
                 "void main() { float x = foo(1, 2); }",
                 "error: 1: call to 'foo' expected 1 argument, but found 2\n1 error\n");
}

DEF_TEST(SkSLArgumentMismatch, r) {
    test_failure(r,
                 "float foo(float x) { return x * x; }"
                 "void main() { float x = foo(true); }",
                 "error: 1: expected 'float', but found 'bool'\n1 error\n");
}

DEF_TEST(SkSLIfTypeMismatch, r) {
    test_failure(r,
                 "void main() { if (3) { } }",
                 "error: 1: expected 'bool', but found 'int'\n1 error\n");
}

DEF_TEST(SkSLDoTypeMismatch, r) {
    test_failure(r,
                 "void main() { do { } while (float2(1)); }",
                 "error: 1: expected 'bool', but found 'float2'\n1 error\n");
}

DEF_TEST(SkSLWhileTypeMismatch, r) {
    test_failure(r,
                 "void main() { while (float3(1)) { } }",
                 "error: 1: expected 'bool', but found 'float3'\n1 error\n");
}

DEF_TEST(SkSLForTypeMismatch, r) {
    test_failure(r,
                 "void main() { for (int x = 0; x; x++) { } }",
                 "error: 1: expected 'bool', but found 'int'\n1 error\n");
}

DEF_TEST(SkSLConstructorTypeMismatch, r) {
    test_failure(r,
                 "void main() { float2 x = float2(1.0, false); }",
                 "error: 1: expected 'float', but found 'bool'\n1 error\n");
    test_failure(r,
                 "void main() { float2 x = float2(bool2(false)); }",
                 "error: 1: 'bool2' is not a valid parameter to 'float2' constructor\n1 error\n");
    test_failure(r,
                 "void main() { bool2 x = bool2(float2(1)); }",
                 "error: 1: 'float2' is not a valid parameter to 'bool2' constructor\n1 error\n");
    test_failure(r,
                 "void main() { bool x = bool(1.0); }",
                 "error: 1: cannot construct 'bool'\n1 error\n");
    test_failure(r,
                 "struct foo { int x; }; void main() { foo x = foo(5); }",
                 "error: 1: cannot construct 'foo'\n1 error\n");
    test_failure(r,
                 "struct foo { int x; } foo; void main() { float x = float(foo); }",
                 "error: 1: invalid argument to 'float' constructor (expected a number or bool, but found 'foo')\n1 error\n");
    test_failure(r,
                 "struct foo { int x; } foo; void main() { float2 x = float2(foo); }",
                 "error: 1: 'foo' is not a valid parameter to 'float2' constructor\n1 error\n");
    test_failure(r,
                 "void main() { float2x2 x = float2x2(true); }",
                 "error: 1: expected 'float', but found 'bool'\n1 error\n");
}

DEF_TEST(SkSLConstructorArgumentCount, r) {
    test_failure(r,
                 "void main() { float3 x = float3(1.0, 2.0); }",
                 "error: 1: invalid arguments to 'float3' constructor (expected 3 scalars, but "
                 "found 2)\n1 error\n");
    test_failure(r,
                 "void main() { float3 x = float3(1.0, 2.0, 3.0, 4.0); }",
                 "error: 1: invalid arguments to 'float3' constructor (expected 3 scalars, but found "
                 "4)\n1 error\n");
}

DEF_TEST(SkSLSwizzleScalar, r) {
    test_failure(r,
                 "void main() { float x = 1; float y = x.y; }",
                 "error: 1: cannot swizzle value of type 'float'\n1 error\n");
}

DEF_TEST(SkSLSwizzleMatrix, r) {
    test_failure(r,
                 "void main() { float2x2 x = float2x2(1); float y = x.y; }",
                 "error: 1: cannot swizzle value of type 'float2x2'\n1 error\n");
}

DEF_TEST(SkSLSwizzleOutOfBounds, r) {
    test_failure(r,
                 "void main() { float3 test = float2(1).xyz; }",
                 "error: 1: invalid swizzle component 'z'\n1 error\n");
}

DEF_TEST(SkSLSwizzleTooManyComponents, r) {
    test_failure(r,
                 "void main() { float4 test = float2(1).xxxxx; }",
                 "error: 1: too many components in swizzle mask 'xxxxx'\n1 error\n");
}

DEF_TEST(SkSLSwizzleDuplicateOutput, r) {
    test_failure(r,
                 "void main() { float4 test = float4(1); test.xyyz = float4(1); }",
                 "error: 1: cannot write to the same swizzle field more than once\n1 error\n");
}

DEF_TEST(SkSLSwizzleConstantOutput, r) {
    test_failure(r,
                 "void main() { float4 test = float4(1); test.xyz0 = float4(1); }",
                 "error: 1: cannot write to a swizzle mask containing a constant\n1 error\n");
}

DEF_TEST(SkSLAssignmentTypeMismatch, r) {
    test_failure(r,
                 "void main() { int x = 1.0; }",
                 "error: 1: expected 'int', but found 'float'\n1 error\n");
    test_failure(r,
                 "void main() { int x; x = 1.0; }",
                 "error: 1: type mismatch: '=' cannot operate on 'int', 'float'\n1 error\n");
    test_success(r,
                 "void main() { float3 x = float3(0); x *= 1.0; }");
    test_failure(r,
                 "void main() { int3 x = int3(0); x *= 1.0; }",
                 "error: 1: type mismatch: '*=' cannot operate on 'int3', 'float'\n1 error\n");
}

DEF_TEST(SkSLReturnFromVoid, r) {
    test_failure(r,
                 "void main() { return true; }",
                 "error: 1: may not return a value from a void function\n1 error\n");
}

DEF_TEST(SkSLReturnMissingValue, r) {
    test_failure(r,
                 "int foo() { return; } void main() { }",
                 "error: 1: expected function to return 'int'\n1 error\n");
}

DEF_TEST(SkSLReturnTypeMismatch, r) {
    test_failure(r,
                 "int foo() { return 1.0; } void main() { }",
                 "error: 1: expected 'int', but found 'float'\n1 error\n");
}

DEF_TEST(SkSLDuplicateFunction, r) {
    test_failure(r,
                 "void main() { } void main() { }",
                 "error: 1: duplicate definition of void main()\n1 error\n");
    test_success(r,
                 "void main(); void main() { }");
}

DEF_TEST(SkSLUsingInvalidValue, r) {
    test_failure(r,
                 "void main() { int x = int; }",
                 "error: 1: expected '(' to begin constructor invocation\n1 error\n");
    test_failure(r,
                 "int test() { return 1; } void main() { int x = test; }",
                 "error: 1: expected '(' to begin function call\n1 error\n");
}
DEF_TEST(SkSLDifferentReturnType, r) {
    test_failure(r,
                 "int main() { return 1; } void main() { }",
                 "error: 1: functions 'void main()' and 'int main()' differ only in return type\n1 "
                 "error\n");
}

DEF_TEST(SkSLDifferentModifiers, r) {
    test_failure(r,
                 "void test(int x); void test(out int x) { }",
                 "error: 1: modifiers on parameter 1 differ between declaration and definition\n1 "
                 "error\n");
}

DEF_TEST(SkSLDuplicateSymbol, r) {
    test_failure(r,
                 "int main; void main() { }",
                 "error: 1: symbol 'main' was already defined\n1 error\n");

    test_failure(r,
                 "int x; int x; void main() { }",
                 "error: 1: symbol 'x' was already defined\n1 error\n");

    test_success(r, "int x; void main() { int x; }");
}

DEF_TEST(SkSLBinaryTypeMismatch, r) {
    test_failure(r,
                 "void main() { float x = 3 * true; }",
                 "error: 1: type mismatch: '*' cannot operate on 'int', 'bool'\n1 error\n");
    test_failure(r,
                 "void main() { bool x = 1 || 2.0; }",
                 "error: 1: type mismatch: '||' cannot operate on 'int', 'float'\n1 error\n");
    test_failure(r,
                 "void main() { bool x = float2(0) == 0; }",
                 "error: 1: type mismatch: '==' cannot operate on 'float2', 'int'\n1 error\n");
    test_failure(r,
                 "void main() { bool x = float2(0) != 0; }",
                 "error: 1: type mismatch: '!=' cannot operate on 'float2', 'int'\n1 error\n");
}

DEF_TEST(SkSLCallNonFunction, r) {
    test_failure(r,
                 "void main() { float x = 3; x(); }",
                 "error: 1: 'x' is not a function\n1 error\n");
}

DEF_TEST(SkSLInvalidUnary, r) {
    test_failure(r,
                 "void main() { float4x4 x = float4x4(1); ++x; }",
                 "error: 1: '++' cannot operate on 'float4x4'\n1 error\n");
    test_failure(r,
                 "void main() { float3 x = float3(1); --x; }",
                 "error: 1: '--' cannot operate on 'float3'\n1 error\n");
    test_failure(r,
                 "void main() { float4x4 x = float4x4(1); x++; }",
                 "error: 1: '++' cannot operate on 'float4x4'\n1 error\n");
    test_failure(r,
                 "void main() { float3 x = float3(1); x--; }",
                 "error: 1: '--' cannot operate on 'float3'\n1 error\n");
    test_failure(r,
                 "void main() { int x = !12; }",
                 "error: 1: '!' cannot operate on 'int'\n1 error\n");
    test_failure(r,
                 "struct foo { } bar; void main() { foo x = +bar; }",
                 "error: 1: '+' cannot operate on 'foo'\n1 error\n");
    test_failure(r,
                 "struct foo { } bar; void main() { foo x = -bar; }",
                 "error: 1: '-' cannot operate on 'foo'\n1 error\n");
    test_success(r,
                 "void main() { float2 x = float2(1, 1); x = +x; x = -x; }");
}

DEF_TEST(SkSLInvalidAssignment, r) {
    test_failure(r,
                 "void main() { 1 = 2; }",
                 "error: 1: cannot assign to '1'\n1 error\n");
    test_failure(r,
                 "uniform int x; void main() { x = 0; }",
                 "error: 1: cannot modify immutable variable 'x'\n1 error\n");
    test_failure(r,
                 "const int x; void main() { x = 0; }",
                 "error: 1: cannot modify immutable variable 'x'\n1 error\n");
}

DEF_TEST(SkSLBadIndex, r) {
    test_failure(r,
                 "void main() { int x = 2[0]; }",
                 "error: 1: expected array, but found 'int'\n1 error\n");
    test_failure(r,
                 "void main() { float2 x = float2(0); int y = x[0][0]; }",
                 "error: 1: expected array, but found 'float'\n1 error\n");
}

DEF_TEST(SkSLTernaryMismatch, r) {
    test_failure(r,
                 "void main() { int x = 5 > 2 ? true : 1.0; }",
                 "error: 1: ternary operator result mismatch: 'bool', 'float'\n1 error\n");
    test_failure(r,
                 "void main() { int x = 5 > 2 ? float3(1) : 1.0; }",
                 "error: 1: ternary operator result mismatch: 'float3', 'float'\n1 error\n");
}

DEF_TEST(SkSLInterfaceBlockStorageModifiers, r) {
    test_failure(r,
                 "uniform foo { out int x; };",
                 "error: 1: interface block fields may not have storage qualifiers\n1 error\n");
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
                 "void main() { return; while (true); }",
                 "error: 1: unreachable\n1 error\n");
}

DEF_TEST(SkSLNoReturn, r) {
    test_failure(r,
                 "int foo() { if (2 > 5) return 3; }",
                 "error: 1: function can exit without returning a value\n1 error\n");
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
                 "void main() { float x = gl_FragCoord.x; };",
                 "error: 1: unknown identifier 'gl_FragCoord'\n1 error\n");
    test_failure(r,
                 "void main() { float r = gl_FragColor.r; };",
                 "error: 1: unknown identifier 'gl_FragColor'\n1 error\n");
}

DEF_TEST(SkSLWrongSwitchTypes, r) {
    test_failure(r,
                 "void main() { switch (float2(1)) { case 1: break; } }",
                 "error: 1: expected 'int', but found 'float2'\n1 error\n");
    test_failure(r,
                 "void main() { switch (1) { case float2(1): break; } }",
                 "error: 1: expected 'int', but found 'float2'\n1 error\n");
}

DEF_TEST(SkSLNonConstantCase, r) {
    test_failure(r,
                 "void main() { int x = 1; switch (1) { case x: break; } }",
                 "error: 1: case value must be a constant\n1 error\n");
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

DEF_TEST(SkSLStaticSwitch, r) {
    test_success(r,
                 "void main() {"
                 "int x = 1;"
                 "@switch (x) {"
                 "case 1: sk_FragColor = half4(1); break;"
                 "default: sk_FragColor = half4(0);"
                 "}"
                 "}");
    test_failure(r,
                 "void main() {"
                 "int x = int(sqrt(1));"
                 "@switch (x) {"
                 "case 1: sk_FragColor = half4(1); break;"
                 "default: sk_FragColor = half4(0);"
                 "}"
                 "}",
                 "error: 1: static switch has non-static test\n1 error\n");
    test_failure(r,
                 "void main() {"
                 "int x = 1;"
                 "@switch (x) {"
                 "case 1: sk_FragColor = half4(1); if (sqrt(0) < sqrt(1)) break;"
                 "default: sk_FragColor = half4(0);"
                 "}"
                 "}",
                 "error: 1: static switch contains non-static conditional break\n1 error\n");
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

DEF_TEST(SkSLConstantSwizzleNotLast, r) {
    test_failure(r,
                 "void main() { sk_FragColor = half4(1).rg00; }",
                 "error: 1: only the last swizzle component can be a constant\n1 error\n");
}
