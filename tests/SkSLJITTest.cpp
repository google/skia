/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLJIT.h"

#include "tests/Test.h"

#ifdef SK_LLVM_AVAILABLE

template<typename type>
void test(skiatest::Reporter* r, const char* src, type x, type y, type result) {
    SkSL::Compiler compiler;
    SkSL::Program::Settings settings;
    std::unique_ptr<SkSL::Program> program = compiler.convertProgram(
                                                                 SkSL::Program::kPipelineStage_Kind,
                                                                 SkSL::String(src), settings);
    REPORTER_ASSERT(r, program);
    if (program) {
       SkSL::JIT jit(&compiler);
       std::unique_ptr<SkSL::JIT::Module> module = jit.compile(std::move(program));
       type (*test)(type, type) = (type(*)(type, type)) module->getSymbol("test");
       REPORTER_ASSERT(r, test(x, y) == result);
    } else {
        printf("%s", compiler.errorText().c_str());
    }
}

DEF_TEST(SkSLJITAdd, r) {
    test<int>(r, "int test(int x, int y) { return x + y; }", 12, 5, 17);
    test<float>(r, "float test(float x, float y) { return x + y; }", -1, 76, 75);
    test<int>(r, "int test(int x, int y) { x += y; return x; }", 12, 5, 17);
    test<float>(r, "float test(float x, float y) { x += y; return x; }", -1, 76, 75);
    test<int>(r, "int test(int x, int y) { return (int2(x) + int2(y)).x; }", 0, -100, -100);
    test<float>(r, "float test(float x, float y) { return (float2(x) + float2(y)).x; }", 36, 6, 42);
}

DEF_TEST(SkSLJITSub, r) {
    test<int>(r, "int test(int x, int y) { return x - y; }", 12, 5, 7);
    test<float>(r, "float test(float x, float y) { return x - y; }", -1, 76, -77);
    test<int>(r, "int test(int x, int y) { x -= y; return x; }", 12, 5, 7);
    test<float>(r, "float test(float x, float y) { x -= y; return x; }", -1, 76, -77);
    test<int>(r, "int test(int x, int y) { return (int2(x) - int2(y)).x; }", 0, -100, 100);
    test<float>(r, "float test(float x, float y) { return (float2(x) - float2(y)).x; }", 36, 6, 30);
}

DEF_TEST(SkSLJITMul, r) {
    test<int>(r, "int test(int x, int y) { return x * y; }", 12, 5, 60);
    test<float>(r, "float test(float x, float y) { return x * y; }", -1, 76, -76);
    test<int>(r, "int test(int x, int y) { x *= y; return x; }", 12, 5, 60);
    test<float>(r, "float test(float x, float y) { x *= y; return x; }", -1, 76, -76);
    test<int>(r, "int test(int x, int y) { return (int2(x) * int2(y)).x; }", 0, -100, 0);
    test<float>(r, "float test(float x, float y) { return (float2(x) * float2(y)).x; }", 36, 6,
                216);
}

DEF_TEST(SkSLJITDiv, r) {
    test<int>(r, "int test(int x, int y) { return x / y; }", 12, 5, 2);
    test<float>(r, "float test(float x, float y) { return x / y; }", -1, 76, -1.0 / 76.0);
    test<int>(r, "int test(int x, int y) { x /= y; return x; }", 12, 5, 2);
    test<float>(r, "float test(float x, float y) { x /= y; return x; }", -1, 76, -1.0 / 76.0);
    test<int>(r, "int test(int x, int y) { return (int2(x) / int2(y)).x; }", 0, -100, 0);
    test<float>(r, "float test(float x, float y) { return (float2(x) / float2(y)).x; }", 36, 6,
                6);
}

DEF_TEST(SkSLJITOr, r) {
    test<int>(r, "int test(int x, int y) { return x | y; }", 45, 15, 47);
    test<int>(r, "int test(int x, int y) { x |= y; return x; }", 45, 15, 47);
}

DEF_TEST(SkSLJITAnd, r) {
    test<int>(r, "int test(int x, int y) { return x & y; }", 45, 15, 13);
    test<int>(r, "int test(int x, int y) { x &= y; return x; }", 45, 15, 13);
}

DEF_TEST(SkSLJITIf, r) {
    test<int>(r, "int test(int x, int y) { if (x > y) return x; else return y; }", 17, 8, 17);
    test<int>(r, "int test(int x, int y) { if (x > y) return x; else return y; }", 8, 17, 17);
    test<int>(r, "int test(int x, int y) { if (x > y) if (x > 0) return x; else return -x; "
                 "else if (y > 0) return y; else return -y; }", -8, -17, 8);
}

DEF_TEST(SkSLJITTernary, r) {
    test<int>(r, "int test(int x, int y) { return x > y ? x : y; }", 17, 8, 17);
    test<int>(r, "int test(int x, int y) { return x > y ? x : y; }", 8, 17, 17);
    test<int>(r, "int test(int x, int y) { return x > y ? (x > 0 ? x : -x) :"
                 "(y > 0 ? y : -y); }", -8, -17, 8);
}

DEF_TEST(SkSLJITFor, r) {
    test<int>(r, "int test(int x, int y) {"
                 "    int result = 0;"
                 "    for (int i = 0; i < x; ++i)"
                 "        result += y;"
                 "    return result;"
                 "}", 124, 17, 2108);
}

DEF_TEST(SkSLJITDo, r) {
    test<int>(r, "int test(int x, int y) {"
                 "    int result = -10;"
                 "    do { result = 0; } while (false);"
                 "    do { result += x; } while (result < y);"
                 "    return result;"
                 "}", 96, 200, 288);
}

DEF_TEST(SkSLJITWhile, r) {
    test<int>(r, "int test(int x, int y) {"
                 "    int result = 0;"
                 "    while (false) { result = -10; }"
                 "    while (result < y) { result += x; }"
                 "    return result;"
                 "}", 96, 200, 288);
}

#endif
