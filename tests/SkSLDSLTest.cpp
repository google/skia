/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/dsl/DSL.h"
#include "src/sksl/dsl/priv/DSLWriter.h"

#include "tests/Test.h"

#include <limits>

using namespace SkSL::dsl;

class AutoDSLContext {
public:
    AutoDSLContext(GrGpu* gpu) {
        Start(gpu->shaderCompiler());
        DSLWriter::Instance().fMangle = false;
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
        REPORTER_ASSERT(fReporter, !fMsg);
        SetErrorHandler(nullptr);
    }

    void handleError(const char* msg) override {
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

static bool whitespace_insensitive_compare(DSLStatement& stmt, const char* description) {
    return whitespace_insensitive_compare(stmt.release()->description().c_str(), description);
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLStartup, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Expression e1 = 1;
    REPORTER_ASSERT(r, e1.release()->description() == "1");
    Expression e2 = 1.0;
    REPORTER_ASSERT(r, e2.release()->description() == "1.0");
    Expression e3 = true;
    REPORTER_ASSERT(r, e3.release()->description() == "true");
    Var a(kInt, "a");
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

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLFloat, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Expression e1 = Float(std::numeric_limits<float>::max());
    REPORTER_ASSERT(r, atof(e1.release()->description().c_str()) ==
                       std::numeric_limits<float>::max());

    Expression e2 = Float(std::numeric_limits<float>::min());
    REPORTER_ASSERT(r, atof(e2.release()->description().c_str()) ==
                       std::numeric_limits<float>::min());

    Expression e3 = Float2(0);
    REPORTER_ASSERT(r, e3.release()->description() == "float2(0.0)");

    Expression e4 = Float2(-0.5, 1);
    REPORTER_ASSERT(r, e4.release()->description() == "float2(-0.5, 1.0)");

    Expression e5 = Float3(0.75);
    REPORTER_ASSERT(r, e5.release()->description() == "float3(0.75)");

    Expression e6 = Float3(Float2(0, 1), -2);
    REPORTER_ASSERT(r, e6.release()->description() == "float3(float2(0.0, 1.0), -2.0)");

    Expression e7 = Float3(0, 1, 2);
    REPORTER_ASSERT(r, e7.release()->description() == "float3(0.0, 1.0, 2.0)");

    Expression e8 = Float4(0);
    REPORTER_ASSERT(r, e8.release()->description() == "float4(0.0)");

    Expression e9 = Float4(Float2(0, 1), Float2(2, 3));
    REPORTER_ASSERT(r, e9.release()->description() == "float4(float2(0.0, 1.0), float2(2.0, 3.0))");

    Expression e10 = Float4(0, 1, Float2(2, 3));
    REPORTER_ASSERT(r, e10.release()->description() == "float4(0.0, 1.0, float2(2.0, 3.0))");

    Expression e11 = Float4(0, 1, 2, 3);
    REPORTER_ASSERT(r, e11.release()->description() == "float4(0.0, 1.0, 2.0, 3.0)");

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
    REPORTER_ASSERT(r, atof(e1.release()->description().c_str()) ==
                       std::numeric_limits<float>::max());

    Expression e2 = Half(std::numeric_limits<float>::min());
    REPORTER_ASSERT(r, atof(e2.release()->description().c_str()) ==
                       std::numeric_limits<float>::min());

    Expression e3 = Half2(0);
    REPORTER_ASSERT(r, e3.release()->description() == "half2(0.0)");

    Expression e4 = Half2(-0.5, 1);
    REPORTER_ASSERT(r, e4.release()->description() == "half2(-0.5, 1.0)");

    Expression e5 = Half3(0.75);
    REPORTER_ASSERT(r, e5.release()->description() == "half3(0.75)");

    Expression e6 = Half3(Half2(0, 1), -2);
    REPORTER_ASSERT(r, e6.release()->description() == "half3(half2(0.0, 1.0), -2.0)");

    Expression e7 = Half3(0, 1, 2);
    REPORTER_ASSERT(r, e7.release()->description() == "half3(0.0, 1.0, 2.0)");

    Expression e8 = Half4(0);
    REPORTER_ASSERT(r, e8.release()->description() == "half4(0.0)");

    Expression e9 = Half4(Half2(0, 1), Half2(2, 3));
    REPORTER_ASSERT(r, e9.release()->description() == "half4(half2(0.0, 1.0), half2(2.0, 3.0))");

    Expression e10 = Half4(0, 1, Half2(2, 3));
    REPORTER_ASSERT(r, e10.release()->description() == "half4(0.0, 1.0, half2(2.0, 3.0))");

    Expression e11 = Half4(0, 1, 2, 3);
    REPORTER_ASSERT(r, e11.release()->description() == "half4(0.0, 1.0, 2.0, 3.0)");

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
    Expression e1 = Int(std::numeric_limits<int32_t>::max());
    REPORTER_ASSERT(r, e1.release()->description() == "2147483647");

    Expression e2 = Int2(std::numeric_limits<int32_t>::min());
    REPORTER_ASSERT(r, e2.release()->description() == "int2(-2147483648)");

    Expression e3 = Int2(0, 1);
    REPORTER_ASSERT(r, e3.release()->description() == "int2(0, 1)");

    Expression e4 = Int3(0);
    REPORTER_ASSERT(r, e4.release()->description() == "int3(0)");

    Expression e5 = Int3(Int2(0, 1), -2);
    REPORTER_ASSERT(r, e5.release()->description() == "int3(int2(0, 1), -2)");

    Expression e6 = Int3(0, 1, 2);
    REPORTER_ASSERT(r, e6.release()->description() == "int3(0, 1, 2)");

    Expression e7 = Int4(0);
    REPORTER_ASSERT(r, e7.release()->description() == "int4(0)");

    Expression e8 = Int4(Int2(0, 1), Int2(2, 3));
    REPORTER_ASSERT(r, e8.release()->description() == "int4(int2(0, 1), int2(2, 3))");

    Expression e9 = Int4(0, 1, Int2(2, 3));
    REPORTER_ASSERT(r, e9.release()->description() == "int4(0, 1, int2(2, 3))");

    Expression e10 = Int4(0, 1, 2, 3);
    REPORTER_ASSERT(r, e10.release()->description() == "int4(0, 1, 2, 3)");

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
    Expression e1 = Short(std::numeric_limits<int16_t>::max());
    REPORTER_ASSERT(r, e1.release()->description() == "32767");

    Expression e2 = Short2(std::numeric_limits<int16_t>::min());
    REPORTER_ASSERT(r, e2.release()->description() == "short2(-32768)");

    Expression e3 = Short2(0, 1);
    REPORTER_ASSERT(r, e3.release()->description() == "short2(0, 1)");

    Expression e4 = Short3(0);
    REPORTER_ASSERT(r, e4.release()->description() == "short3(0)");

    Expression e5 = Short3(Short2(0, 1), -2);
    REPORTER_ASSERT(r, e5.release()->description() == "short3(short2(0, 1), -2)");

    Expression e6 = Short3(0, 1, 2);
    REPORTER_ASSERT(r, e6.release()->description() == "short3(0, 1, 2)");

    Expression e7 = Short4(0);
    REPORTER_ASSERT(r, e7.release()->description() == "short4(0)");

    Expression e8 = Short4(Short2(0, 1), Short2(2, 3));
    REPORTER_ASSERT(r, e8.release()->description() == "short4(short2(0, 1), short2(2, 3))");

    Expression e9 = Short4(0, 1, Short2(2, 3));
    REPORTER_ASSERT(r, e9.release()->description() == "short4(0, 1, short2(2, 3))");

    Expression e10 = Short4(0, 1, 2, 3);
    REPORTER_ASSERT(r, e10.release()->description() == "short4(0, 1, 2, 3)");

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
    Expression e1 = Bool2(false);
    REPORTER_ASSERT(r, e1.release()->description() == "bool2(false)");

    Expression e2 = Bool2(false, true);
    REPORTER_ASSERT(r, e2.release()->description() == "bool2(false, true)");

    Expression e3 = Bool3(false);
    REPORTER_ASSERT(r, e3.release()->description() == "bool3(false)");

    Expression e4 = Bool3(Bool2(false, true), false);
    REPORTER_ASSERT(r, e4.release()->description() == "bool3(bool2(false, true), false)");

    Expression e5 = Bool3(false, true, false);
    REPORTER_ASSERT(r, e5.release()->description() == "bool3(false, true, false)");

    Expression e6 = Bool4(false);
    REPORTER_ASSERT(r, e6.release()->description() == "bool4(false)");

    Expression e7 = Bool4(Bool2(false, true), Bool2(false, true));
    REPORTER_ASSERT(r, e7.release()->description() == "bool4(bool2(false, true), "
                                                      "bool2(false, true))");

    Expression e8 = Bool4(false, true, Bool2(false, true));
    REPORTER_ASSERT(r, e8.release()->description() == "bool4(false, true, bool2(false, true))");

    Expression e9 = Bool4(false, true, false, true);
    REPORTER_ASSERT(r, e9.release()->description() == "bool4(false, true, false, true)");

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

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLPlus, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kFloat, "a"), b(kFloat, "b");
    Expression e1 = a + b;
    REPORTER_ASSERT(r, e1.release()->description() == "(a + b)");

    Expression e2 = a + 1;
    REPORTER_ASSERT(r, e2.release()->description() == "(a + 1.0)");

    Expression e3 = 0.5 + a + -99;
    REPORTER_ASSERT(r, e3.release()->description() == "((0.5 + a) + -99.0)");

    Expression e4 = a += b + 1;
    REPORTER_ASSERT(r, e4.release()->description() == "(a += (b + 1.0))");

    {
        ExpectError error(r, "error: type mismatch: '+' cannot operate on 'bool2', 'float'\n");
        (Bool2(true) + a).release();
    }

    {
        ExpectError error(r, "error: type mismatch: '+=' cannot operate on 'float', 'bool2'\n");
        (a += Bool2(true)).release();
    }

    {
        ExpectError error(r, "error: cannot assign to this expression\n");
        (1.0 += a).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLMinus, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt, "a"), b(kInt, "b");
    Expression e1 = a - b;
    REPORTER_ASSERT(r, e1.release()->description() == "(a - b)");

    Expression e2 = a - 1;
    REPORTER_ASSERT(r, e2.release()->description() == "(a - 1)");

    Expression e3 = 2 - a - b;
    REPORTER_ASSERT(r, e3.release()->description() == "((2 - a) - b)");

    Expression e4 = a -= b + 1;
    REPORTER_ASSERT(r, e4.release()->description() == "(a -= (b + 1))");

    {
        ExpectError error(r, "error: type mismatch: '-' cannot operate on 'bool2', 'int'\n");
        (Bool2(true) - a).release();
    }

    {
        ExpectError error(r, "error: type mismatch: '-=' cannot operate on 'int', 'bool2'\n");
        (a -= Bool2(true)).release();
    }

    {
        ExpectError error(r, "error: cannot assign to this expression\n");
        (1.0 -= a).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLMultiply, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kFloat, "a"), b(kFloat, "b");
    Expression e1 = a * b;
    REPORTER_ASSERT(r, e1.release()->description() == "(a * b)");

    Expression e2 = a * 1;
    REPORTER_ASSERT(r, e2.release()->description() == "(a * 1.0)");

    Expression e3 = 0.5 * a * -99;
    REPORTER_ASSERT(r, e3.release()->description() == "((0.5 * a) * -99.0)");

    Expression e4 = a *= b + 1;
    REPORTER_ASSERT(r, e4.release()->description() == "(a *= (b + 1.0))");

    {
        ExpectError error(r, "error: type mismatch: '*' cannot operate on 'bool2', 'float'\n");
        (Bool2(true) * a).release();
    }

    {
        ExpectError error(r, "error: type mismatch: '*=' cannot operate on 'float', 'bool2'\n");
        (a *= Bool2(true)).release();
    }

    {
        ExpectError error(r, "error: cannot assign to this expression\n");
        (1.0 *= a).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLDivide, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kFloat, "a"), b(kFloat, "b");
    Expression e1 = a / b;
    REPORTER_ASSERT(r, e1.release()->description() == "(a / b)");

    Expression e2 = a / 1;
    REPORTER_ASSERT(r, e2.release()->description() == "(a / 1.0)");

    Expression e3 = 0.5 / a / -99;
    REPORTER_ASSERT(r, e3.release()->description() == "((0.5 / a) / -99.0)");

    Expression e4 = b / (a - 1);
    REPORTER_ASSERT(r, e4.release()->description() == "(b / (a - 1.0))");

    Expression e5 = a /= b + 1;
    REPORTER_ASSERT(r, e5.release()->description() == "(a /= (b + 1.0))");

    {
        ExpectError error(r, "error: type mismatch: '/' cannot operate on 'bool2', 'float'\n");
        (Bool2(true) / a).release();
    }

    {
        ExpectError error(r, "error: type mismatch: '/=' cannot operate on 'float', 'bool2'\n");
        (a /= Bool2(true)).release();
    }

    {
        ExpectError error(r, "error: cannot assign to this expression\n");
        (1.0 /= a).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLMod, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt, "a"), b(kInt, "b");
    Expression e1 = a % b;
    REPORTER_ASSERT(r, e1.release()->description() == "(a % b)");

    Expression e2 = a % 2;
    REPORTER_ASSERT(r, e2.release()->description() == "(a % 2)");

    Expression e3 = 10 % a % -99;
    REPORTER_ASSERT(r, e3.release()->description() == "((10 % a) % -99)");

    Expression e4 = a %= b + 1;
    REPORTER_ASSERT(r, e4.release()->description() == "(a %= (b + 1))");

    {
        ExpectError error(r, "error: type mismatch: '%' cannot operate on 'bool2', 'int'\n");
        (Bool2(true) % a).release();
    }

    {
        ExpectError error(r, "error: type mismatch: '%=' cannot operate on 'int', 'bool2'\n");
        (a %= Bool2(true)).release();
    }

    {
        ExpectError error(r, "error: cannot assign to this expression\n");
        (1 %= a).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLShl, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt, "a"), b(kInt, "b");
    Expression e1 = a << b;
    REPORTER_ASSERT(r, e1.release()->description() == "(a << b)");

    Expression e2 = a << 1;
    REPORTER_ASSERT(r, e2.release()->description() == "(a << 1)");

    Expression e3 = 1 << a << 2;
    REPORTER_ASSERT(r, e3.release()->description() == "((1 << a) << 2)");

    Expression e4 = a <<= b + 1;
    REPORTER_ASSERT(r, e4.release()->description() == "(a <<= (b + 1))");

    {
        ExpectError error(r, "error: type mismatch: '<<' cannot operate on 'bool2', 'int'\n");
        (Bool2(true) << a).release();
    }

    {
        ExpectError error(r, "error: type mismatch: '<<=' cannot operate on 'int', 'bool2'\n");
        (a <<= Bool2(true)).release();
    }

    {
        ExpectError error(r, "error: cannot assign to this expression\n");
        (1 <<= a).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLShr, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt, "a"), b(kInt, "b");
    Expression e1 = a >> b;
    REPORTER_ASSERT(r, e1.release()->description() == "(a >> b)");

    Expression e2 = a >> 1;
    REPORTER_ASSERT(r, e2.release()->description() == "(a >> 1)");

    Expression e3 = 1 >> a >> 2;
    REPORTER_ASSERT(r, e3.release()->description() == "((1 >> a) >> 2)");

    Expression e4 = a >>= b + 1;
    REPORTER_ASSERT(r, e4.release()->description() == "(a >>= (b + 1))");

    {
        ExpectError error(r, "error: type mismatch: '>>' cannot operate on 'bool2', 'int'\n");
        (Bool2(true) >> a).release();
    }

    {
        ExpectError error(r, "error: type mismatch: '>>=' cannot operate on 'int', 'bool2'\n");
        (a >>= Bool2(true)).release();
    }

    {
        ExpectError error(r, "error: cannot assign to this expression\n");
        (1 >>= a).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLBitwiseAnd, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt, "a"), b(kInt, "b");
    Expression e1 = a & b;
    REPORTER_ASSERT(r, e1.release()->description() == "(a & b)");

    Expression e2 = a & 1;
    REPORTER_ASSERT(r, e2.release()->description() == "(a & 1)");

    Expression e3 = 1 & a & 2;
    REPORTER_ASSERT(r, e3.release()->description() == "((1 & a) & 2)");

    Expression e4 = a &= b + 1;
    REPORTER_ASSERT(r, e4.release()->description() == "(a &= (b + 1))");

    {
        ExpectError error(r, "error: type mismatch: '&' cannot operate on 'bool2', 'int'\n");
        (Bool2(true) & a).release();
    }

    {
        ExpectError error(r, "error: type mismatch: '&=' cannot operate on 'int', 'bool2'\n");
        (a &= Bool2(true)).release();
    }

    {
        ExpectError error(r, "error: cannot assign to this expression\n");
        (1 &= a).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLBitwiseOr, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt, "a"), b(kInt, "b");
    Expression e1 = a | b;
    REPORTER_ASSERT(r, e1.release()->description() == "(a | b)");

    Expression e2 = a | 1;
    REPORTER_ASSERT(r, e2.release()->description() == "(a | 1)");

    Expression e3 = 1 | a | 2;
    REPORTER_ASSERT(r, e3.release()->description() == "((1 | a) | 2)");

    Expression e4 = a |= b + 1;
    REPORTER_ASSERT(r, e4.release()->description() == "(a |= (b + 1))");

    {
        ExpectError error(r, "error: type mismatch: '|' cannot operate on 'bool2', 'int'\n");
        (Bool2(true) | a).release();
    }

    {
        ExpectError error(r, "error: type mismatch: '|=' cannot operate on 'int', 'bool2'\n");
        (a |= Bool2(true)).release();
    }

    {
        ExpectError error(r, "error: cannot assign to this expression\n");
        (1 |= a).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLBitwiseXor, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt, "a"), b(kInt, "b");
    Expression e1 = a ^ b;
    REPORTER_ASSERT(r, e1.release()->description() == "(a ^ b)");

    Expression e2 = a ^ 1;
    REPORTER_ASSERT(r, e2.release()->description() == "(a ^ 1)");

    Expression e3 = 1 ^ a ^ 2;
    REPORTER_ASSERT(r, e3.release()->description() == "((1 ^ a) ^ 2)");

    Expression e4 = a ^= b + 1;
    REPORTER_ASSERT(r, e4.release()->description() == "(a ^= (b + 1))");

    {
        ExpectError error(r, "error: type mismatch: '^' cannot operate on 'bool2', 'int'\n");
        (Bool2(true) ^ a).release();
    }

    {
        ExpectError error(r, "error: type mismatch: '^=' cannot operate on 'int', 'bool2'\n");
        (a ^= Bool2(true)).release();
    }

    {
        ExpectError error(r, "error: cannot assign to this expression\n");
        (1 ^= a).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLLogicalAnd, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kBool, "a"), b(kBool, "b");
    Expression e1 = a && b;
    REPORTER_ASSERT(r, e1.release()->description() == "(a && b)");

    Expression e2 = a && true && b;
    REPORTER_ASSERT(r, e2.release()->description() == "(a && b)");

    Expression e3 = a && false && b;
    REPORTER_ASSERT(r, e3.release()->description() == "false");

    {
        ExpectError error(r, "error: type mismatch: '&&' cannot operate on 'bool', 'int'\n");
        (a && 5).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLLogicalOr, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kBool, "a"), b(kBool, "b");
    Expression e1 = a || b;
    REPORTER_ASSERT(r, e1.release()->description() == "(a || b)");

    Expression e2 = a || true || b;
    REPORTER_ASSERT(r, e2.release()->description() == "true");

    Expression e3 = a || false || b;
    REPORTER_ASSERT(r, e3.release()->description() == "(a || b)");

    {
        ExpectError error(r, "error: type mismatch: '||' cannot operate on 'bool', 'int'\n");
        (a || 5).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLComma, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt, "a"), b(kInt, "b");
    Expression e1 = (a += b, b);
    REPORTER_ASSERT(r, e1.release()->description() == "((a += b) , b)");

    Expression e2 = (a += b, b += b, Int2(a));
    REPORTER_ASSERT(r, e2.release()->description() == "(((a += b) , (b += b)) , int2(a))");
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLEqual, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt, "a"), b(kInt, "b");
    Expression e1 = a == b;
    REPORTER_ASSERT(r, e1.release()->description() == "(a == b)");

    Expression e2 = a == 5;
    REPORTER_ASSERT(r, e2.release()->description() == "(a == 5)");

    {
        ExpectError error(r, "error: type mismatch: '==' cannot operate on 'int', 'bool2'\n");
        (a == Bool2(true)).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLNotEqual, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt, "a"), b(kInt, "b");
    Expression e1 = a != b;
    REPORTER_ASSERT(r, e1.release()->description() == "(a != b)");

    Expression e2 = a != 5;
    REPORTER_ASSERT(r, e2.release()->description() == "(a != 5)");

    {
        ExpectError error(r, "error: type mismatch: '!=' cannot operate on 'int', 'bool2'\n");
        (a != Bool2(true)).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLGreaterThan, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt, "a"), b(kInt, "b");
    Expression e1 = a > b;
    REPORTER_ASSERT(r, e1.release()->description() == "(a > b)");

    Expression e2 = a > 5;
    REPORTER_ASSERT(r, e2.release()->description() == "(a > 5)");

    {
        ExpectError error(r, "error: type mismatch: '>' cannot operate on 'int', 'bool2'\n");
        (a > Bool2(true)).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLGreaterThanOrEqual, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt, "a"), b(kInt, "b");
    Expression e1 = a >= b;
    REPORTER_ASSERT(r, e1.release()->description() == "(a >= b)");

    Expression e2 = a >= 5;
    REPORTER_ASSERT(r, e2.release()->description() == "(a >= 5)");

    {
        ExpectError error(r, "error: type mismatch: '>=' cannot operate on 'int', 'bool2'\n");
        (a >= Bool2(true)).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLLessThan, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt, "a"), b(kInt, "b");
    Expression e1 = a < b;
    REPORTER_ASSERT(r, e1.release()->description() == "(a < b)");

    Expression e2 = a < 5;
    REPORTER_ASSERT(r, e2.release()->description() == "(a < 5)");

    {
        ExpectError error(r, "error: type mismatch: '<' cannot operate on 'int', 'bool2'\n");
        (a < Bool2(true)).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLLessThanOrEqual, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt, "a"), b(kInt, "b");
    Expression e1 = a <= b;
    REPORTER_ASSERT(r, e1.release()->description() == "(a <= b)");

    Expression e2 = a <= 5;
    REPORTER_ASSERT(r, e2.release()->description() == "(a <= 5)");

    {
        ExpectError error(r, "error: type mismatch: '<=' cannot operate on 'int', 'bool2'\n");
        (a <= Bool2(true)).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLLogicalNot, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt, "a"), b(kInt, "b");
    Expression e1 = !(a <= b);
    REPORTER_ASSERT(r, e1.release()->description() == "!(a <= b)");

    {
        ExpectError error(r, "error: '!' cannot operate on 'int'\n");
        (!a).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLBitwiseNot, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt, "a"), b(kBool, "b");
    Expression e1 = ~a;
    REPORTER_ASSERT(r, e1.release()->description() == "~a");

    {
        ExpectError error(r, "error: '~' cannot operate on 'bool'\n");
        (~b).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLIncrement, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt, "a"), b(kBool, "b");
    Expression e1 = ++a;
    REPORTER_ASSERT(r, e1.release()->description() == "++a");

    Expression e2 = a++;
    REPORTER_ASSERT(r, e2.release()->description() == "a++");

    {
        ExpectError error(r, "error: '++' cannot operate on 'bool'\n");
        (++b).release();
    }

    {
        ExpectError error(r, "error: '++' cannot operate on 'bool'\n");
        (b++).release();
    }

    {
        ExpectError error(r, "error: cannot assign to this expression\n");
        (++(a + 1)).release();
    }

    {
        ExpectError error(r, "error: cannot assign to this expression\n");
        ((a + 1)++).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLDecrement, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt, "a"), b(kBool, "b");
    Expression e1 = --a;
    REPORTER_ASSERT(r, e1.release()->description() == "--a");

    Expression e2 = a--;
    REPORTER_ASSERT(r, e2.release()->description() == "a--");

    {
        ExpectError error(r, "error: '--' cannot operate on 'bool'\n");
        (--b).release();
    }

    {
        ExpectError error(r, "error: '--' cannot operate on 'bool'\n");
        (b--).release();
    }

    {
        ExpectError error(r, "error: cannot assign to this expression\n");
        (--(a + 1)).release();
    }

    {
        ExpectError error(r, "error: cannot assign to this expression\n");
        ((a + 1)--).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLBlock, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Statement x = Block();
    REPORTER_ASSERT(r, whitespace_insensitive_compare(x, "{ }"));
    Var a(kInt, "a"), b(kInt, "b");
    Statement y = Block(Declare(a, 1), Declare(b, 2), a = b);
    REPORTER_ASSERT(r, whitespace_insensitive_compare(y, "{ int a = 1; int b = 2; (a = b); }"));
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLDeclare, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kHalf4, "a"), b(kHalf4, "b");
    Statement x = Declare(a);
    REPORTER_ASSERT(r, x.release()->description() == "half4 a;");
    Statement y = Declare(b, Half4(1));
    REPORTER_ASSERT(r, y.release()->description() == "half4 b = half4(1.0);");

    {
        Var c(kHalf4, "c");
        ExpectError error(r, "error: expected 'half4', but found 'int'\n");
        Declare(c, 1).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLDo, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Statement x = Do(Block(), true);
    REPORTER_ASSERT(r, whitespace_insensitive_compare(x, "do {} while (true);"));

    Var a(kFloat, "a"), b(kFloat, "b");
    Statement y = Do(Block(a++, --b), a != b);
    REPORTER_ASSERT(r, whitespace_insensitive_compare(y, "do { a++; --b; } while ((a != b));"));

    {
        ExpectError error(r, "error: expected 'bool', but found 'int'\n");
        Do(Block(), 7).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLFor, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Statement x = For(Statement(), Expression(), Expression(), Block());
    REPORTER_ASSERT(r, whitespace_insensitive_compare(x, "for (;;) {}"));

    Var i(kInt, "i");
    Statement y = For(Declare(i, 0), i < 10, ++i, i += 5);
    REPORTER_ASSERT(r, whitespace_insensitive_compare(y,
                                                      "for (int i = 0; (i < 10); ++i) (i += 5);"));

    {
        ExpectError error(r, "error: expected 'bool', but found 'int'\n");
        For(i = 0, i + 10, ++i, i += 5).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLIf, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kFloat, "a"), b(kFloat, "b");
    Statement x = If(a > b, a -= b);
    REPORTER_ASSERT(r, x.release()->description() == "if ((a > b)) (a -= b);");

    Statement y = If(a > b, a -= b, b -= a);
    REPORTER_ASSERT(r, y.release()->description() == "if ((a > b)) (a -= b); else (b -= a);");

    {
        ExpectError error(r, "error: expected 'bool', but found 'float'\n");
        If(a + b, a -= b).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLSwizzle, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kFloat4, "a");

    Expression e1 = a.x();
    REPORTER_ASSERT(r, e1.release()->description() == "a.x");

    Expression e2 = a.y();
    REPORTER_ASSERT(r, e2.release()->description() == "a.y");

    Expression e3 = a.z();
    REPORTER_ASSERT(r, e3.release()->description() == "a.z");

    Expression e4 = a.w();
    REPORTER_ASSERT(r, e4.release()->description() == "a.w");

    Expression e5 = a.r();
    REPORTER_ASSERT(r, e5.release()->description() == "a.x");

    Expression e6 = a.g();
    REPORTER_ASSERT(r, e6.release()->description() == "a.y");

    Expression e7 = a.b();
    REPORTER_ASSERT(r, e7.release()->description() == "a.z");

    Expression e8 = a.a();
    REPORTER_ASSERT(r, e8.release()->description() == "a.w");

    Expression e9 = Swizzle(a, R);
    REPORTER_ASSERT(r, e9.release()->description() == "a.x");

    Expression e10 = Swizzle(a, ZERO, G);
    REPORTER_ASSERT(r, e10.release()->description() == "float2(a.y, float(0)).yx");

    Expression e11 = Swizzle(a, B, G, G);
    REPORTER_ASSERT(r, e11.release()->description() == "a.zyy");

    Expression e12 = Swizzle(a, R, G, B, ONE);
    REPORTER_ASSERT(r, e12.release()->description() == "float4(a.xyz, float(1))");

    Expression e13 = Swizzle(a, R, G, B, ONE).r();
    REPORTER_ASSERT(r, e13.release()->description() == "float4(a.xyz, float(1)).x");
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLTernary, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt, "a");
    Expression x = Ternary(a > 0, 1, -1);
    REPORTER_ASSERT(r, x.release()->description() == "((a > 0) ? 1 : -1)");

    {
        ExpectError error(r, "error: expected 'bool', but found 'int'\n");
        Ternary(a, 1, -1).release();
    }

    {
        ExpectError error(r, "error: ternary operator result mismatch: 'float2', 'float3'\n");
        Ternary(a > 0, Float2(1), Float3(1)).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLWhile, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Statement x = While(true, Block());
    REPORTER_ASSERT(r, whitespace_insensitive_compare(x, "for (; true;) {}"));

    Var a(kFloat, "a"), b(kFloat, "b");
    Statement y = While(a != b, Block(a++, --b));
    REPORTER_ASSERT(r, whitespace_insensitive_compare(y, "for (; (a != b);) { a++; --b; }"));

    {
        ExpectError error(r, "error: expected 'bool', but found 'int'\n");
        While(7, Block()).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLIndex, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(Array(kInt, 5), "a"), b(kInt, "b");
    Expression e1 = a[0];
    REPORTER_ASSERT(r, e1.release()->description() == "a[0]");
    Expression e2 = a[b];
    REPORTER_ASSERT(r, e2.release()->description() == "a[b]");

    {
        ExpectError error(r, "error: expected 'int', but found 'bool'\n");
        a[true].release();
    }

    {
        ExpectError error(r, "error: expected array, but found 'int'\n");
        b[0].release();
    }

    {
        ExpectError error(r, "error: index -1 out of range for 'int[5]'\n");
        a[-1].release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLBuiltins, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    // There is a Fract type on Mac which can conflict with our Fract builtin
    using SkSL::dsl::Fract;
    Var a(kHalf4, "a"), b(kHalf4, "b"), c(kHalf4, "c");
    Var h3(kHalf3, "h3");
    Var b4(kBool4, "b4");
    REPORTER_ASSERT(r, Abs(a).release()->description()                 == "abs(a)");
    REPORTER_ASSERT(r, All(b4).release()->description()                == "all(b4)");
    REPORTER_ASSERT(r, Any(b4).release()->description()                == "any(b4)");
    REPORTER_ASSERT(r, Ceil(a).release()->description()                == "ceil(a)");
    REPORTER_ASSERT(r, Clamp(a, 0, 1).release()->description()         == "clamp(a, 0.0, 1.0)");
    REPORTER_ASSERT(r, Cos(a).release()->description()                 == "cos(a)");
    REPORTER_ASSERT(r, Cross(h3, h3).release()->description()          == "cross(h3, h3)");
    REPORTER_ASSERT(r, Degrees(a).release()->description()             == "degrees(a)");
    REPORTER_ASSERT(r, Distance(a, b).release()->description()         == "distance(a, b)");
    REPORTER_ASSERT(r, Dot(a, b).release()->description()              == "dot(a, b)");
    REPORTER_ASSERT(r, Equal(a, b).release()->description()            == "equal(a, b)");
    REPORTER_ASSERT(r, Exp(a).release()->description()                 == "exp(a)");
    REPORTER_ASSERT(r, Exp2(a).release()->description()                == "exp2(a)");
    REPORTER_ASSERT(r, Faceforward(a, b, c).release()->description()   == "faceforward(a, b, c)");
    REPORTER_ASSERT(r, Floor(a).release()->description()               == "floor(a)");
    REPORTER_ASSERT(r, Fract(a).release()->description()               == "fract(a)");
    REPORTER_ASSERT(r, GreaterThan(a, b).release()->description()      == "greaterThan(a, b)");
    REPORTER_ASSERT(r, GreaterThanEqual(a, b).release()->description() == "greaterThanEqual(a, b)");
    REPORTER_ASSERT(r, Inversesqrt(a).release()->description()         == "inversesqrt(a)");
    REPORTER_ASSERT(r, LessThan(a, b).release()->description()         == "lessThan(a, b)");
    REPORTER_ASSERT(r, LessThanEqual(a, b).release()->description()    == "lessThanEqual(a, b)");
    REPORTER_ASSERT(r, Length(a).release()->description()              == "length(a)");
    REPORTER_ASSERT(r, Log(a).release()->description()                 == "log(a)");
    REPORTER_ASSERT(r, Log2(a).release()->description()                == "log2(a)");
    REPORTER_ASSERT(r, Max(a, b).release()->description()              == "max(a, b)");
    REPORTER_ASSERT(r, Min(a, b).release()->description()              == "min(a, b)");
    REPORTER_ASSERT(r, Mix(a, b, c).release()->description()           == "mix(a, b, c)");
    REPORTER_ASSERT(r, Mod(a, b).release()->description()              == "mod(a, b)");
    REPORTER_ASSERT(r, Normalize(a).release()->description()           == "normalize(a)");
    REPORTER_ASSERT(r, NotEqual(a, b).release()->description()         == "notEqual(a, b)");
    REPORTER_ASSERT(r, Pow(a, b).release()->description()              == "pow(a, b)");
    REPORTER_ASSERT(r, Radians(a).release()->description()             == "radians(a)");
    REPORTER_ASSERT(r, Reflect(a, b).release()->description()          == "reflect(a, b)");
    REPORTER_ASSERT(r, Refract(a, b, 1).release()->description()       == "refract(a, b, 1.0)");
    REPORTER_ASSERT(r, Saturate(a).release()->description()            == "saturate(a)");
    REPORTER_ASSERT(r, Sign(a).release()->description()                == "sign(a)");
    REPORTER_ASSERT(r, Sin(a).release()->description()                 == "sin(a)");
    REPORTER_ASSERT(r, Smoothstep(a, b, c).release()->description()    == "smoothstep(a, b, c)");
    REPORTER_ASSERT(r, Sqrt(a).release()->description()                == "sqrt(a)");
    REPORTER_ASSERT(r, Step(a, b).release()->description()             == "step(a, b)");
    REPORTER_ASSERT(r, Tan(a).release()->description()                 == "tan(a)");
    REPORTER_ASSERT(r, Unpremul(a).release()->description()            == "unpremul(a)");

    // these calls all go through the normal channels, so it ought to be sufficient to prove that
    // one of them reports errors correctly
    {
        ExpectError error(r, "error: no match for ceil(bool)\n");
        Ceil(a == b).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLModifiers, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());

    Var v1(kConst_Modifier, kInt, "v1");
    Statement d1 = Declare(v1);
    REPORTER_ASSERT(r, d1.release()->description() == "const int v1;");

    // Most modifiers require an appropriate context to be legal. We can't yet give them that
    // context, so we can't as yet Declare() variables with these modifiers.
    // TODO: better tests when able
    Var v2(kIn_Modifier, kInt, "v2");
    REPORTER_ASSERT(r, DSLWriter::Var(v2).modifiers().fFlags == SkSL::Modifiers::kIn_Flag);

    Var v3(kOut_Modifier, kInt, "v3");
    REPORTER_ASSERT(r, DSLWriter::Var(v3).modifiers().fFlags == SkSL::Modifiers::kOut_Flag);

    Var v4(kUniform_Modifier, kInt, "v4");
    REPORTER_ASSERT(r, DSLWriter::Var(v4).modifiers().fFlags == SkSL::Modifiers::kUniform_Flag);

    Var v5(kFlat_Modifier, kInt, "v5");
    REPORTER_ASSERT(r, DSLWriter::Var(v5).modifiers().fFlags == SkSL::Modifiers::kFlat_Flag);

    Var v6(kNoPerspective_Modifier, kInt, "v6");
    REPORTER_ASSERT(r, DSLWriter::Var(v6).modifiers().fFlags ==
                       SkSL::Modifiers::kNoPerspective_Flag);

    Var v7(kIn_Modifier | kOut_Modifier, kInt, "v7");
    REPORTER_ASSERT(r, DSLWriter::Var(v7).modifiers().fFlags ==
                       (SkSL::Modifiers::kIn_Flag | SkSL::Modifiers::kOut_Flag));

    Var v8(kInOut_Modifier, kInt, "v8");
    REPORTER_ASSERT(r, DSLWriter::Var(v8).modifiers().fFlags ==
                       (SkSL::Modifiers::kIn_Flag | SkSL::Modifiers::kOut_Flag));
}
