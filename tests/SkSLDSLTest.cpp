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
