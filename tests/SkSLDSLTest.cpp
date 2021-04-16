/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkSLIRNode.h"
#include "include/sksl/DSL.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/dsl/priv/DSLWriter.h"

#include "tests/Test.h"

#include <limits>

using namespace SkSL::dsl;

/**
 * In addition to issuing an automatic Start() and End(), disables mangling and optionally
 * auto-declares variables during its lifetime. Variable auto-declaration simplifies testing so we
 * don't have to sprinkle all the tests with a bunch of Declare(foo).release() calls just to avoid
 * errors, especially given that some of the variables have options that make them an error to
 * actually declare.
 */
class AutoDSLContext {
public:
    AutoDSLContext(GrGpu* gpu, bool markVarsDeclared = true,
                   SkSL::ProgramKind kind = SkSL::ProgramKind::kFragment) {
        Start(gpu->shaderCompiler(), kind);
        DSLWriter::Instance().fMangle = false;
        DSLWriter::Instance().fMarkVarsDeclared = markVarsDeclared;
    }

    ~AutoDSLContext() {
        End();
    }
};

class ExpectError : public ErrorHandler {
public:
    ExpectError(skiatest::Reporter* reporter, const char* msg)
        : fMsg(msg)
        , fReporter(reporter) {
        SetErrorHandler(this);
    }

    ~ExpectError() override {
        REPORTER_ASSERT(fReporter, !fMsg,
                        "Error mismatch: expected:\n%sbut no error occurred\n", fMsg);
        SetErrorHandler(nullptr);
    }

    void handleError(const char* msg, PositionInfo* pos) override {
        REPORTER_ASSERT(fReporter, !strcmp(msg, fMsg),
                        "Error mismatch: expected:\n%sbut received:\n%s", fMsg, msg);
        fMsg = nullptr;
    }

private:
    const char* fMsg;
    skiatest::Reporter* fReporter;
};

static bool whitespace_insensitive_compare(const char* a, const char* b) {
    for (;;) {
        while (isspace(*a)) {
            ++a;
        }
        while (isspace(*b)) {
            ++b;
        }
        if (*a != *b) {
            return false;
        }
        if (*a == 0) {
            return true;
        }
        ++a;
        ++b;
    }
}

// for use from SkSLDSLOnlyTest.cpp
void StartDSL(const sk_gpu_test::ContextInfo ctxInfo) {
    Start(ctxInfo.directContext()->priv().getGpu()->shaderCompiler());
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLStartup, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Expression e1 = 1;
    REPORTER_ASSERT(r, e1.release()->description() == "1");
    Expression e2 = 1.0;
    REPORTER_ASSERT(r, e2.release()->description() == "1.0");
    Expression e3 = true;
    REPORTER_ASSERT(r, e3.release()->description() == "true");
    Var a(kInt_Type, "a");
    Expression e4 = a;
    REPORTER_ASSERT(r, e4.release()->description() == "a");

    REPORTER_ASSERT(r, whitespace_insensitive_compare("", ""));
    REPORTER_ASSERT(r, !whitespace_insensitive_compare("", "a"));
    REPORTER_ASSERT(r, !whitespace_insensitive_compare("a", ""));
    REPORTER_ASSERT(r, whitespace_insensitive_compare("a", "a"));
    REPORTER_ASSERT(r, whitespace_insensitive_compare("abc", "abc"));
    REPORTER_ASSERT(r, whitespace_insensitive_compare("abc", " abc "));
    REPORTER_ASSERT(r, whitespace_insensitive_compare("a b  c  ", "\n\n\nabc"));
    REPORTER_ASSERT(r, !whitespace_insensitive_compare("a b c  d", "\n\n\nabc"));
}

static SkSL::String stringize(DSLStatement& stmt)          { return stmt.release()->description(); }
static SkSL::String stringize(DSLPossibleStatement& stmt)  { return stmt.release()->description(); }
static SkSL::String stringize(DSLExpression& expr)         { return expr.release()->description(); }
static SkSL::String stringize(DSLPossibleExpression& expr) { return expr.release()->description(); }
static SkSL::String stringize(SkSL::IRNode& node)  { return node.description(); }

template <typename T>
static void expect_equal(skiatest::Reporter* r, int lineNumber, T& input, const char* expected) {
    SkSL::String actual = stringize(input);
    if (!whitespace_insensitive_compare(expected, actual.c_str())) {
        ERRORF(r, "(Failed on line %d)\nExpected: %s\n  Actual: %s\n",
                  lineNumber, expected, actual.c_str());
    }
}

template <typename T>
static void expect_equal(skiatest::Reporter* r, int lineNumber, T&& dsl, const char* expected) {
    // This overload allows temporary values to be passed to expect_equal.
    return expect_equal(r, lineNumber, dsl, expected);
}

#define EXPECT_EQUAL(a, b)  expect_equal(r, __LINE__, (a), (b))

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLFloat, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Expression e1 = Float(std::numeric_limits<float>::max());
    REPORTER_ASSERT(r, atof(e1.release()->description().c_str()) ==
                       std::numeric_limits<float>::max());

    Expression e2 = Float(std::numeric_limits<float>::min());
    REPORTER_ASSERT(r, atof(e2.release()->description().c_str()) ==
                       std::numeric_limits<float>::min());

    EXPECT_EQUAL(Float2(0),
                "float2(0.0)");
    EXPECT_EQUAL(Float2(-0.5, 1),
                "float2(-0.5, 1.0)");
    EXPECT_EQUAL(Float3(0.75),
                "float3(0.75)");
    EXPECT_EQUAL(Float3(Float2(0, 1), -2),
                "float3(0.0, 1.0, -2.0)");
    EXPECT_EQUAL(Float3(0, 1, 2),
                "float3(0.0, 1.0, 2.0)");
    EXPECT_EQUAL(Float4(0),
                "float4(0.0)");
    EXPECT_EQUAL(Float4(Float2(0, 1), Float2(2, 3)),
                "float4(0.0, 1.0, 2.0, 3.0)");
    EXPECT_EQUAL(Float4(0, 1, Float2(2, 3)),
                "float4(0.0, 1.0, 2.0, 3.0)");
    EXPECT_EQUAL(Float4(0, 1, 2, 3),
                "float4(0.0, 1.0, 2.0, 3.0)");

    {
        ExpectError error(r, "error: floating point value is infinite\n");
        Float(std::numeric_limits<float>::infinity()).release();
    }

    {
        ExpectError error(r, "error: floating point value is NaN\n");
        Float(std::numeric_limits<float>::quiet_NaN()).release();
    }

    {
        ExpectError error(r, "error: invalid arguments to 'float2' constructor (expected 2 scalars,"
                             " but found 4)\n");
        Float2(Float4(1)).release();
    }

    {
        ExpectError error(r, "error: invalid arguments to 'float4' constructor (expected 4 scalars,"
                             " but found 3)\n");
        Float4(Float3(1)).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLHalf, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Expression e1 = Half(std::numeric_limits<float>::max());
    REPORTER_ASSERT(r,
                    atof(e1.release()->description().c_str()) == std::numeric_limits<float>::max());

    Expression e2 = Half(std::numeric_limits<float>::min());
    REPORTER_ASSERT(r,
                    atof(e2.release()->description().c_str()) == std::numeric_limits<float>::min());

    EXPECT_EQUAL(Half2(0),
                "half2(0.0)");
    EXPECT_EQUAL(Half2(-0.5, 1),
                "half2(-0.5, 1.0)");
    EXPECT_EQUAL(Half3(0.75),
                "half3(0.75)");
    EXPECT_EQUAL(Half3(Half2(0, 1), -2),
                "half3(0.0, 1.0, -2.0)");
    EXPECT_EQUAL(Half3(0, 1, 2),
                "half3(0.0, 1.0, 2.0)");
    EXPECT_EQUAL(Half4(0),
                "half4(0.0)");
    EXPECT_EQUAL(Half4(Half2(0, 1), Half2(2, 3)),
                "half4(0.0, 1.0, 2.0, 3.0)");
    EXPECT_EQUAL(Half4(0, 1, Half2(2, 3)),
                "half4(0.0, 1.0, 2.0, 3.0)");
    EXPECT_EQUAL(Half4(0, 1, 2, 3),
                "half4(0.0, 1.0, 2.0, 3.0)");

    {
        ExpectError error(r, "error: floating point value is infinite\n");
        Half(std::numeric_limits<float>::infinity()).release();
    }

    {
        ExpectError error(r, "error: floating point value is NaN\n");
        Half(std::numeric_limits<float>::quiet_NaN()).release();
    }

    {
        ExpectError error(r, "error: invalid arguments to 'half2' constructor (expected 2 scalars,"
                             " but found 4)\n");
        Half2(Half4(1)).release();
    }

    {
        ExpectError error(r, "error: invalid arguments to 'half4' constructor (expected 4 scalars,"
                             " but found 3)\n");
        Half4(Half3(1)).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLInt, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());

    EXPECT_EQUAL(Int(std::numeric_limits<int32_t>::max()),
                "2147483647");
    EXPECT_EQUAL(Int2(std::numeric_limits<int32_t>::min()),
                "int2(-2147483648)");
    EXPECT_EQUAL(Int2(0, 1),
                "int2(0, 1)");
    EXPECT_EQUAL(Int3(0),
                "int3(0)");
    EXPECT_EQUAL(Int3(Int2(0, 1), -2),
                "int3(0, 1, -2)");
    EXPECT_EQUAL(Int3(0, 1, 2),
                "int3(0, 1, 2)");
    EXPECT_EQUAL(Int4(0),
                "int4(0)");
    EXPECT_EQUAL(Int4(Int2(0, 1), Int2(2, 3)),
                "int4(0, 1, 2, 3)");
    EXPECT_EQUAL(Int4(0, 1, Int2(2, 3)),
                "int4(0, 1, 2, 3)");
    EXPECT_EQUAL(Int4(0, 1, 2, 3),
                "int4(0, 1, 2, 3)");

    {
        ExpectError error(r, "error: invalid arguments to 'int2' constructor (expected 2 scalars,"
                             " but found 4)\n");
        Int2(Int4(1)).release();
    }

    {
        ExpectError error(r, "error: invalid arguments to 'int4' constructor (expected 4 scalars,"
                             " but found 3)\n");
        Int4(Int3(1)).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLShort, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());

    EXPECT_EQUAL(Short(std::numeric_limits<int16_t>::max()),
                "32767");
    EXPECT_EQUAL(Short2(std::numeric_limits<int16_t>::min()),
                "short2(-32768)");
    EXPECT_EQUAL(Short2(0, 1),
                "short2(0, 1)");
    EXPECT_EQUAL(Short3(0),
                "short3(0)");
    EXPECT_EQUAL(Short3(Short2(0, 1), -2),
                "short3(0, 1, -2)");
    EXPECT_EQUAL(Short3(0, 1, 2),
                "short3(0, 1, 2)");
    EXPECT_EQUAL(Short4(0),
                "short4(0)");
    EXPECT_EQUAL(Short4(Short2(0, 1), Short2(2, 3)),
                "short4(0, 1, 2, 3)");
    EXPECT_EQUAL(Short4(0, 1, Short2(2, 3)),
                "short4(0, 1, 2, 3)");
    EXPECT_EQUAL(Short4(0, 1, 2, 3),
                "short4(0, 1, 2, 3)");

    {
        ExpectError error(r, "error: invalid arguments to 'short2' constructor (expected 2 scalars,"
                             " but found 4)\n");
        Short2(Short4(1)).release();
    }

    {
        ExpectError error(r, "error: invalid arguments to 'short4' constructor (expected 4 scalars,"
                             " but found 3)\n");
        Short4(Short3(1)).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLBool, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());

    EXPECT_EQUAL(Bool2(false),
                "bool2(false)");
    EXPECT_EQUAL(Bool2(false, true),
                "bool2(false, true)");
    EXPECT_EQUAL(Bool3(false),
                "bool3(false)");
    EXPECT_EQUAL(Bool3(Bool2(false, true), false),
                "bool3(false, true, false)");
    EXPECT_EQUAL(Bool3(false, true, false),
                "bool3(false, true, false)");
    EXPECT_EQUAL(Bool4(false),
                "bool4(false)");
    EXPECT_EQUAL(Bool4(Bool2(false, true), Bool2(false, true)),
                "bool4(false, true, false, true)");
    EXPECT_EQUAL(Bool4(false, true, Bool2(false, true)),
                "bool4(false, true, false, true)");
    EXPECT_EQUAL(Bool4(false, true, false, true),
                "bool4(false, true, false, true)");

    {
        ExpectError error(r, "error: invalid arguments to 'bool2' constructor (expected 2 scalars,"
                             " but found 4)\n");
        Bool2(Bool4(true)).release();
    }

    {
        ExpectError error(r, "error: invalid arguments to 'bool4' constructor (expected 4 scalars,"
                             " but found 3)\n");
        Bool4(Bool3(true)).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLMatrices, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var f22(kFloat2x2_Type, "f22");
    EXPECT_EQUAL(f22 = Float2x2(1), "(f22 = float2x2(1.0))");
    Var f32(kFloat3x2_Type, "f32");
    EXPECT_EQUAL(f32 = Float3x2(1, 2, 3, 4, 5, 6),
                 "(f32 = float3x2(1.0, 2.0, 3.0, 4.0, 5.0, 6.0))");
    Var f42(kFloat4x2_Type, "f42");
    EXPECT_EQUAL(f42 = Float4x2(Float4(1, 2, 3, 4), 5, 6, 7, 8),
                 "(f42 = float4x2(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0))");
    Var f23(kFloat2x3_Type, "f23");
    EXPECT_EQUAL(f23 = Float2x3(1, Float2(2, 3), 4, Float2(5, 6)),
                 "(f23 = float2x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0))");
    Var f33(kFloat3x3_Type, "f33");
    EXPECT_EQUAL(f33 = Float3x3(Float3(1, 2, 3), 4, Float2(5, 6), 7, 8, 9),
                 "(f33 = float3x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0))");
    Var f43(kFloat4x3_Type, "f43");
    EXPECT_EQUAL(f43 = Float4x3(Float4(1, 2, 3, 4), Float4(5, 6, 7, 8), Float4(9, 10, 11, 12)),
                 "(f43 = float4x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0))");
    Var f24(kFloat2x4_Type, "f24");
    EXPECT_EQUAL(f24 = Float2x4(1, 2, 3, 4, 5, 6, 7, 8),
                 "(f24 = float2x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0))");
    Var f34(kFloat3x4_Type, "f34");
    EXPECT_EQUAL(f34 = Float3x4(1, 2, 3, 4, 5, 6, 7, 8, 9, Float3(10, 11, 12)),
                 "(f34 = float3x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0))");
    Var f44(kFloat4x4_Type, "f44");
    EXPECT_EQUAL(f44 = Float4x4(1), "(f44 = float4x4(1.0))");

    Var h22(kHalf2x2_Type, "h22");
    EXPECT_EQUAL(h22 = Half2x2(1), "(h22 = half2x2(1.0))");
    Var h32(kHalf3x2_Type, "h32");
    EXPECT_EQUAL(h32 = Half3x2(1, 2, 3, 4, 5, 6),
                 "(h32 = half3x2(1.0, 2.0, 3.0, 4.0, 5.0, 6.0))");
    Var h42(kHalf4x2_Type, "h42");
    EXPECT_EQUAL(h42 = Half4x2(Half4(1, 2, 3, 4), 5, 6, 7, 8),
                 "(h42 = half4x2(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0))");
    Var h23(kHalf2x3_Type, "h23");
    EXPECT_EQUAL(h23 = Half2x3(1, Half2(2, 3), 4, Half2(5, 6)),
                 "(h23 = half2x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0))");
    Var h33(kHalf3x3_Type, "h33");
    EXPECT_EQUAL(h33 = Half3x3(Half3(1, 2, 3), 4, Half2(5, 6), 7, 8, 9),
                 "(h33 = half3x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0))");
    Var h43(kHalf4x3_Type, "h43");
    EXPECT_EQUAL(h43 = Half4x3(Half4(1, 2, 3, 4), Half4(5, 6, 7, 8), Half4(9, 10, 11, 12)),
                 "(h43 = half4x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0))");
    Var h24(kHalf2x4_Type, "h24");
    EXPECT_EQUAL(h24 = Half2x4(1, 2, 3, 4, 5, 6, 7, 8),
                 "(h24 = half2x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0))");
    Var h34(kHalf3x4_Type, "h34");
    EXPECT_EQUAL(h34 = Half3x4(1, 2, 3, 4, 5, 6, 7, 8, 9, Half3(10, 11, 12)),
                 "(h34 = half3x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0))");
    Var h44(kHalf4x4_Type, "h44");
    EXPECT_EQUAL(h44 = Half4x4(1), "(h44 = half4x4(1.0))");

    EXPECT_EQUAL(f22 * 2, "(f22 * 2.0)");
    EXPECT_EQUAL(f22 == Float2x2(1), "(f22 == float2x2(1.0))");
    EXPECT_EQUAL(h42[0][1], "h42[0].y");
    EXPECT_EQUAL(f43 * Float4(0), "(f43 * float4(0.0))");
    EXPECT_EQUAL(h23 * 2, "(h23 * 2.0)");
    EXPECT_EQUAL(Inverse(f44), "inverse(f44)");

    {
        ExpectError error(r, "error: invalid arguments to 'float3x3' constructor (expected 9 "
                             "scalars, but found 2)\n");
        DSLExpression(Float3x3(Float2(1))).release();
    }

    {
        ExpectError error(r, "error: invalid arguments to 'half2x2' constructor (expected 4 "
                             "scalars, but found 5)\n");
        DSLExpression(Half2x2(1, 2, 3, 4, 5)).release();
    }

    {
        ExpectError error(r, "error: type mismatch: '*' cannot operate on 'float4x3', 'float3'\n");
        DSLExpression(f43 * Float3(1)).release();
    }

    {
        ExpectError error(r, "error: type mismatch: '=' cannot operate on 'float4x3', "
                             "'float3x3'\n");
        DSLExpression(f43 = f33).release();
    }

    {
        ExpectError error(r, "error: type mismatch: '=' cannot operate on 'half2x2', "
                             "'float2x2'\n");
        DSLExpression(h22 = f22).release();
    }

    {
        ExpectError error(r,
                          "error: no match for inverse(float4x3)\n");
        DSLExpression(Inverse(f43)).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLPlus, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kFloat_Type, "a"), b(kFloat_Type, "b");

    EXPECT_EQUAL(a + b,
               "(a + b)");
    EXPECT_EQUAL(a + 1,
               "(a + 1.0)");
    EXPECT_EQUAL(0.5 + a + -99,
              "((0.5 + a) + -99.0)");
    EXPECT_EQUAL(a += b + 1,
               "(a += (b + 1.0))");
    EXPECT_EQUAL(+a,
                 "a");
    EXPECT_EQUAL(+(a + b),
                 "(a + b)");

    {
        ExpectError error(r, "error: type mismatch: '+' cannot operate on 'bool2', 'float'\n");
        DSLExpression((Bool2(true) + a)).release();
    }

    {
        ExpectError error(r, "error: type mismatch: '+=' cannot operate on 'float', 'bool2'\n");
        DSLExpression((a += Bool2(true))).release();
    }

    {
        ExpectError error(r, "error: cannot assign to this expression\n");
        DSLExpression((1.0 += a)).release();
    }

    {
        ExpectError error(r, "error: '+' cannot operate on 'bool'\n");
        Var c(kBool_Type);
        DSLExpression(+c);
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLMinus, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kInt_Type, "b");

    EXPECT_EQUAL(a - b,
               "(a - b)");
    EXPECT_EQUAL(a - 1,
               "(a - 1)");
    EXPECT_EQUAL(2 - a - b,
              "((2 - a) - b)");
    EXPECT_EQUAL(a -= b + 1,
               "(a -= (b + 1))");
    EXPECT_EQUAL(-a,
                "-a");
    EXPECT_EQUAL(-(a - b),
                "-(a - b)");

    {
        ExpectError error(r, "error: type mismatch: '-' cannot operate on 'bool2', 'int'\n");
        DSLExpression(Bool2(true) - a).release();
    }

    {
        ExpectError error(r, "error: type mismatch: '-=' cannot operate on 'int', 'bool2'\n");
        DSLExpression(a -= Bool2(true)).release();
    }

    {
        ExpectError error(r, "error: cannot assign to this expression\n");
        DSLExpression(1.0 -= a).release();
    }

    {
        ExpectError error(r, "error: '-' cannot operate on 'bool'\n");
        Var c(kBool_Type);
        DSLExpression(-c);
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLMultiply, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kFloat_Type, "a"), b(kFloat_Type, "b");

    EXPECT_EQUAL(a * b,
               "(a * b)");
    EXPECT_EQUAL(a * 2,
               "(a * 2.0)");
    EXPECT_EQUAL(0.5 * a * -99,
              "((0.5 * a) * -99.0)");
    EXPECT_EQUAL(a *= b + 1,
               "(a *= (b + 1.0))");

    {
        ExpectError error(r, "error: type mismatch: '*' cannot operate on 'bool2', 'float'\n");
        DSLExpression(Bool2(true) * a).release();
    }

    {
        ExpectError error(r, "error: type mismatch: '*=' cannot operate on 'float', 'bool2'\n");
        DSLExpression(a *= Bool2(true)).release();
    }

    {
        ExpectError error(r, "error: cannot assign to this expression\n");
        DSLExpression(1.0 *= a).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLDivide, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kFloat_Type, "a"), b(kFloat_Type, "b");

    EXPECT_EQUAL(a / b,
               "(a / b)");
    EXPECT_EQUAL(a / 2,
               "(a / 2.0)");
    EXPECT_EQUAL(0.5 / a / -99,
              "((0.5 / a) / -99.0)");
    EXPECT_EQUAL(b / (a - 1),
               "(b / (a - 1.0))");
    EXPECT_EQUAL(a /= b + 1,
               "(a /= (b + 1.0))");

    {
        ExpectError error(r, "error: type mismatch: '/' cannot operate on 'bool2', 'float'\n");
        DSLExpression(Bool2(true) / a).release();
    }

    {
        ExpectError error(r, "error: type mismatch: '/=' cannot operate on 'float', 'bool2'\n");
        DSLExpression(a /= Bool2(true)).release();
    }

    {
        ExpectError error(r, "error: cannot assign to this expression\n");
        DSLExpression(1.0 /= a).release();
    }

    {
        ExpectError error(r, "error: division by zero\n");
        DSLExpression(a /= 0).release();
    }

    {
        Var c(kFloat2_Type, "c");
        ExpectError error(r, "error: division by zero\n");
        DSLExpression(c /= Float2(Float(0), 1)).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLMod, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kInt_Type, "b");
    Expression e1 = a % b;
    EXPECT_EQUAL(e1, "(a % b)");

    Expression e2 = a % 2;
    EXPECT_EQUAL(e2, "(a % 2)");

    Expression e3 = 10 % a % -99;
    EXPECT_EQUAL(e3, "((10 % a) % -99)");

    Expression e4 = a %= b + 1;
    EXPECT_EQUAL(e4, "(a %= (b + 1))");

    {
        ExpectError error(r, "error: type mismatch: '%' cannot operate on 'bool2', 'int'\n");
        DSLExpression(Bool2(true) % a).release();
    }

    {
        ExpectError error(r, "error: type mismatch: '%=' cannot operate on 'int', 'bool2'\n");
        DSLExpression(a %= Bool2(true)).release();
    }

    {
        ExpectError error(r, "error: cannot assign to this expression\n");
        DSLExpression(1 %= a).release();
    }

    {
        ExpectError error(r, "error: division by zero\n");
        DSLExpression(a %= 0).release();
    }

    {
        Var c(kInt2_Type, "c");
        ExpectError error(r, "error: division by zero\n");
        DSLExpression(c %= Int2(Int(0), 1)).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLShl, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kInt_Type, "b");
    Expression e1 = a << b;
    EXPECT_EQUAL(e1, "(a << b)");

    Expression e2 = a << 1;
    EXPECT_EQUAL(e2, "(a << 1)");

    Expression e3 = 1 << a << 2;
    EXPECT_EQUAL(e3, "((1 << a) << 2)");

    Expression e4 = a <<= b + 1;
    EXPECT_EQUAL(e4, "(a <<= (b + 1))");

    {
        ExpectError error(r, "error: type mismatch: '<<' cannot operate on 'bool2', 'int'\n");
        DSLExpression(Bool2(true) << a).release();
    }

    {
        ExpectError error(r, "error: type mismatch: '<<=' cannot operate on 'int', 'bool2'\n");
        DSLExpression(a <<= Bool2(true)).release();
    }

    {
        ExpectError error(r, "error: cannot assign to this expression\n");
        DSLExpression(1 <<= a).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLShr, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kInt_Type, "b");
    Expression e1 = a >> b;
    EXPECT_EQUAL(e1, "(a >> b)");

    Expression e2 = a >> 1;
    EXPECT_EQUAL(e2, "(a >> 1)");

    Expression e3 = 1 >> a >> 2;
    EXPECT_EQUAL(e3, "((1 >> a) >> 2)");

    Expression e4 = a >>= b + 1;
    EXPECT_EQUAL(e4, "(a >>= (b + 1))");

    {
        ExpectError error(r, "error: type mismatch: '>>' cannot operate on 'bool2', 'int'\n");
        DSLExpression(Bool2(true) >> a).release();
    }

    {
        ExpectError error(r, "error: type mismatch: '>>=' cannot operate on 'int', 'bool2'\n");
        DSLExpression(a >>= Bool2(true)).release();
    }

    {
        ExpectError error(r, "error: cannot assign to this expression\n");
        DSLExpression(1 >>= a).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLBitwiseAnd, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kInt_Type, "b");
    Expression e1 = a & b;
    EXPECT_EQUAL(e1, "(a & b)");

    Expression e2 = a & 1;
    EXPECT_EQUAL(e2, "(a & 1)");

    Expression e3 = 1 & a & 2;
    EXPECT_EQUAL(e3, "((1 & a) & 2)");

    Expression e4 = a &= b + 1;
    EXPECT_EQUAL(e4, "(a &= (b + 1))");

    {
        ExpectError error(r, "error: type mismatch: '&' cannot operate on 'bool2', 'int'\n");
        DSLExpression(Bool2(true) & a).release();
    }

    {
        ExpectError error(r, "error: type mismatch: '&=' cannot operate on 'int', 'bool2'\n");
        DSLExpression(a &= Bool2(true)).release();
    }

    {
        ExpectError error(r, "error: cannot assign to this expression\n");
        DSLExpression(1 &= a).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLBitwiseOr, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kInt_Type, "b");
    Expression e1 = a | b;
    EXPECT_EQUAL(e1, "(a | b)");

    Expression e2 = a | 1;
    EXPECT_EQUAL(e2, "(a | 1)");

    Expression e3 = 1 | a | 2;
    EXPECT_EQUAL(e3, "((1 | a) | 2)");

    Expression e4 = a |= b + 1;
    EXPECT_EQUAL(e4, "(a |= (b + 1))");

    {
        ExpectError error(r, "error: type mismatch: '|' cannot operate on 'bool2', 'int'\n");
        DSLExpression(Bool2(true) | a).release();
    }

    {
        ExpectError error(r, "error: type mismatch: '|=' cannot operate on 'int', 'bool2'\n");
        DSLExpression(a |= Bool2(true)).release();
    }

    {
        ExpectError error(r, "error: cannot assign to this expression\n");
        DSLExpression(1 |= a).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLBitwiseXor, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kInt_Type, "b");
    Expression e1 = a ^ b;
    EXPECT_EQUAL(e1, "(a ^ b)");

    Expression e2 = a ^ 1;
    EXPECT_EQUAL(e2, "(a ^ 1)");

    Expression e3 = 1 ^ a ^ 2;
    EXPECT_EQUAL(e3, "((1 ^ a) ^ 2)");

    Expression e4 = a ^= b + 1;
    EXPECT_EQUAL(e4, "(a ^= (b + 1))");

    {
        ExpectError error(r, "error: type mismatch: '^' cannot operate on 'bool2', 'int'\n");
        DSLExpression(Bool2(true) ^ a).release();
    }

    {
        ExpectError error(r, "error: type mismatch: '^=' cannot operate on 'int', 'bool2'\n");
        DSLExpression(a ^= Bool2(true)).release();
    }

    {
        ExpectError error(r, "error: cannot assign to this expression\n");
        DSLExpression(1 ^= a).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLLogicalAnd, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kBool_Type, "a"), b(kBool_Type, "b");
    Expression e1 = a && b;
    EXPECT_EQUAL(e1, "(a && b)");

    Expression e2 = a && true && b;
    EXPECT_EQUAL(e2, "(a && b)");

    Expression e3 = a && false && b;
    EXPECT_EQUAL(e3, "false");

    {
        ExpectError error(r, "error: type mismatch: '&&' cannot operate on 'bool', 'int'\n");
        DSLExpression(a && 5).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLLogicalOr, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kBool_Type, "a"), b(kBool_Type, "b");
    Expression e1 = a || b;
    EXPECT_EQUAL(e1, "(a || b)");

    Expression e2 = a || true || b;
    EXPECT_EQUAL(e2, "true");

    Expression e3 = a || false || b;
    EXPECT_EQUAL(e3, "(a || b)");

    {
        ExpectError error(r, "error: type mismatch: '||' cannot operate on 'bool', 'int'\n");
        DSLExpression(a || 5).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLComma, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kInt_Type, "b");
    Expression e1 = (a += b, b);
    EXPECT_EQUAL(e1, "((a += b) , b)");

    Expression e2 = (a += b, b += b, Int2(a));
    EXPECT_EQUAL(e2, "(((a += b) , (b += b)) , int2(a))");
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLEqual, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kInt_Type, "b");
    Expression e1 = a == b;
    EXPECT_EQUAL(e1, "(a == b)");

    Expression e2 = a == 5;
    EXPECT_EQUAL(e2, "(a == 5)");

    {
        ExpectError error(r, "error: type mismatch: '==' cannot operate on 'int', 'bool2'\n");
        DSLExpression(a == Bool2(true)).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLNotEqual, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kInt_Type, "b");
    Expression e1 = a != b;
    EXPECT_EQUAL(e1, "(a != b)");

    Expression e2 = a != 5;
    EXPECT_EQUAL(e2, "(a != 5)");

    {
        ExpectError error(r, "error: type mismatch: '!=' cannot operate on 'int', 'bool2'\n");
        DSLExpression(a != Bool2(true)).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLGreaterThan, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kInt_Type, "b");
    Expression e1 = a > b;
    EXPECT_EQUAL(e1, "(a > b)");

    Expression e2 = a > 5;
    EXPECT_EQUAL(e2, "(a > 5)");

    {
        ExpectError error(r, "error: type mismatch: '>' cannot operate on 'int', 'bool2'\n");
        DSLExpression(a > Bool2(true)).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLGreaterThanOrEqual, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kInt_Type, "b");
    Expression e1 = a >= b;
    EXPECT_EQUAL(e1, "(a >= b)");

    Expression e2 = a >= 5;
    EXPECT_EQUAL(e2, "(a >= 5)");

    {
        ExpectError error(r, "error: type mismatch: '>=' cannot operate on 'int', 'bool2'\n");
        DSLExpression(a >= Bool2(true)).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLLessThan, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kInt_Type, "b");
    Expression e1 = a < b;
    EXPECT_EQUAL(e1, "(a < b)");

    Expression e2 = a < 5;
    EXPECT_EQUAL(e2, "(a < 5)");

    {
        ExpectError error(r, "error: type mismatch: '<' cannot operate on 'int', 'bool2'\n");
        DSLExpression(a < Bool2(true)).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLLessThanOrEqual, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kInt_Type, "b");
    Expression e1 = a <= b;
    EXPECT_EQUAL(e1, "(a <= b)");

    Expression e2 = a <= 5;
    EXPECT_EQUAL(e2, "(a <= 5)");

    {
        ExpectError error(r, "error: type mismatch: '<=' cannot operate on 'int', 'bool2'\n");
        DSLExpression(a <= Bool2(true)).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLLogicalNot, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kInt_Type, "b");
    Expression e1 = !(a <= b);
    EXPECT_EQUAL(e1, "!(a <= b)");

    {
        ExpectError error(r, "error: '!' cannot operate on 'int'\n");
        DSLExpression(!a).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLBitwiseNot, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kBool_Type, "b");
    Expression e1 = ~a;
    EXPECT_EQUAL(e1, "~a");

    {
        ExpectError error(r, "error: '~' cannot operate on 'bool'\n");
        DSLExpression(~b).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLIncrement, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kBool_Type, "b");
    Expression e1 = ++a;
    EXPECT_EQUAL(e1, "++a");

    Expression e2 = a++;
    EXPECT_EQUAL(e2, "a++");

    {
        ExpectError error(r, "error: '++' cannot operate on 'bool'\n");
        DSLExpression(++b).release();
    }

    {
        ExpectError error(r, "error: '++' cannot operate on 'bool'\n");
        DSLExpression(b++).release();
    }

    {
        ExpectError error(r, "error: cannot assign to this expression\n");
        DSLExpression(++(a + 1)).release();
    }

    {
        ExpectError error(r, "error: cannot assign to this expression\n");
        DSLExpression((a + 1)++).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLDecrement, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kBool_Type, "b");
    Expression e1 = --a;
    EXPECT_EQUAL(e1, "--a");

    Expression e2 = a--;
    EXPECT_EQUAL(e2, "a--");

    {
        ExpectError error(r, "error: '--' cannot operate on 'bool'\n");
        DSLExpression(--b).release();
    }

    {
        ExpectError error(r, "error: '--' cannot operate on 'bool'\n");
        DSLExpression(b--).release();
    }

    {
        ExpectError error(r, "error: cannot assign to this expression\n");
        DSLExpression(--(a + 1)).release();
    }

    {
        ExpectError error(r, "error: cannot assign to this expression\n");
        DSLExpression((a + 1)--).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLBlock, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu(), /*markVarsDeclared=*/false);
    Statement x = Block();
    EXPECT_EQUAL(x, "{ }");
    Var a(kInt_Type, "a", 1), b(kInt_Type, "b", 2);
    Statement y = Block(Declare(a), Declare(b), a = b);
    EXPECT_EQUAL(y, "{ int a = 1; int b = 2; (a = b); }");
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLBreak, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu(), /*markVarsDeclared=*/false);
    Var i(kInt_Type, "i", 0);
    DSLFunction(kVoid_Type, "success").define(
        For(Declare(i), i < 10, ++i, Block(
            If(i > 5, Break())
        ))
    );
    REPORTER_ASSERT(r, DSLWriter::ProgramElements().size() == 1);
    EXPECT_EQUAL(*DSLWriter::ProgramElements()[0],
                 "void success() { for (int i = 0; (i < 10); ++i) { if ((i > 5)) break; } }");

    {
        ExpectError error(r, "error: break statement must be inside a loop or switch\n");
        DSLFunction(kVoid_Type, "fail").define(
            Break()
        );
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLContinue, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu(), /*markVarsDeclared=*/false);
    Var i(kInt_Type, "i", 0);
    DSLFunction(kVoid_Type, "success").define(
        For(Declare(i), i < 10, ++i, Block(
            If(i < 5, Continue())
        ))
    );
    REPORTER_ASSERT(r, DSLWriter::ProgramElements().size() == 1);
    EXPECT_EQUAL(*DSLWriter::ProgramElements()[0],
                 "void success() { for (int i = 0; (i < 10); ++i) { if ((i < 5)) continue; } }");

    {
        ExpectError error(r, "error: continue statement must be inside a loop\n");
        DSLFunction(kVoid_Type, "fail").define(
            Continue()
        );
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLDeclare, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu(), /*markVarsDeclared=*/false);
    Var a(kHalf4_Type, "a"), b(kHalf4_Type, "b", Half4(1));
    Statement x = Declare(a);
    EXPECT_EQUAL(x, "half4 a;");
    Statement y = Declare(b);
    EXPECT_EQUAL(y, "half4 b = half4(1.0);");

    {
        Var c(kHalf4_Type, "c", 1);
        ExpectError error(r, "error: expected 'half4', but found 'int'\n");
        Declare(c).release();
    }

    {
        Var d(kInt_Type, "d");
        Declare(d).release();
        ExpectError error(r, "error: variable has already been declared\n");
        Declare(d).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLDiscard, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Statement x = If(Sqrt(1) > 0, Discard());
    EXPECT_EQUAL(x, "if ((sqrt(1.0) > 0.0)) discard;");
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLDo, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Statement x = Do(Block(), true);
    EXPECT_EQUAL(x, "do {} while (true);");

    Var a(kFloat_Type, "a"), b(kFloat_Type, "b");
    Statement y = Do(Block(a++, --b), a != b);
    EXPECT_EQUAL(y, "do { a++; --b; } while ((a != b));");

    {
        ExpectError error(r, "error: expected 'bool', but found 'int'\n");
        Do(Block(), 7).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLFor, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu(), /*markVarsDeclared=*/false);
    Statement x = For(Statement(), Expression(), Expression(), Block());
    EXPECT_EQUAL(x, "for (;;) {}");

    Var i(kInt_Type, "i", 0);
    Statement y = For(Declare(i), i < 10, ++i, i += 5);
    EXPECT_EQUAL(y, "for (int i = 0; (i < 10); ++i) (i += 5);");

    {
        ExpectError error(r, "error: expected 'bool', but found 'int'\n");
        For(i = 0, i + 10, ++i, i += 5).release();
    }

    {
        ExpectError error(r, "error: invalid for loop initializer\n");
        For(If(i == 0, i = 1), i < 10, ++i, i += 5).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLFunction, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu(), /*markVarsDeclared=*/false);
    Var coords(kHalf2_Type, "coords");
    DSLFunction(kVoid_Type, "main", coords).define(
        sk_FragColor() = Half4(coords, 0, 1)
    );
    REPORTER_ASSERT(r, DSLWriter::ProgramElements().size() == 1);
    EXPECT_EQUAL(*DSLWriter::ProgramElements()[0],
                 "void main(half2 coords) { (sk_FragColor = half4(coords, 0.0, 1.0)); }");

    {
        DSLWriter::Reset();
        Var x(kFloat_Type, "x");
        DSLFunction sqr(kFloat_Type, "sqr", x);
        sqr.define(
            Return(x * x)
        );
        EXPECT_EQUAL(sqr(sk_FragCoord().x()), "sqr(sk_FragCoord.x)");
        REPORTER_ASSERT(r, DSLWriter::ProgramElements().size() == 1);
        EXPECT_EQUAL(*DSLWriter::ProgramElements()[0], "float sqr(float x) { return (x * x); }");
    }

    {
        DSLWriter::Reset();
        Var x(kFloat2_Type, "x");
        Var y(kFloat2_Type, "y");
        DSLFunction dot(kFloat2_Type, "dot", x, y);
        dot.define(
            Return(x * x + y * y)
        );
        EXPECT_EQUAL(dot(Float2(1.0f, 2.0f), Float2(3.0f, 4.0f)),
                     "dot(float2(1.0, 2.0), float2(3.0, 4.0))");
        REPORTER_ASSERT(r, DSLWriter::ProgramElements().size() == 1);
        EXPECT_EQUAL(*DSLWriter::ProgramElements()[0],
                "float2 dot(float2 x, float2 y) { return ((x * x) + (y * y)); }");
    }

    {
        DSLWriter::Reset();
        Var x(kFloat_Type, "x");
        Var y(kFloat_Type, "y");
        DSLFunction pair(kFloat2_Type, "pair", x, y);
        pair.define(
            Return(Float2(x, y))
        );
        Var varArg1(kFloat_Type, "varArg1");
        Var varArg2(kFloat_Type, "varArg2");
        DSLWriter::MarkDeclared(varArg1);
        DSLWriter::MarkDeclared(varArg2);
        EXPECT_EQUAL(pair(varArg1, varArg2), "pair(varArg1, varArg2)");
    }

    {
        ExpectError error(r, "error: expected 'float', but found 'bool'\n");
        DSLWriter::Reset();
        DSLFunction(kFloat_Type, "broken").define(
            Return(true)
        );
    }

    {
        ExpectError error(r, "error: expected function to return 'float'\n");
        DSLWriter::Reset();
        DSLFunction(kFloat_Type, "broken").define(
            Return()
        );
    }

    {
        ExpectError error(r, "error: function 'broken' can exit without returning a value\n");
        DSLWriter::Reset();
        Var x(kFloat_Type, "x", 0);
        DSLFunction(kFloat_Type, "broken").define(
            Declare(x),
            If(x == 1, Return(x))
        );
    }

    {
        ExpectError error(r, "error: may not return a value from a void function\n");
        DSLWriter::Reset();
        DSLFunction(kVoid_Type, "broken").define(
            Return(0)
        );
    }

    {
        ExpectError error(r, "error: function 'broken' can exit without returning a value\n");
        DSLWriter::Reset();
        DSLFunction(kFloat_Type, "broken").define(
        );
    }

    {
        ExpectError error(r, "error: using an already-declared variable as a function parameter\n");
        DSLWriter::Reset();
        DSLVar p(kFloat_Type);
        Declare(p).release();
        DSLFunction(kVoid_Type, "broken", p).define(
        );
    }

    {
        ExpectError error(r, "error: variable has already been declared\n");
        DSLWriter::Reset();
        DSLVar p(kFloat_Type);
        DSLFunction(kVoid_Type, "broken", p).define(
        );
        Declare(p).release();
    }

    {
        ExpectError error(r, "error: variables used as function parameters cannot have initial "
                             "values\n");
        DSLWriter::Reset();
        DSLVar p(kFloat_Type, 1);
        DSLFunction(kVoid_Type, "broken", p).define(
        );
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLIf, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kFloat_Type, "a"), b(kFloat_Type, "b");
    Statement x = If(a > b, a -= b);
    EXPECT_EQUAL(x, "if ((a > b)) (a -= b);");

    Statement y = If(a > b, a -= b, b -= a);
    EXPECT_EQUAL(y, "if ((a > b)) (a -= b); else (b -= a);");

    {
        ExpectError error(r, "error: expected 'bool', but found 'float'\n");
        If(a + b, a -= b).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLReturn, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());

    Statement x = Return();
    EXPECT_EQUAL(x, "return;");

    Statement y = Return(true);
    EXPECT_EQUAL(y, "return true;");
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLSelect, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a");
    Expression x = Select(a > 0, 1, -1);
    EXPECT_EQUAL(x, "((a > 0) ? 1 : -1)");

    {
        ExpectError error(r, "error: expected 'bool', but found 'int'\n");
        DSLExpression x = Select(a, 1, -1);
    }

    {
        ExpectError error(r, "error: ternary operator result mismatch: 'float2', 'float3'\n");
        DSLExpression x = Select(a > 0, Float2(1), Float3(1));
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLSwitch, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());

    Var a(kFloat_Type, "a"), b(kInt_Type, "b");

    Statement x = Switch(b,
        Case(0, a = 0, Break()),
        Case(1, a = 1, Continue()),
        Case(2, a = 2  /*Fallthrough*/),
        Default(Discard())
    );
    EXPECT_EQUAL(x, R"(
        switch (b) {
            case 0: (a = 0.0); break;
            case 1: (a = 1.0); continue;
            case 2: (a = 2.0);
            default: discard;
        }
    )");

    EXPECT_EQUAL(Switch(b),
                "switch (b) {}");

    EXPECT_EQUAL(Switch(b, Default(), Case(0), Case(1)),
                "switch (b) { default: case 0: case 1: }");

    {
        ExpectError error(r, "error: duplicate case value '0'\n");
        DSLStatement(Switch(0, Case(0), Case(0))).release();
    }

    {
        ExpectError error(r, "error: duplicate default case\n");
        DSLStatement(Switch(0, Default(a = 0), Default(a = 1))).release();
    }

    {
        ExpectError error(r, "error: case value must be a constant integer\n");
        Var b(kInt_Type);
        DSLStatement(Switch(0, Case(b))).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLSwizzle, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kFloat4_Type, "a");

    EXPECT_EQUAL(a.x(),
                "a.x");
    EXPECT_EQUAL(a.y(),
                "a.y");
    EXPECT_EQUAL(a.z(),
                "a.z");
    EXPECT_EQUAL(a.w(),
                "a.w");
    EXPECT_EQUAL(a.r(),
                "a.x");
    EXPECT_EQUAL(a.g(),
                "a.y");
    EXPECT_EQUAL(a.b(),
                "a.z");
    EXPECT_EQUAL(a.a(),
                "a.w");
    EXPECT_EQUAL(Swizzle(a, R),
                "a.x");
    EXPECT_EQUAL(Swizzle(a, ZERO, G),
                "float2(0.0, a.y)");
    EXPECT_EQUAL(Swizzle(a, B, G, G),
                "a.zyy");
    EXPECT_EQUAL(Swizzle(a, R, G, B, ONE),
                "float4(a.xyz, 1.0)");
    EXPECT_EQUAL(Swizzle(a, B, G, R, ONE).r(),
                "a.z");
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLWhile, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Statement x = While(true, Block());
    EXPECT_EQUAL(x, "for (; true;) {}");

    Var a(kFloat_Type, "a"), b(kFloat_Type, "b");
    Statement y = While(a != b, Block(a++, --b));
    EXPECT_EQUAL(y, "for (; (a != b);) { a++; --b; }");

    {
        ExpectError error(r, "error: expected 'bool', but found 'int'\n");
        DSLStatement x = While(7, Block());
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLIndex, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(Array(kInt_Type, 5), "a"), b(kInt_Type, "b");

    EXPECT_EQUAL(a[0], "a[0]");
    EXPECT_EQUAL(a[b], "a[b]");

    {
        ExpectError error(r, "error: expected 'int', but found 'bool'\n");
        DSLExpression x = a[true];
    }

    {
        ExpectError error(r, "error: expected array, but found 'int'\n");
        DSLExpression x = b[0];
    }

    {
        ExpectError error(r, "error: index -1 out of range for 'int[5]'\n");
        DSLExpression x = a[-1];
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLBuiltins, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    // There is a Fract type on Mac which can conflict with our Fract builtin
    using SkSL::dsl::Fract;
    Var a(kHalf4_Type, "a"), b(kHalf4_Type, "b"), c(kHalf4_Type, "c");
    Var h3(kHalf3_Type, "h3");
    Var b4(kBool4_Type, "b4");
    EXPECT_EQUAL(Abs(a),                 "abs(a)");
    EXPECT_EQUAL(All(b4),                "all(b4)");
    EXPECT_EQUAL(Any(b4),                "any(b4)");
    EXPECT_EQUAL(Ceil(a),                "ceil(a)");
    EXPECT_EQUAL(Clamp(a, 0, 1),         "clamp(a, 0.0, 1.0)");
    EXPECT_EQUAL(Cos(a),                 "cos(a)");
    EXPECT_EQUAL(Cross(h3, h3),          "cross(h3, h3)");
    EXPECT_EQUAL(Degrees(a),             "degrees(a)");
    EXPECT_EQUAL(Distance(a, b),         "distance(a, b)");
    EXPECT_EQUAL(Dot(a, b),              "dot(a, b)");
    EXPECT_EQUAL(Equal(a, b),            "equal(a, b)");
    EXPECT_EQUAL(Exp(a),                 "exp(a)");
    EXPECT_EQUAL(Exp2(a),                "exp2(a)");
    EXPECT_EQUAL(Faceforward(a, b, c),   "faceforward(a, b, c)");
    EXPECT_EQUAL(Floor(a),               "floor(a)");
    EXPECT_EQUAL(Fract(a),               "fract(a)");
    EXPECT_EQUAL(GreaterThan(a, b),      "greaterThan(a, b)");
    EXPECT_EQUAL(GreaterThanEqual(a, b), "greaterThanEqual(a, b)");
    EXPECT_EQUAL(Inversesqrt(a),         "inversesqrt(a)");
    EXPECT_EQUAL(LessThan(a, b),         "lessThan(a, b)");
    EXPECT_EQUAL(LessThanEqual(a, b),    "lessThanEqual(a, b)");
    EXPECT_EQUAL(Length(a),              "length(a)");
    EXPECT_EQUAL(Log(a),                 "log(a)");
    EXPECT_EQUAL(Log2(a),                "log2(a)");
    EXPECT_EQUAL(Max(a, b),              "max(a, b)");
    EXPECT_EQUAL(Min(a, b),              "min(a, b)");
    EXPECT_EQUAL(Mix(a, b, c),           "mix(a, b, c)");
    EXPECT_EQUAL(Mod(a, b),              "mod(a, b)");
    EXPECT_EQUAL(Normalize(a),           "normalize(a)");
    EXPECT_EQUAL(NotEqual(a, b),         "notEqual(a, b)");
    EXPECT_EQUAL(Pow(a, b),              "pow(a, b)");
    EXPECT_EQUAL(Radians(a),             "radians(a)");
    EXPECT_EQUAL(Reflect(a, b),          "reflect(a, b)");
    EXPECT_EQUAL(Refract(a, b, 1),       "refract(a, b, 1.0)");
    EXPECT_EQUAL(Saturate(a),            "saturate(a)");
    EXPECT_EQUAL(Sign(a),                "sign(a)");
    EXPECT_EQUAL(Sin(a),                 "sin(a)");
    EXPECT_EQUAL(Smoothstep(a, b, c),    "smoothstep(a, b, c)");
    EXPECT_EQUAL(Sqrt(a),                "sqrt(a)");
    EXPECT_EQUAL(Step(a, b),             "step(a, b)");
    EXPECT_EQUAL(Tan(a),                 "tan(a)");
    EXPECT_EQUAL(Unpremul(a),            "unpremul(a)");

    // these calls all go through the normal channels, so it ought to be sufficient to prove that
    // one of them reports errors correctly
    {
        ExpectError error(r, "error: no match for ceil(bool)\n");
        Ceil(a == b).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLModifiers, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu(), /*markVarsDeclared=*/false);

    Var v1(kConst_Modifier, kInt_Type, "v1", 0);
    Statement d1 = Declare(v1);
    EXPECT_EQUAL(d1, "const int v1 = 0;");

    // Most modifiers require an appropriate context to be legal. We can't yet give them that
    // context, so we can't as yet Declare() variables with these modifiers.
    // TODO: better tests when able
    Var v2(kIn_Modifier, kInt_Type, "v2");
    REPORTER_ASSERT(r, DSLWriter::Var(v2).modifiers().fFlags == SkSL::Modifiers::kIn_Flag);
    DSLWriter::MarkDeclared(v2);

    Var v3(kOut_Modifier, kInt_Type, "v3");
    REPORTER_ASSERT(r, DSLWriter::Var(v3).modifiers().fFlags == SkSL::Modifiers::kOut_Flag);
    DSLWriter::MarkDeclared(v3);

    Var v4(kFlat_Modifier, kInt_Type, "v4");
    REPORTER_ASSERT(r, DSLWriter::Var(v4).modifiers().fFlags == SkSL::Modifiers::kFlat_Flag);
    DSLWriter::MarkDeclared(v4);

    Var v5(kNoPerspective_Modifier, kInt_Type, "v5");
    REPORTER_ASSERT(r, DSLWriter::Var(v5).modifiers().fFlags ==
                       SkSL::Modifiers::kNoPerspective_Flag);
    DSLWriter::MarkDeclared(v5);

    Var v6(kIn_Modifier | kOut_Modifier, kInt_Type, "v6");
    REPORTER_ASSERT(r, DSLWriter::Var(v6).modifiers().fFlags ==
                       (SkSL::Modifiers::kIn_Flag | SkSL::Modifiers::kOut_Flag));
    DSLWriter::MarkDeclared(v6);

    Var v7(kInOut_Modifier, kInt_Type, "v7");
    REPORTER_ASSERT(r, DSLWriter::Var(v7).modifiers().fFlags ==
                       (SkSL::Modifiers::kIn_Flag | SkSL::Modifiers::kOut_Flag));
    DSLWriter::MarkDeclared(v7);

    Var v8(kUniform_Modifier, kInt_Type, "v8");
    REPORTER_ASSERT(r, DSLWriter::Var(v8).modifiers().fFlags == SkSL::Modifiers::kUniform_Flag);
    // Uniforms do not need to be explicitly declared
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLSampleFragmentProcessor, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu(), /*markVarsDeclared=*/true,
                           SkSL::ProgramKind::kFragmentProcessor);
    DSLVar child(kUniform_Modifier, kFragmentProcessor_Type, "child");
    EXPECT_EQUAL(Sample(child), "sample(child)");
    EXPECT_EQUAL(Sample(child, Float2(0, 0)), "sample(child, float2(0.0, 0.0))");
    EXPECT_EQUAL(Sample(child, Float3x3(1.0)), "sample(child, float3x3(1.0))");
    EXPECT_EQUAL(Sample(child, Half4(1)), "sample(child, half4(1.0))");
    EXPECT_EQUAL(Sample(child, Float2(0), Half4(1)), "sample(child, float2(0.0), half4(1.0))");
    EXPECT_EQUAL(Sample(child, Float3x3(1.0), Half4(1)),
                 "sample(child, float3x3(1.0), half4(1.0))");

    {
        ExpectError error(r, "error: no match for sample(fragmentProcessor, bool)\n");
        Sample(child, true).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLSampleShader, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu(), /*markVarsDeclared=*/true,
                           SkSL::ProgramKind::kRuntimeEffect);
    DSLVar shader(kUniform_Modifier, kShader_Type, "shader");
    EXPECT_EQUAL(Sample(shader), "sample(shader)");
    EXPECT_EQUAL(Sample(shader, Float2(0, 0)), "sample(shader, float2(0.0, 0.0))");
    EXPECT_EQUAL(Sample(shader, Float3x3(1)), "sample(shader, float3x3(1.0))");

    {
        ExpectError error(r, "error: no match for sample(shader, half4)\n");
        Sample(shader, Half4(1)).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLStruct, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu(), /*markVarsDeclared=*/false);

    DSLType simpleStruct = Struct("SimpleStruct",
        Field(kFloat_Type, "x"),
        Field(kBool_Type, "b"),
        Field(Array(kFloat_Type, 3), "a")
    );
    DSLVar result(simpleStruct, "result");
    DSLFunction(simpleStruct, "returnStruct").define(
        Declare(result),
        result.field("x") = 123,
        result.field("b") = result.field("x") > 0,
        result.field("a")[0] = result.field("x"),
        Return(result)
    );
    REPORTER_ASSERT(r, DSLWriter::ProgramElements().size() == 2);
    EXPECT_EQUAL(*DSLWriter::ProgramElements()[0],
                 "struct SimpleStruct { float x; bool b; float[3] a; };");
    EXPECT_EQUAL(*DSLWriter::ProgramElements()[1],
                 "SimpleStruct returnStruct() { SimpleStruct result; (result.x = 123.0);"
                 "(result.b = (result.x > 0.0)); (result.a[0] = result.x); return result; }");

    Struct("NestedStruct",
        Field(kInt_Type, "x"),
        Field(simpleStruct, "simple")
    );
    REPORTER_ASSERT(r, DSLWriter::ProgramElements().size() == 3);
    EXPECT_EQUAL(*DSLWriter::ProgramElements()[2],
                 "struct NestedStruct { int x; SimpleStruct simple; };");
}
