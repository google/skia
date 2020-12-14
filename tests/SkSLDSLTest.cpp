/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/dsl/DSL.h"

#include "tests/Test.h"

using namespace SkSL::dsl;

class AutoDisableMangle {
public:
    AutoDisableMangle() {
        fOldState = DSLWriter::ManglingEnabled();
        DSLWriter::SetManglingEnabled(false);
        DSLWriter::IRGenerator().pushSymbolTable();
    }

    ~AutoDisableMangle() {
        DSLWriter::SetManglingEnabled(fOldState);
        DSLWriter::IRGenerator().popSymbolTable();
    }

private:
    bool fOldState;
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

DEF_TEST(DSLPlus, r) {
    AutoDisableMangle disableMangle;
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
}

DEF_TEST(DSLMinus, r) {
    AutoDisableMangle disableMangle;
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
}

DEF_TEST(DSLMultiply, r) {
    AutoDisableMangle disableMangle;
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
}

DEF_TEST(DSLDivide, r) {
    AutoDisableMangle disableMangle;
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
}

DEF_TEST(DSLMod, r) {
    AutoDisableMangle disableMangle;
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
}

DEF_TEST(DSLShl, r) {
    AutoDisableMangle disableMangle;
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
}

DEF_TEST(DSLShr, r) {
    AutoDisableMangle disableMangle;
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
}

DEF_TEST(DSLBitwiseAnd, r) {
    AutoDisableMangle disableMangle;
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
}

DEF_TEST(DSLBitwiseOr, r) {
    AutoDisableMangle disableMangle;
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
}

DEF_TEST(DSLBitwiseXor, r) {
    AutoDisableMangle disableMangle;
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
}

DEF_TEST(DSLLogicalAnd, r) {
    AutoDisableMangle disableMangle;
    Var a(kBool, "a"), b(kBool, "b");
    Expression e1 = a && b;
    REPORTER_ASSERT(r, e1.release()->description() == "(a && b)");

    Expression e2 = a && true && b;
    REPORTER_ASSERT(r, e2.release()->description() == "(a && b)");

    Expression e3 = a && false && b;
    REPORTER_ASSERT(r, e3.release()->description() == "false");

    {
        ExpectError error(r, "error: expected 'bool', but found 'int'\n");
        (a && 5).release();
    }
}

DEF_TEST(DSLLogicalOr, r) {
    AutoDisableMangle disableMangle;
    Var a(kBool, "a"), b(kBool, "b");
    Expression e1 = a || b;
    REPORTER_ASSERT(r, e1.release()->description() == "(a || b)");

    Expression e2 = a || true || b;
    REPORTER_ASSERT(r, e2.release()->description() == "true");

    Expression e3 = a || false || b;
    REPORTER_ASSERT(r, e3.release()->description() == "(a || b)");

    {
        ExpectError error(r, "error: expected 'bool', but found 'int'\n");
        (a || 5).release();
    }
}

DEF_TEST(DSLComma, r) {
    AutoDisableMangle disableMangle;
    Var a(kInt, "a"), b(kInt, "b");
    Expression e1 = (a += b, b);
    REPORTER_ASSERT(r, e1.release()->description() == "((a += b) , b)");

    Expression e2 = (a += b, b += b, Int2(a));
    REPORTER_ASSERT(r, e2.release()->description() == "(((a += b) , (b += b)) , int2(a))");
}

DEF_TEST(DSLEqual, r) {
    AutoDisableMangle disableMangle;
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

DEF_TEST(DSLNotEqual, r) {
    AutoDisableMangle disableMangle;
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

DEF_TEST(DSLGreaterThan, r) {
    AutoDisableMangle disableMangle;
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

DEF_TEST(DSLGreaterThanOrEqual, r) {
    AutoDisableMangle disableMangle;
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

DEF_TEST(DSLLessThan, r) {
    AutoDisableMangle disableMangle;
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

DEF_TEST(DSLLessThanOrEqual, r) {
    AutoDisableMangle disableMangle;
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

DEF_TEST(DSLLogicalNot, r) {
    AutoDisableMangle disableMangle;
    Var a(kInt, "a"), b(kInt, "b");
    Expression e1 = !(a <= b);
    REPORTER_ASSERT(r, e1.release()->description() == "!(a <= b)");

    {
        ExpectError error(r, "error: '!' cannot operate on 'int'\n");
        (!a).release();
    }
}

DEF_TEST(DSLBitwiseNot, r) {
    AutoDisableMangle disableMangle;
    Var a(kInt, "a"), b(kBool, "b");
    Expression e1 = ~a;
    REPORTER_ASSERT(r, e1.release()->description() == "~a");

    {
        ExpectError error(r, "error: '~' cannot operate on 'bool'\n");
        (~b).release();
    }
}

DEF_TEST(DSLIncrement, r) {
    AutoDisableMangle disableMangle;
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

DEF_TEST(DSLDecrement, r) {
    AutoDisableMangle disableMangle;
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

DEF_TEST(DSLFloat, r) {
    AutoDisableMangle disableMangle;
    Expression e1 = Float(0);
    REPORTER_ASSERT(r, e1.release()->description() == "0.0");

    Expression e2 = Float2(0);
    REPORTER_ASSERT(r, e2.release()->description() == "float2(0.0)");

    Expression e3 = Float2(0, 1);
    REPORTER_ASSERT(r, e3.release()->description() == "float2(0.0, 1.0)");

    Expression e4 = Float3(0);
    REPORTER_ASSERT(r, e4.release()->description() == "float3(0.0)");

    Expression e5 = Float3(Float2(0, 1), 2);
    REPORTER_ASSERT(r, e5.release()->description() == "float3(float2(0.0, 1.0), 2.0)");

    Expression e6 = Float3(0, 1, 2);
    REPORTER_ASSERT(r, e6.release()->description() == "float3(0.0, 1.0, 2.0)");

    Expression e7 = Float4(0);
    REPORTER_ASSERT(r, e7.release()->description() == "float4(0.0)");

    Expression e8 = Float4(Float2(0, 1), Float2(2, 3));
    REPORTER_ASSERT(r, e8.release()->description() == "float4(float2(0.0, 1.0), float2(2.0, 3.0))");

    Expression e9 = Float4(0, 1, Float2(2, 3));
    REPORTER_ASSERT(r, e9.release()->description() == "float4(0.0, 1.0, float2(2.0, 3.0))");

    Expression e10 = Float4(0, 1, 2, 3);
    REPORTER_ASSERT(r, e10.release()->description() == "float4(0.0, 1.0, 2.0, 3.0)");

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

DEF_TEST(DSLHalf, r) {
    AutoDisableMangle disableMangle;
    Expression e1 = Half(0);
    REPORTER_ASSERT(r, e1.release()->description() == "0.0");

    Expression e2 = Half2(0);
    REPORTER_ASSERT(r, e2.release()->description() == "half2(0.0)");

    Expression e3 = Half2(0, 1);
    REPORTER_ASSERT(r, e3.release()->description() == "half2(0.0, 1.0)");

    Expression e4 = Half3(0);
    REPORTER_ASSERT(r, e4.release()->description() == "half3(0.0)");

    Expression e5 = Half3(Half2(0, 1), 2);
    REPORTER_ASSERT(r, e5.release()->description() == "half3(half2(0.0, 1.0), 2.0)");

    Expression e6 = Half3(0, 1, 2);
    REPORTER_ASSERT(r, e6.release()->description() == "half3(0.0, 1.0, 2.0)");

    Expression e7 = Half4(0);
    REPORTER_ASSERT(r, e7.release()->description() == "half4(0.0)");

    Expression e8 = Half4(Half2(0, 1), Half2(2, 3));
    REPORTER_ASSERT(r, e8.release()->description() == "half4(half2(0.0, 1.0), half2(2.0, 3.0))");

    Expression e9 = Half4(0, 1, Half2(2, 3));
    REPORTER_ASSERT(r, e9.release()->description() == "half4(0.0, 1.0, half2(2.0, 3.0))");

    Expression e10 = Half4(0, 1, 2, 3);
    REPORTER_ASSERT(r, e10.release()->description() == "half4(0.0, 1.0, 2.0, 3.0)");

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

DEF_TEST(DSLInt, r) {
    AutoDisableMangle disableMangle;
    Expression e1 = Int(0);
    REPORTER_ASSERT(r, e1.release()->description() == "0");

    Expression e2 = Int2(0);
    REPORTER_ASSERT(r, e2.release()->description() == "int2(0)");

    Expression e3 = Int2(0, 1);
    REPORTER_ASSERT(r, e3.release()->description() == "int2(0, 1)");

    Expression e4 = Int3(0);
    REPORTER_ASSERT(r, e4.release()->description() == "int3(0)");

    Expression e5 = Int3(Int2(0, 1), 2);
    REPORTER_ASSERT(r, e5.release()->description() == "int3(int2(0, 1), 2)");

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

DEF_TEST(DSLShort, r) {
    AutoDisableMangle disableMangle;
    Expression e1 = Short(0);
    REPORTER_ASSERT(r, e1.release()->description() == "short(0)");

    Expression e2 = Short2(0);
    REPORTER_ASSERT(r, e2.release()->description() == "short2(short(0))");

    Expression e3 = Short2(0, 1);
    REPORTER_ASSERT(r, e3.release()->description() == "short2(short(0), short(1))");

    Expression e4 = Short3(0);
    REPORTER_ASSERT(r, e4.release()->description() == "short3(short(0))");

    Expression e5 = Short3(Short2(0, 1), 2);
    REPORTER_ASSERT(r, e5.release()->description() == "short3(short2(short(0), short(1)), "
                                                      "short(2))");

    Expression e6 = Short3(0, 1, 2);
    REPORTER_ASSERT(r, e6.release()->description() == "short3(short(0), short(1), short(2))");

    Expression e7 = Short4(0);
    REPORTER_ASSERT(r, e7.release()->description() == "short4(short(0))");

    Expression e8 = Short4(Short2(0, 1), Short2(2, 3));
    REPORTER_ASSERT(r, e8.release()->description() == "short4(short2(short(0), short(1)), "
                                                      "short2(short(2), short(3)))");

    Expression e9 = Short4(0, 1, Short2(2, 3));
    REPORTER_ASSERT(r, e9.release()->description() == "short4(short(0), short(1), short2(short(2), "
                                                      "short(3)))");

    Expression e10 = Short4(0, 1, 2, 3);
    REPORTER_ASSERT(r, e10.release()->description() == "short4(short(0), short(1), short(2), "
                                                       "short(3))");

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

DEF_TEST(DSLBool, r) {
    AutoDisableMangle disableMangle;
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

DEF_TEST(DSLBlock, r) {
    AutoDisableMangle disableMangle;
    Statement x = Block();
    REPORTER_ASSERT(r, x.release()->description() == "{\n}\n");
    Var a(kInt, "a"), b(kInt, "b");
    Statement y = Block(Declare(a, 1), Declare(b, 2), a = b);
    REPORTER_ASSERT(r, y.release()->description() == "{\nint a = 1;\nint b = 2;\n(a = b);\n}\n");
}

DEF_TEST(DSLDeclare, r) {
    AutoDisableMangle disableMangle;
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

DEF_TEST(DSLDo, r) {
    AutoDisableMangle disableMangle;
    Statement x = Do(Block(), true);
    REPORTER_ASSERT(r, x.release()->description() == "do {\n}\n while (true);");

    Var a(kFloat, "a"), b(kFloat, "b");
    Statement y = Do(Block(a++, --b), a != b);
    REPORTER_ASSERT(r, y.release()->description() == "do {\na++;\n--b;\n}\n while ((a != b));");

    {
        ExpectError error(r, "error: expected 'bool', but found 'int'\n");
        Do(Block(), 7).release();
    }
}

DEF_TEST(DSLFor, r) {
    AutoDisableMangle disableMangle;
    Statement x = For(Statement(), Expression(), Expression(), Block());
    REPORTER_ASSERT(r, x.release()->description() == "for (; ; ) {\n}\n");

    Var i(kInt, "i");
    Statement y = For(Declare(i, 0), i < 10, ++i, i += 5);
    REPORTER_ASSERT(r, y.release()->description() == "for (int i = 0; (i < 10); ++i) (i += 5);");

    {
        ExpectError error(r, "error: expected 'bool', but found 'int'\n");
        For(i = 0, i + 10, ++i, i += 5).release();
    }
}

DEF_TEST(DSLFunction, r) {
    AutoDisableMangle disableMangle;
    DSLWriter::ProgramElements().clear();
    Var coords(kHalf2, "coords");
    DSLFunction(kVoid, "main", coords).define(
        sk_FragColor() = Half4(coords, 0, 1)
    );
    REPORTER_ASSERT(r, DSLWriter::ProgramElements().size() == 1);
    REPORTER_ASSERT(r, DSLWriter::ProgramElements()[0]->description() ==
R"(void main(half2 coords) {
(sk_FragColor = half4(coords, 0.0, 1.0));
}
)");
}

DEF_TEST(DSLIf, r) {
    AutoDisableMangle disableMangle;
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

DEF_TEST(DSLTernary, r) {
    AutoDisableMangle disableMangle;
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

DEF_TEST(DSLWhile, r) {
    AutoDisableMangle disableMangle;
    Statement x = While(true, Block());
    REPORTER_ASSERT(r, x.release()->description() == "while (true) {\n}\n");

    Var a(kFloat, "a"), b(kFloat, "b");
    Statement y = While(a != b, Block(a++, --b));
    REPORTER_ASSERT(r, y.release()->description() == "while ((a != b)) {\na++;\n--b;\n}\n");

    {
        ExpectError error(r, "error: expected 'bool', but found 'int'\n");
        While(7, Block()).release();
    }
}

DEF_TEST(DSLBuiltins, r) {
    AutoDisableMangle disableMangle;
    Var a(kHalf4, "a"), b(kHalf4, "b");
    REPORTER_ASSERT(r, ceil(a).release()->description()        == "ceil(a)");
    REPORTER_ASSERT(r, clamp(a, 0, 1).release()->description() == "clamp(a, 0.0, 1.0)");
    REPORTER_ASSERT(r, dot(a, b).release()->description()      == "dot(a, b)");
    REPORTER_ASSERT(r, floor(a).release()->description()       == "floor(a)");
    REPORTER_ASSERT(r, saturate(a).release()->description()    == "saturate(a)");
    REPORTER_ASSERT(r, unpremul(a).release()->description()    == "unpremul(a)");

    // these calls all go through the normal channels, so it ought to be sufficient to prove that
    // one of them reports errors correctly
    {
        ExpectError error(r, "error: no match for ceil(bool)\n");
        ceil(a == b).release();
    }
}
