/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrDirectContext.h"
#include "include/private/base/SkTArray.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/SkSLProgramKind.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/dsl/DSL.h"
#include "src/sksl/dsl/DSLCore.h"
#include "src/sksl/dsl/DSLExpression.h"
#include "src/sksl/dsl/DSLLayout.h"
#include "src/sksl/dsl/DSLModifiers.h"
#include "src/sksl/dsl/DSLStatement.h"
#include "src/sksl/dsl/DSLType.h"
#include "src/sksl/dsl/DSLVar.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLModifiers.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "tests/Test.h"

#include <ctype.h>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using namespace skia_private;

struct GrContextOptions;

using namespace SkSL::dsl;

SkSL::ProgramSettings default_settings() {
    return SkSL::ProgramSettings{};
}

/**
 * Issues an automatic Start() and End().
 */
class AutoDSLContext {
public:
    AutoDSLContext(GrGpu* gpu, SkSL::ProgramSettings settings = default_settings(),
                   SkSL::ProgramKind kind = SkSL::ProgramKind::kFragment) {
        Start(gpu->shaderCompiler(), kind, settings);
    }

    ~AutoDSLContext() {
        End();
    }
};

class ExpectError : public SkSL::ErrorReporter {
public:
    ExpectError(skiatest::Reporter* reporter, const char* msg)
        : fMsg(msg)
        , fReporter(reporter)
        , fOldReporter(&GetErrorReporter()) {
        SetErrorReporter(this);
    }

    ~ExpectError() override {
        REPORTER_ASSERT(fReporter, !fMsg,
                        "Error mismatch: expected:\n%s\nbut no error occurred\n", fMsg);
        SetErrorReporter(fOldReporter);
    }

    void handleError(std::string_view msg, SkSL::Position pos) override {
        REPORTER_ASSERT(fReporter, fMsg, "Received unexpected extra error: %.*s\n",
                (int)msg.length(), msg.data());
        REPORTER_ASSERT(fReporter, !fMsg || msg == fMsg,
                "Error mismatch: expected:\n%s\nbut received:\n%.*s", fMsg, (int)msg.length(),
                msg.data());
        fMsg = nullptr;
    }

private:
    const char* fMsg;
    skiatest::Reporter* fReporter;
    ErrorReporter* fOldReporter;
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

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLStartup, r, ctxInfo) {
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

static std::string stringize(DSLStatement& stmt)          { return stmt.release()->description(); }
static std::string stringize(DSLExpression& expr)         { return expr.release()->description(); }
static std::string stringize(SkSL::IRNode& node)          { return node.description(); }

template <typename T>
static void expect_equal(skiatest::Reporter* r, int lineNumber, T& input, const char* expected) {
    std::string actual = stringize(input);
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

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLFlags, r, ctxInfo) {
    {
        SkSL::ProgramSettings settings = default_settings();
        settings.fAllowNarrowingConversions = true;
        AutoDSLContext context(ctxInfo.directContext()->priv().getGpu(), settings,
                               SkSL::ProgramKind::kFragment);
        Var x(kHalf_Type, "x");
        Var y(kFloat_Type, "y");
        EXPECT_EQUAL(x.assign(y), "x = half(y)");
    }
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLType, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    REPORTER_ASSERT(r,  DSLType(kBool_Type).isBoolean());
    REPORTER_ASSERT(r, !DSLType(kBool_Type).isNumber());
    REPORTER_ASSERT(r, !DSLType(kBool_Type).isFloat());
    REPORTER_ASSERT(r, !DSLType(kBool_Type).isSigned());
    REPORTER_ASSERT(r, !DSLType(kBool_Type).isUnsigned());
    REPORTER_ASSERT(r, !DSLType(kBool_Type).isInteger());
    REPORTER_ASSERT(r,  DSLType(kBool_Type).isScalar());
    REPORTER_ASSERT(r, !DSLType(kBool_Type).isVector());
    REPORTER_ASSERT(r, !DSLType(kBool_Type).isMatrix());
    REPORTER_ASSERT(r, !DSLType(kBool_Type).isArray());
    REPORTER_ASSERT(r, !DSLType(kBool_Type).isStruct());

    REPORTER_ASSERT(r, !DSLType(kInt_Type).isBoolean());
    REPORTER_ASSERT(r,  DSLType(kInt_Type).isNumber());
    REPORTER_ASSERT(r, !DSLType(kInt_Type).isFloat());
    REPORTER_ASSERT(r,  DSLType(kInt_Type).isSigned());
    REPORTER_ASSERT(r, !DSLType(kInt_Type).isUnsigned());
    REPORTER_ASSERT(r,  DSLType(kInt_Type).isInteger());
    REPORTER_ASSERT(r,  DSLType(kInt_Type).isScalar());
    REPORTER_ASSERT(r, !DSLType(kInt_Type).isVector());
    REPORTER_ASSERT(r, !DSLType(kInt_Type).isMatrix());
    REPORTER_ASSERT(r, !DSLType(kInt_Type).isArray());
    REPORTER_ASSERT(r, !DSLType(kInt_Type).isStruct());

    REPORTER_ASSERT(r, !DSLType(kUInt_Type).isBoolean());
    REPORTER_ASSERT(r,  DSLType(kUInt_Type).isNumber());
    REPORTER_ASSERT(r, !DSLType(kUInt_Type).isFloat());
    REPORTER_ASSERT(r, !DSLType(kUInt_Type).isSigned());
    REPORTER_ASSERT(r,  DSLType(kUInt_Type).isUnsigned());
    REPORTER_ASSERT(r,  DSLType(kUInt_Type).isInteger());
    REPORTER_ASSERT(r,  DSLType(kUInt_Type).isScalar());
    REPORTER_ASSERT(r, !DSLType(kUInt_Type).isVector());
    REPORTER_ASSERT(r, !DSLType(kUInt_Type).isMatrix());
    REPORTER_ASSERT(r, !DSLType(kUInt_Type).isArray());
    REPORTER_ASSERT(r, !DSLType(kUInt_Type).isStruct());

    REPORTER_ASSERT(r, !DSLType(kFloat_Type).isBoolean());
    REPORTER_ASSERT(r,  DSLType(kFloat_Type).isNumber());
    REPORTER_ASSERT(r,  DSLType(kFloat_Type).isFloat());
    REPORTER_ASSERT(r, !DSLType(kFloat_Type).isSigned());
    REPORTER_ASSERT(r, !DSLType(kFloat_Type).isUnsigned());
    REPORTER_ASSERT(r, !DSLType(kFloat_Type).isInteger());
    REPORTER_ASSERT(r,  DSLType(kFloat_Type).isScalar());
    REPORTER_ASSERT(r, !DSLType(kFloat_Type).isVector());
    REPORTER_ASSERT(r, !DSLType(kFloat_Type).isMatrix());
    REPORTER_ASSERT(r, !DSLType(kFloat_Type).isArray());
    REPORTER_ASSERT(r, !DSLType(kFloat_Type).isStruct());

    REPORTER_ASSERT(r, !DSLType(kFloat2_Type).isBoolean());
    REPORTER_ASSERT(r, !DSLType(kFloat2_Type).isNumber());
    REPORTER_ASSERT(r, !DSLType(kFloat2_Type).isFloat());
    REPORTER_ASSERT(r, !DSLType(kFloat2_Type).isSigned());
    REPORTER_ASSERT(r, !DSLType(kFloat2_Type).isUnsigned());
    REPORTER_ASSERT(r, !DSLType(kFloat2_Type).isInteger());
    REPORTER_ASSERT(r, !DSLType(kFloat2_Type).isScalar());
    REPORTER_ASSERT(r,  DSLType(kFloat2_Type).isVector());
    REPORTER_ASSERT(r, !DSLType(kFloat2_Type).isMatrix());
    REPORTER_ASSERT(r, !DSLType(kFloat2_Type).isArray());
    REPORTER_ASSERT(r, !DSLType(kFloat2_Type).isStruct());

    REPORTER_ASSERT(r, !DSLType(kHalf2x2_Type).isBoolean());
    REPORTER_ASSERT(r, !DSLType(kHalf2x2_Type).isNumber());
    REPORTER_ASSERT(r, !DSLType(kHalf2x2_Type).isFloat());
    REPORTER_ASSERT(r, !DSLType(kHalf2x2_Type).isSigned());
    REPORTER_ASSERT(r, !DSLType(kHalf2x2_Type).isUnsigned());
    REPORTER_ASSERT(r, !DSLType(kHalf2x2_Type).isInteger());
    REPORTER_ASSERT(r, !DSLType(kHalf2x2_Type).isScalar());
    REPORTER_ASSERT(r, !DSLType(kHalf2x2_Type).isVector());
    REPORTER_ASSERT(r,  DSLType(kHalf2x2_Type).isMatrix());
    REPORTER_ASSERT(r, !DSLType(kHalf2x2_Type).isArray());
    REPORTER_ASSERT(r, !DSLType(kHalf2x2_Type).isStruct());

    REPORTER_ASSERT(r, !DSLType(Array(kFloat_Type, 2)).isBoolean());
    REPORTER_ASSERT(r, !DSLType(Array(kFloat_Type, 2)).isNumber());
    REPORTER_ASSERT(r, !DSLType(Array(kFloat_Type, 2)).isFloat());
    REPORTER_ASSERT(r, !DSLType(Array(kFloat_Type, 2)).isSigned());
    REPORTER_ASSERT(r, !DSLType(Array(kFloat_Type, 2)).isUnsigned());
    REPORTER_ASSERT(r, !DSLType(Array(kFloat_Type, 2)).isInteger());
    REPORTER_ASSERT(r, !DSLType(Array(kFloat_Type, 2)).isScalar());
    REPORTER_ASSERT(r, !DSLType(Array(kFloat_Type, 2)).isVector());
    REPORTER_ASSERT(r, !DSLType(Array(kFloat_Type, 2)).isMatrix());
    REPORTER_ASSERT(r,  DSLType(Array(kFloat_Type, 2)).isArray());
    REPORTER_ASSERT(r, !DSLType(Array(kFloat_Type, 2)).isStruct());

    Var x(kFloat_Type, "x");
    DSLExpression e = x + 1;
    REPORTER_ASSERT(r, e.type().isFloat());
    e.release();

    {
        ExpectError error(r, "array size must be positive");
        Array(kFloat_Type, -1);
    }

    {
        ExpectError error(r, "multi-dimensional arrays are not supported");
        Array(Array(kFloat_Type, 2), 2);
    }
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLPlus, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kFloat_Type, "a"), b(kFloat_Type, "b");

    EXPECT_EQUAL(a + b,
                "a + b");
    EXPECT_EQUAL(a + 1,
                "a + 1.0");
    EXPECT_EQUAL(0.5 + a + -99,
               "(0.5 + a) + -99.0");
    EXPECT_EQUAL(a += b + 1,
                "a += b + 1.0");
    EXPECT_EQUAL(+a,
                 "a");
    EXPECT_EQUAL(+(a + b),
                  "a + b");

    {
        ExpectError error(r, "cannot assign to this expression");
        DSLExpression((1.0 += a)).release();
    }

    {
        ExpectError error(r, "'+' cannot operate on 'bool'");
        Var c(kBool_Type, "c");
        DSLExpression(+c).release();
    }
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLMinus, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kInt_Type, "b");

    EXPECT_EQUAL(a - b,
                "a - b");
    EXPECT_EQUAL(a - 1,
                "a - 1");
    EXPECT_EQUAL(2 - a - b,
               "(2 - a) - b");
    EXPECT_EQUAL(a -= b + 1,
                "a -= b + 1");
    EXPECT_EQUAL(-a,
                "-a");
    EXPECT_EQUAL(-(a - b),
                "-(a - b)");

    {
        ExpectError error(r, "cannot assign to this expression");
        DSLExpression(1.0 -= a).release();
    }

    {
        ExpectError error(r, "'-' cannot operate on 'bool'");
        Var c(kBool_Type, "c");
        DSLExpression(-c).release();
    }
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLMultiply, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kFloat_Type, "a"), b(kFloat_Type, "b");

    EXPECT_EQUAL(a * b,
                "a * b");
    EXPECT_EQUAL(a * 2,
                "a * 2.0");
    EXPECT_EQUAL(0.5 * a * -99,
               "(0.5 * a) * -99.0");
    EXPECT_EQUAL(a *= b + 1,
                "a *= b + 1.0");

    {
        ExpectError error(r, "cannot assign to this expression");
        DSLExpression(1.0 *= a).release();
    }
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLDivide, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kFloat_Type, "a"), b(kFloat_Type, "b");

    EXPECT_EQUAL(a / b,
                "a / b");
    EXPECT_EQUAL(a / 2,
                "a * 0.5");
    EXPECT_EQUAL(0.5 / a / -100,
               "(0.5 / a) * -0.01");
    EXPECT_EQUAL(b / (a - 1),
                "b / (a - 1.0)");
    EXPECT_EQUAL(a /= b + 1,
                "a /= b + 1.0");

    {
        ExpectError error(r, "cannot assign to this expression");
        DSLExpression(1.0 /= a).release();
    }

    {
        ExpectError error(r, "division by zero");
        DSLExpression(a /= 0).release();
    }
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLMod, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kInt_Type, "b");
    Expression e1 = a % b;
    EXPECT_EQUAL(e1, "a % b");

    Expression e2 = a % 2;
    EXPECT_EQUAL(e2, "a % 2");

    Expression e3 = 10 % a % -99;
    EXPECT_EQUAL(e3, "(10 % a) % -99");

    Expression e4 = a %= b + 1;
    EXPECT_EQUAL(e4, "a %= b + 1");

    {
        ExpectError error(r, "cannot assign to this expression");
        DSLExpression(1 %= a).release();
    }

    {
        ExpectError error(r, "division by zero");
        DSLExpression(a %= 0).release();
    }
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLShl, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kInt_Type, "b");
    Expression e1 = a << b;
    EXPECT_EQUAL(e1, "a << b");

    Expression e2 = a << 1;
    EXPECT_EQUAL(e2, "a << 1");

    Expression e3 = 1 << a << 2;
    EXPECT_EQUAL(e3, "(1 << a) << 2");

    Expression e4 = a <<= b + 1;
    EXPECT_EQUAL(e4, "a <<= b + 1");

    {
        ExpectError error(r, "cannot assign to this expression");
        DSLExpression(1 <<= a).release();
    }
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLShr, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kInt_Type, "b");
    Expression e1 = a >> b;
    EXPECT_EQUAL(e1, "a >> b");

    Expression e2 = a >> 1;
    EXPECT_EQUAL(e2, "a >> 1");

    Expression e3 = 1 >> a >> 2;
    EXPECT_EQUAL(e3, "(1 >> a) >> 2");

    Expression e4 = a >>= b + 1;
    EXPECT_EQUAL(e4, "a >>= b + 1");

    {
        ExpectError error(r, "cannot assign to this expression");
        DSLExpression(1 >>= a).release();
    }
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLBitwiseAnd, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kInt_Type, "b");
    Expression e1 = a & b;
    EXPECT_EQUAL(e1, "a & b");

    Expression e2 = a & 1;
    EXPECT_EQUAL(e2, "a & 1");

    Expression e3 = 1 & a & 2;
    EXPECT_EQUAL(e3, "(1 & a) & 2");

    Expression e4 = a &= b + 1;
    EXPECT_EQUAL(e4, "a &= b + 1");

    {
        ExpectError error(r, "cannot assign to this expression");
        DSLExpression(1 &= a).release();
    }
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLBitwiseOr, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kInt_Type, "b");
    Expression e1 = a | b;
    EXPECT_EQUAL(e1, "a | b");

    Expression e2 = a | 1;
    EXPECT_EQUAL(e2, "a | 1");

    Expression e3 = 1 | a | 2;
    EXPECT_EQUAL(e3, "(1 | a) | 2");

    Expression e4 = a |= b + 1;
    EXPECT_EQUAL(e4, "a |= b + 1");

    {
        ExpectError error(r, "cannot assign to this expression");
        DSLExpression(1 |= a).release();
    }
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLBitwiseXor, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kInt_Type, "b");
    Expression e1 = a ^ b;
    EXPECT_EQUAL(e1, "a ^ b");

    Expression e2 = a ^ 1;
    EXPECT_EQUAL(e2, "a ^ 1");

    Expression e3 = 1 ^ a ^ 2;
    EXPECT_EQUAL(e3, "(1 ^ a) ^ 2");

    Expression e4 = a ^= b + 1;
    EXPECT_EQUAL(e4, "a ^= b + 1");

    {
        ExpectError error(r, "cannot assign to this expression");
        DSLExpression(1 ^= a).release();
    }
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLLogicalAnd, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kBool_Type, "a"), b(kBool_Type, "b");
    Expression e1 = a && b;
    EXPECT_EQUAL(e1, "a && b");

    Expression e2 = a && true && b;
    EXPECT_EQUAL(e2, "a && b");

    Expression e3 = a && false && b;
    EXPECT_EQUAL(e3, "false");

    {
        ExpectError error(r, "type mismatch: '&&' cannot operate on 'bool', 'int'");
        DSLExpression(a && 5).release();
    }
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLLogicalOr, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kBool_Type, "a"), b(kBool_Type, "b");
    Expression e1 = a || b;
    EXPECT_EQUAL(e1, "a || b");

    Expression e2 = a || true || b;
    EXPECT_EQUAL(e2, "true");

    Expression e3 = a || false || b;
    EXPECT_EQUAL(e3, "a || b");

    {
        ExpectError error(r, "type mismatch: '||' cannot operate on 'bool', 'int'");
        DSLExpression(a || 5).release();
    }
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLLogicalXor, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kBool_Type, "a"), b(kBool_Type, "b");
    Expression e1 = LogicalXor(a, b);
    EXPECT_EQUAL(e1, "a ^^ b");

    {
        ExpectError error(r, "type mismatch: '^^' cannot operate on 'bool', 'int'");
        DSLExpression(LogicalXor(a, 5)).release();
    }
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLComma, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kInt_Type, "b");
    Expression e1 = (a += b, b);
    EXPECT_EQUAL(e1, "(a += b, b)");
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLEqual, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kInt_Type, "b");
    Expression e1 = a == b;
    EXPECT_EQUAL(e1, "a == b");

    Expression e2 = a == 5;
    EXPECT_EQUAL(e2, "a == 5");
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLNotEqual, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kInt_Type, "b");
    Expression e1 = a != b;
    EXPECT_EQUAL(e1, "a != b");

    Expression e2 = a != 5;
    EXPECT_EQUAL(e2, "a != 5");
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLGreaterThan, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kInt_Type, "b");
    Expression e1 = a > b;
    EXPECT_EQUAL(e1, "a > b");

    Expression e2 = a > 5;
    EXPECT_EQUAL(e2, "a > 5");
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLGreaterThanOrEqual, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kInt_Type, "b");
    Expression e1 = a >= b;
    EXPECT_EQUAL(e1, "a >= b");

    Expression e2 = a >= 5;
    EXPECT_EQUAL(e2, "a >= 5");
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLLessThan, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kInt_Type, "b");
    Expression e1 = a < b;
    EXPECT_EQUAL(e1, "a < b");

    Expression e2 = a < 5;
    EXPECT_EQUAL(e2, "a < 5");
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLLessThanOrEqual, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kInt_Type, "b");
    Expression e1 = a <= b;
    EXPECT_EQUAL(e1, "a <= b");

    Expression e2 = a <= 5;
    EXPECT_EQUAL(e2, "a <= 5");
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLLogicalNot, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kInt_Type, "b");
    Expression e1 = !(a <= b);
    EXPECT_EQUAL(e1, "!(a <= b)");

    {
        ExpectError error(r, "'!' cannot operate on 'int'");
        DSLExpression(!a).release();
    }
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLBitwiseNot, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kBool_Type, "b");
    Expression e1 = ~a;
    EXPECT_EQUAL(e1, "~a");

    {
        ExpectError error(r, "'~' cannot operate on 'bool'");
        DSLExpression(~b).release();
    }
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLIncrement, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kBool_Type, "b");
    Expression e1 = ++a;
    EXPECT_EQUAL(e1, "++a");

    Expression e2 = a++;
    EXPECT_EQUAL(e2, "a++");

    {
        ExpectError error(r, "'++' cannot operate on 'bool'");
        DSLExpression(++b).release();
    }

    {
        ExpectError error(r, "'++' cannot operate on 'bool'");
        DSLExpression(b++).release();
    }

    {
        ExpectError error(r, "cannot assign to this expression");
        DSLExpression(++(a + 1)).release();
    }

    {
        ExpectError error(r, "cannot assign to this expression");
        DSLExpression((a + 1)++).release();
    }
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLDecrement, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kBool_Type, "b");
    Expression e1 = --a;
    EXPECT_EQUAL(e1, "--a");

    Expression e2 = a--;
    EXPECT_EQUAL(e2, "a--");

    {
        ExpectError error(r, "'--' cannot operate on 'bool'");
        DSLExpression(--b).release();
    }

    {
        ExpectError error(r, "'--' cannot operate on 'bool'");
        DSLExpression(b--).release();
    }

    {
        ExpectError error(r, "cannot assign to this expression");
        DSLExpression(--(a + 1)).release();
    }

    {
        ExpectError error(r, "cannot assign to this expression");
        DSLExpression((a + 1)--).release();
    }
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLCall, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    {
        DSLExpression sqrt(SkSL::ThreadContext::Compiler().convertIdentifier(SkSL::Position(),
                "sqrt"));
        TArray<DSLExpression> args;
        args.emplace_back(16);
        EXPECT_EQUAL(sqrt(std::move(args)), "4.0");  // sqrt(16) gets optimized to 4
    }

    {
        DSLExpression pow(SkSL::ThreadContext::Compiler().convertIdentifier(SkSL::Position(),
                "pow"));
        DSLVar a(kFloat_Type, "a");
        DSLVar b(kFloat_Type, "b");
        TArray<DSLExpression> args;
        args.emplace_back(a);
        args.emplace_back(b);
        EXPECT_EQUAL(pow(std::move(args)), "pow(a, b)");
    }
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLDeclare, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    {
        DSLWriter::Reset();
        TArray<Var> vars;
        vars.push_back(Var(kBool_Type, "a", true));
        vars.push_back(Var(kFloat_Type, "b"));
        EXPECT_EQUAL(Declare(vars), "bool a = true; float b;");
    }

    {
        DSLWriter::Reset();
        Var a(kHalf4_Type, "a", 1);
        ExpectError error(r, "expected 'half4', but found 'int'");
        Declare(a).release();
    }

    {
        DSLWriter::Reset();
        Var a(kUniform_Modifier, kInt_Type, "a");
        ExpectError error(r, "'uniform' is not permitted here");
        Declare(a).release();
    }
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLDeclareGlobal, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    DSLGlobalVar x(kInt_Type, "x", 0);
    Declare(x);
    DSLGlobalVar y(kUniform_Modifier, kFloat2_Type, "y");
    Declare(y);
    REPORTER_ASSERT(r, SkSL::ThreadContext::ProgramElements().size() == 2);
    EXPECT_EQUAL(*SkSL::ThreadContext::ProgramElements()[0], "int x = 0;");
    EXPECT_EQUAL(*SkSL::ThreadContext::ProgramElements()[1], "uniform float2 y;");
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLInterfaceBlock, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    DSLExpression intf = InterfaceBlock(kUniform_Modifier, "InterfaceBlock1",
                                        {Field(kFloat_Type, "a"), Field(kInt_Type, "b")});
    REPORTER_ASSERT(r, SkSL::ThreadContext::ProgramElements().size() == 1);
    EXPECT_EQUAL(*SkSL::ThreadContext::ProgramElements().back(),
                 "uniform InterfaceBlock1 { float a; int b; };");
    EXPECT_EQUAL(intf.field("a"), "a");

    DSLExpression intf2 = InterfaceBlock(kUniform_Modifier, "InterfaceBlock2",
                                         {Field(kFloat2_Type, "x"), Field(kHalf2x2_Type, "y")},
                                         "blockVar");
    REPORTER_ASSERT(r, SkSL::ThreadContext::ProgramElements().size() == 2);
    EXPECT_EQUAL(*SkSL::ThreadContext::ProgramElements().back(),
                 "uniform InterfaceBlock2 { float2 x; half2x2 y; } blockVar;");
    EXPECT_EQUAL(intf2.field("x"), "blockVar.x");

    DSLExpression intf3 = InterfaceBlock(kUniform_Modifier, "InterfaceBlock3",
                                         {Field(kFloat_Type, "z")},
                                         "arrayVar", 4);
    REPORTER_ASSERT(r, SkSL::ThreadContext::ProgramElements().size() == 3);
    EXPECT_EQUAL(*SkSL::ThreadContext::ProgramElements().back(),
                 "uniform InterfaceBlock3 { float z; } arrayVar[4];");
    EXPECT_EQUAL(intf3[1].field("z"), "arrayVar[1].z");

    DSLExpression intf4 = InterfaceBlock(
            kUniform_Modifier, "InterfaceBlock4",
            {Field(DSLLayout().builtin(123), kFloat_Type, "sk_Widget")},
            "intf");
    REPORTER_ASSERT(r, SkSL::ThreadContext::ProgramElements().size() == 4);
    EXPECT_EQUAL(*SkSL::ThreadContext::ProgramElements().back(),
                 "uniform InterfaceBlock4 { layout(builtin=123) float sk_Widget; } intf;");
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLSelect, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a");
    Expression x = Select(a > 0, 1, -1);
    EXPECT_EQUAL(x, "a > 0 ? 1 : -1");

    {
        ExpectError error(r, "expected 'bool', but found 'int'");
        Select(a, 1, -1).release();
    }
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLIndex, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(Array(kInt_Type, 5), "a"), b(kInt_Type, "b");

    EXPECT_EQUAL(a[0], "a[0]");
    EXPECT_EQUAL(a[b], "a[b]");

    {
        ExpectError error(r, "expected 'int', but found 'bool'");
        a[true].release();
    }

    {
        ExpectError error(r, "expected array, but found 'int'");
        b[0].release();
    }

    {
        ExpectError error(r, "index -1 out of range for 'int[5]'");
        a[-1].release();
    }
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLModifiers, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());

    Var v1(kConst_Modifier, kInt_Type, "v1", 0);
    Statement d1 = Declare(v1);
    EXPECT_EQUAL(d1, "const int v1 = 0;");

    // Most modifiers require an appropriate context to be legal. We can't yet give them that
    // context, so we can't as yet Declare() variables with these modifiers.
    // TODO: better tests when able
    Var v2(kIn_Modifier, kInt_Type, "v2");
    REPORTER_ASSERT(r, v2.modifiers().flags() == SkSL::Modifiers::kIn_Flag);

    Var v3(kOut_Modifier, kInt_Type, "v3");
    REPORTER_ASSERT(r, v3.modifiers().flags() == SkSL::Modifiers::kOut_Flag);

    Var v4(kFlat_Modifier, kInt_Type, "v4");
    REPORTER_ASSERT(r, v4.modifiers().flags() == SkSL::Modifiers::kFlat_Flag);

    Var v5(kNoPerspective_Modifier, kInt_Type, "v5");
    REPORTER_ASSERT(r, v5.modifiers().flags() == SkSL::Modifiers::kNoPerspective_Flag);

    Var v6(kIn_Modifier | kOut_Modifier, kInt_Type, "v6");
    REPORTER_ASSERT(r, v6.modifiers().flags() == (SkSL::Modifiers::kIn_Flag |
                                                  SkSL::Modifiers::kOut_Flag));

    Var v7(kInOut_Modifier, kInt_Type, "v7");
    REPORTER_ASSERT(r, v7.modifiers().flags() == (SkSL::Modifiers::kIn_Flag |
                                                  SkSL::Modifiers::kOut_Flag));

    Var v8(kUniform_Modifier, kInt_Type, "v8");
    REPORTER_ASSERT(r, v8.modifiers().flags() == SkSL::Modifiers::kUniform_Flag);
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLLayout, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var v1(DSLModifiers(DSLLayout().location(1).offset(4).index(5).builtin(6)
                                   .inputAttachmentIndex(7),
                        kConst_Modifier), kInt_Type, "v1", 0);
    EXPECT_EQUAL(Declare(v1), "layout (location = 1, offset = 4, index = 5, "
                              "builtin = 6, input_attachment_index = 7) const int v1 = 0;");

    Var v2(DSLLayout().originUpperLeft(), kFloat2_Type, "v2");
    EXPECT_EQUAL(Declare(v2), "layout (origin_upper_left) float2 v2;");

    Var v4(DSLLayout().pushConstant(), kBool_Type, "v4");
    EXPECT_EQUAL(Declare(v4), "layout (push_constant) bool v4;");

    Var v5(DSLLayout().blendSupportAllEquations(), kHalf4_Type, "v5");
    EXPECT_EQUAL(Declare(v5), "layout (blend_support_all_equations) half4 v5;");

    {
        ExpectError error(r, "'layout(color)' is only permitted in runtime effects");
        DSLGlobalVar v(DSLModifiers(DSLLayout().color(), kUniform_Modifier), kHalf4_Type, "v");
        Declare(v);
    }

    {
        ExpectError error(r, "layout qualifier 'location' appears more than once");
        DSLLayout().location(1).location(2);
    }

    {
        ExpectError error(r, "layout qualifier 'set' appears more than once");
        DSLLayout().set(1).set(2);
    }

    {
        ExpectError error(r, "layout qualifier 'binding' appears more than once");
        DSLLayout().binding(1).binding(2);
    }

    {
        ExpectError error(r, "layout qualifier 'offset' appears more than once");
        DSLLayout().offset(1).offset(2);
    }

    {
        ExpectError error(r, "layout qualifier 'index' appears more than once");
        DSLLayout().index(1).index(2);
    }

    {
        ExpectError error(r, "layout qualifier 'builtin' appears more than once");
        DSLLayout().builtin(1).builtin(2);
    }

    {
        ExpectError error(r, "layout qualifier 'input_attachment_index' appears more than once");
        DSLLayout().inputAttachmentIndex(1).inputAttachmentIndex(2);
    }

    {
        ExpectError error(r, "layout qualifier 'origin_upper_left' appears more than once");
        DSLLayout().originUpperLeft().originUpperLeft();
    }

    {
        ExpectError error(r, "layout qualifier 'push_constant' appears more than once");
        DSLLayout().pushConstant().pushConstant();
    }

    {
        ExpectError error(r, "layout qualifier 'blend_support_all_equations' appears more than "
                             "once");
        DSLLayout().blendSupportAllEquations().blendSupportAllEquations();
    }

    {
        ExpectError error(r, "layout qualifier 'color' appears more than once");
        DSLLayout().color().color();
    }
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLRTAdjust, r, ctxInfo) {
    {
        AutoDSLContext context(ctxInfo.directContext()->priv().getGpu(), default_settings(),
                               SkSL::ProgramKind::kVertex);
        REPORTER_ASSERT(r, !SkSL::ThreadContext::RTAdjustState().fInterfaceBlock);

        DSLExpression intf = InterfaceBlock(kUniform_Modifier, "uniforms",
                                            {Field(kInt_Type, "unused"),
                                             Field(kFloat4_Type, "sk_RTAdjust")});
        REPORTER_ASSERT(r, SkSL::ThreadContext::RTAdjustState().fInterfaceBlock);
        REPORTER_ASSERT(r, SkSL::ThreadContext::RTAdjustState().fFieldIndex == 1);
    }

    {
        AutoDSLContext context(ctxInfo.directContext()->priv().getGpu(), default_settings(),
                               SkSL::ProgramKind::kVertex);
        ExpectError error(r, "sk_RTAdjust must have type 'float4'");
        InterfaceBlock(kUniform_Modifier, "uniforms",
                       { Field(kInt_Type, "unused"), Field(kHalf4_Type, "sk_RTAdjust") });
    }

    {
        AutoDSLContext context(ctxInfo.directContext()->priv().getGpu(), default_settings(),
                               SkSL::ProgramKind::kVertex);
        ExpectError error(r, "symbol 'sk_RTAdjust' was already defined");
        InterfaceBlock(kUniform_Modifier, "uniforms1",
                       {Field(kInt_Type, "unused1"), Field(kFloat4_Type, "sk_RTAdjust")});
        InterfaceBlock(kUniform_Modifier, "uniforms2",
                       {Field(kInt_Type, "unused2"), Field(kFloat4_Type, "sk_RTAdjust")});
    }
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLExtension, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    AddExtension("test_extension");
    REPORTER_ASSERT(r, SkSL::ThreadContext::ProgramElements().size() == 1);
    EXPECT_EQUAL(*SkSL::ThreadContext::ProgramElements()[0], "#extension test_extension : enable");
}
