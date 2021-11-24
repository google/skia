/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkSLIRNode.h"
#include "include/sksl/DSL.h"
#include "include/sksl/DSLRuntimeEffects.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLVariable.h"

#include "tests/Test.h"

#include <limits>

using namespace SkSL::dsl;

SkSL::ProgramSettings default_settings() {
    SkSL::ProgramSettings result;
    result.fDSLMarkVarsDeclared = true;
    result.fDSLMangling = false;
    return result;
}

SkSL::ProgramSettings no_mark_vars_declared() {
    SkSL::ProgramSettings result = default_settings();
    result.fDSLMarkVarsDeclared = false;
    return result;
}

/**
 * In addition to issuing an automatic Start() and End(), disables mangling and optionally
 * auto-declares variables during its lifetime. Variable auto-declaration simplifies testing so we
 * don't have to sprinkle all the tests with a bunch of Declare(foo).release() calls just to avoid
 * errors, especially given that some of the variables have options that make them an error to
 * actually declare.
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

    void handleError(skstd::string_view msg, SkSL::PositionInfo pos) override {
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
static SkSL::String stringize(DSLBlock& blck)              { return blck.release()->description(); }
static SkSL::String stringize(SkSL::IRNode& node)          { return node.description(); }
static SkSL::String stringize(SkSL::Program& program)      { return program.description(); }

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

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLFlags, r, ctxInfo) {
    {
        AutoDSLContext context(ctxInfo.directContext()->priv().getGpu(), no_mark_vars_declared());
        EXPECT_EQUAL(All(GreaterThan(Float4(1), Float4(0))), "true");

        Var x(kInt_Type, "x");
        EXPECT_EQUAL(Declare(x), "int x;");
    }

    {
        SkSL::ProgramSettings settings = default_settings();
        settings.fAllowNarrowingConversions = true;
        AutoDSLContext context(ctxInfo.directContext()->priv().getGpu(), settings,
                               SkSL::ProgramKind::kFragment);
        Var x(kHalf_Type, "x");
        Var y(kFloat_Type, "y");
        EXPECT_EQUAL(x = y, "(x = half(y))");
    }

    {
        AutoDSLContext context(ctxInfo.directContext()->priv().getGpu(), SkSL::ProgramSettings());
        Var x(kInt_Type, "x");
        EXPECT_EQUAL(Declare(x), "int _0_x;");
    }
}

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

    DSLVar x(kFloat_Type, "x");
    EXPECT_EQUAL(x = 1.0, "(x = 1.0)");
    EXPECT_EQUAL(x = 1.0f, "(x = 1.0)");

    DSLVar y(kFloat2_Type, "y");
    EXPECT_EQUAL(y.x() = 1.0, "(y.x = 1.0)");
    EXPECT_EQUAL(y.x() = 1.0f, "(y.x = 1.0)");

    {
        ExpectError error(r, "floating point value is infinite");
        Float(std::numeric_limits<float>::infinity()).release();
    }

    {
        ExpectError error(r, "floating point value is NaN");
        Float(std::numeric_limits<float>::quiet_NaN()).release();
    }

    {
        ExpectError error(r, "'float4' is not a valid parameter to 'float2' constructor; use '.xy' "
                             "instead");
        Float2(Float4(1)).release();
    }

    {
        ExpectError error(r, "invalid arguments to 'float4' constructor (expected 4 scalars, but "
                             "found 3)");
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
        ExpectError error(r, "floating point value is infinite");
        Half(std::numeric_limits<float>::infinity()).release();
    }

    {
        ExpectError error(r, "floating point value is NaN");
        Half(std::numeric_limits<float>::quiet_NaN()).release();
    }

    {
        ExpectError error(r, "'half4' is not a valid parameter to 'half2' constructor; use '.xy' "
                             "instead");
        Half2(Half4(1)).release();
    }

    {
        ExpectError error(r, "invalid arguments to 'half4' constructor (expected 4 scalars, but "
                             "found 3)");
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
        ExpectError error(r, "'int4' is not a valid parameter to 'int2' constructor; use '.xy' "
                             "instead");
        Int2(Int4(1)).release();
    }

    {
        ExpectError error(r, "invalid arguments to 'int4' constructor (expected 4 scalars, but "
                             "found 3)");
        Int4(Int3(1)).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLUInt, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());

    EXPECT_EQUAL(UInt(std::numeric_limits<uint32_t>::max()),
                "4294967295");
    EXPECT_EQUAL(UInt2(std::numeric_limits<uint32_t>::min()),
                "uint2(0)");
    EXPECT_EQUAL(UInt2(0, 1),
                "uint2(0, 1)");
    EXPECT_EQUAL(UInt3(0),
                "uint3(0)");
    EXPECT_EQUAL(UInt3(0, 1, 2),
                "uint3(0, 1, 2)");
    EXPECT_EQUAL(UInt4(0),
                "uint4(0)");
    EXPECT_EQUAL(UInt4(UInt2(0, 1), UInt2(2, 3)),
                "uint4(0, 1, 2, 3)");
    EXPECT_EQUAL(UInt4(0, 1, UInt2(2, 3)),
                "uint4(0, 1, 2, 3)");
    EXPECT_EQUAL(UInt4(0, 1, 2, 3),
                "uint4(0, 1, 2, 3)");

    {
        ExpectError error(r, "integer is out of range for type 'uint': -2");
        UInt3(UInt2(0, 1), -2).release();
    }

    {
        ExpectError error(r, "'uint4' is not a valid parameter to 'uint2' constructor; use '.xy' "
                             "instead");
        UInt2(UInt4(1)).release();
    }

    {
        ExpectError error(r, "invalid arguments to 'uint4' constructor (expected 4 scalars, but "
                             "found 3)");
        UInt4(UInt3(1)).release();
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
        ExpectError error(r, "'short4' is not a valid parameter to 'short2' constructor; use '.xy' "
                             "instead");
        Short2(Short4(1)).release();
    }

    {
        ExpectError error(r, "invalid arguments to 'short4' constructor (expected 4 scalars, but "
                             "found 3)");
        Short4(Short3(1)).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLUShort, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());

    EXPECT_EQUAL(UShort(std::numeric_limits<uint16_t>::max()),
                "65535");
    EXPECT_EQUAL(UShort2(std::numeric_limits<uint16_t>::min()),
                "ushort2(0)");
    EXPECT_EQUAL(UShort2(0, 1),
                "ushort2(0, 1)");
    EXPECT_EQUAL(UShort3(0),
                "ushort3(0)");
    EXPECT_EQUAL(UShort3(0, 1, 2),
                "ushort3(0, 1, 2)");
    EXPECT_EQUAL(UShort4(0),
                "ushort4(0)");
    EXPECT_EQUAL(UShort4(UShort2(0, 1), UShort2(2, 3)),
                "ushort4(0, 1, 2, 3)");
    EXPECT_EQUAL(UShort4(0, 1, UShort2(2, 3)),
                "ushort4(0, 1, 2, 3)");
    EXPECT_EQUAL(UShort4(0, 1, 2, 3),
                "ushort4(0, 1, 2, 3)");

    {
        ExpectError error(r, "integer is out of range for type 'ushort': -2");
        UShort3(UShort2(0, 1), -2).release();
    }

    {
        ExpectError error(r, "'ushort4' is not a valid parameter to 'ushort2' constructor; use "
                             "'.xy' instead");
        UShort2(UShort4(1)).release();
    }

    {
        ExpectError error(r, "invalid arguments to 'ushort4' constructor (expected 4 scalars, but "
                             "found 3)");
        UShort4(UShort3(1)).release();
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
        ExpectError error(r, "'bool4' is not a valid parameter to 'bool2' constructor; use '.xy' "
                             "instead");
        Bool2(Bool4(true)).release();
    }

    {
        ExpectError error(r, "invalid arguments to 'bool4' constructor (expected 4 scalars, but "
                             "found 3)");
        Bool4(Bool3(true)).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLType, r, ctxInfo) {
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

    Var x(kFloat_Type);
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
        ExpectError error(r, "invalid arguments to 'float3x3' constructor (expected 9 scalars, but "
                             "found 2)");
        DSLExpression(Float3x3(Float2(1))).release();
    }

    {
        ExpectError error(r, "invalid arguments to 'half2x2' constructor (expected 4 scalars, but "
                             "found 5)");
        DSLExpression(Half2x2(1, 2, 3, 4, 5)).release();
    }

    {
        ExpectError error(r, "type mismatch: '*' cannot operate on 'float4x3', 'float3'");
        DSLExpression(f43 * Float3(1)).release();
    }

    {
        ExpectError error(r, "type mismatch: '=' cannot operate on 'float4x3', 'float3x3'");
        DSLExpression(f43 = f33).release();
    }

    {
        ExpectError error(r, "type mismatch: '=' cannot operate on 'half2x2', 'float2x2'");
        DSLExpression(h22 = f22).release();
    }

    {
        ExpectError error(r, "no match for inverse(float4x3)");
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
        ExpectError error(r, "type mismatch: '+' cannot operate on 'bool2', 'float'");
        DSLExpression((Bool2(true) + a)).release();
    }

    {
        ExpectError error(r, "type mismatch: '+=' cannot operate on 'float', 'bool2'");
        DSLExpression((a += Bool2(true))).release();
    }

    {
        ExpectError error(r, "cannot assign to this expression");
        DSLExpression((1.0 += a)).release();
    }

    {
        ExpectError error(r, "'+' cannot operate on 'bool'");
        Var c(kBool_Type);
        DSLExpression(+c).release();
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
        ExpectError error(r, "type mismatch: '-' cannot operate on 'bool2', 'int'");
        DSLExpression(Bool2(true) - a).release();
    }

    {
        ExpectError error(r, "type mismatch: '-=' cannot operate on 'int', 'bool2'");
        DSLExpression(a -= Bool2(true)).release();
    }

    {
        ExpectError error(r, "cannot assign to this expression");
        DSLExpression(1.0 -= a).release();
    }

    {
        ExpectError error(r, "'-' cannot operate on 'bool'");
        Var c(kBool_Type);
        DSLExpression(-c).release();
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
        ExpectError error(r, "type mismatch: '*' cannot operate on 'bool2', 'float'");
        DSLExpression(Bool2(true) * a).release();
    }

    {
        ExpectError error(r, "type mismatch: '*=' cannot operate on 'float', 'bool2'");
        DSLExpression(a *= Bool2(true)).release();
    }

    {
        ExpectError error(r, "cannot assign to this expression");
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
        ExpectError error(r, "type mismatch: '/' cannot operate on 'bool2', 'float'");
        DSLExpression(Bool2(true) / a).release();
    }

    {
        ExpectError error(r, "type mismatch: '/=' cannot operate on 'float', 'bool2'");
        DSLExpression(a /= Bool2(true)).release();
    }

    {
        ExpectError error(r, "cannot assign to this expression");
        DSLExpression(1.0 /= a).release();
    }

    {
        ExpectError error(r, "division by zero");
        DSLExpression(a /= 0).release();
    }

    {
        Var c(kFloat2_Type, "c");
        ExpectError error(r, "division by zero");
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
        ExpectError error(r, "type mismatch: '%' cannot operate on 'bool2', 'int'");
        DSLExpression(Bool2(true) % a).release();
    }

    {
        ExpectError error(r, "type mismatch: '%=' cannot operate on 'int', 'bool2'");
        DSLExpression(a %= Bool2(true)).release();
    }

    {
        ExpectError error(r, "cannot assign to this expression");
        DSLExpression(1 %= a).release();
    }

    {
        ExpectError error(r, "division by zero");
        DSLExpression(a %= 0).release();
    }

    {
        Var c(kInt2_Type, "c");
        ExpectError error(r, "division by zero");
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
        ExpectError error(r, "type mismatch: '<<' cannot operate on 'bool2', 'int'");
        DSLExpression(Bool2(true) << a).release();
    }

    {
        ExpectError error(r, "type mismatch: '<<=' cannot operate on 'int', 'bool2'");
        DSLExpression(a <<= Bool2(true)).release();
    }

    {
        ExpectError error(r, "cannot assign to this expression");
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
        ExpectError error(r, "type mismatch: '>>' cannot operate on 'bool2', 'int'");
        DSLExpression(Bool2(true) >> a).release();
    }

    {
        ExpectError error(r, "type mismatch: '>>=' cannot operate on 'int', 'bool2'");
        DSLExpression(a >>= Bool2(true)).release();
    }

    {
        ExpectError error(r, "cannot assign to this expression");
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
        ExpectError error(r, "type mismatch: '&' cannot operate on 'bool2', 'int'");
        DSLExpression(Bool2(true) & a).release();
    }

    {
        ExpectError error(r, "type mismatch: '&=' cannot operate on 'int', 'bool2'");
        DSLExpression(a &= Bool2(true)).release();
    }

    {
        ExpectError error(r, "cannot assign to this expression");
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
        ExpectError error(r, "type mismatch: '|' cannot operate on 'bool2', 'int'");
        DSLExpression(Bool2(true) | a).release();
    }

    {
        ExpectError error(r, "type mismatch: '|=' cannot operate on 'int', 'bool2'");
        DSLExpression(a |= Bool2(true)).release();
    }

    {
        ExpectError error(r, "cannot assign to this expression");
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
        ExpectError error(r, "type mismatch: '^' cannot operate on 'bool2', 'int'");
        DSLExpression(Bool2(true) ^ a).release();
    }

    {
        ExpectError error(r, "type mismatch: '^=' cannot operate on 'int', 'bool2'");
        DSLExpression(a ^= Bool2(true)).release();
    }

    {
        ExpectError error(r, "cannot assign to this expression");
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
        ExpectError error(r, "type mismatch: '&&' cannot operate on 'bool', 'int'");
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
        ExpectError error(r, "type mismatch: '||' cannot operate on 'bool', 'int'");
        DSLExpression(a || 5).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLLogicalXor, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kBool_Type, "a"), b(kBool_Type, "b");
    Expression e1 = LogicalXor(a, b);
    EXPECT_EQUAL(e1, "(a ^^ b)");

    {
        ExpectError error(r, "type mismatch: '^^' cannot operate on 'bool', 'int'");
        DSLExpression(LogicalXor(a, 5)).release();
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
        ExpectError error(r, "type mismatch: '==' cannot operate on 'int', 'bool2'");
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
        ExpectError error(r, "type mismatch: '!=' cannot operate on 'int', 'bool2'");
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
        ExpectError error(r, "type mismatch: '>' cannot operate on 'int', 'bool2'");
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
        ExpectError error(r, "type mismatch: '>=' cannot operate on 'int', 'bool2'");
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
        ExpectError error(r, "type mismatch: '<' cannot operate on 'int', 'bool2'");
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
        ExpectError error(r, "type mismatch: '<=' cannot operate on 'int', 'bool2'");
        DSLExpression(a <= Bool2(true)).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLLogicalNot, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kInt_Type, "b");
    Expression e1 = !(a <= b);
    EXPECT_EQUAL(e1, "!(a <= b)");

    {
        ExpectError error(r, "'!' cannot operate on 'int'");
        DSLExpression(!a).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLBitwiseNot, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kBool_Type, "b");
    Expression e1 = ~a;
    EXPECT_EQUAL(e1, "~a");

    {
        ExpectError error(r, "'~' cannot operate on 'bool'");
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

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLDecrement, r, ctxInfo) {
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

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLCall, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    {
        DSLExpression sqrt(SkSL::ThreadContext::Compiler().convertIdentifier(/*line=*/-1, "sqrt"));
        SkTArray<DSLWrapper<DSLExpression>> args;
        args.emplace_back(16);
        EXPECT_EQUAL(sqrt(std::move(args)), "4.0");  // sqrt(16) gets optimized to 4
    }

    {
        DSLExpression pow(SkSL::ThreadContext::Compiler().convertIdentifier(/*line=*/-1, "pow"));
        DSLVar a(kFloat_Type, "a");
        DSLVar b(kFloat_Type, "b");
        SkTArray<DSLWrapper<DSLExpression>> args;
        args.emplace_back(a);
        args.emplace_back(b);
        EXPECT_EQUAL(pow(std::move(args)), "pow(a, b)");
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLBlock, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu(), no_mark_vars_declared());
    EXPECT_EQUAL(Block(), "{ }");
    Var a(kInt_Type, "a", 1), b(kInt_Type, "b", 2);
    EXPECT_EQUAL(Block(Declare(a), Declare(b), a = b), "{ int a = 1; int b = 2; (a = b); }");

    EXPECT_EQUAL((If(a > 0, --a), ++b), "if ((a > 0)) --a; ++b;");

    SkTArray<DSLStatement> statements;
    statements.push_back(a = 0);
    statements.push_back(++a);
    EXPECT_EQUAL(Block(std::move(statements)), "{ (a = 0); ++a; }");
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLBreak, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu(), no_mark_vars_declared());
    Var i(kInt_Type, "i", 0);
    DSLFunction(kVoid_Type, "success").define(
        For(Declare(i), i < 10, ++i, Block(
            If(i > 5, Break())
        ))
    );
    REPORTER_ASSERT(r, SkSL::ThreadContext::ProgramElements().size() == 1);
    EXPECT_EQUAL(*SkSL::ThreadContext::ProgramElements()[0],
                 "void success() { for (int i = 0; (i < 10); ++i) { if ((i > 5)) break; } }");

    {
        ExpectError error(r, "break statement must be inside a loop or switch");
        DSLFunction(kVoid_Type, "fail").define(
            Break()
        );
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLContinue, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu(), no_mark_vars_declared());
    Var i(kInt_Type, "i", 0);
    DSLFunction(kVoid_Type, "success").define(
        For(Declare(i), i < 10, ++i, Block(
            If(i < 5, Continue())
        ))
    );
    REPORTER_ASSERT(r, SkSL::ThreadContext::ProgramElements().size() == 1);
    EXPECT_EQUAL(*SkSL::ThreadContext::ProgramElements()[0],
                 "void success() { for (int i = 0; (i < 10); ++i) { if ((i < 5)) continue; } }");

    {
        ExpectError error(r, "continue statement must be inside a loop");
        DSLFunction(kVoid_Type, "fail").define(
            Continue()
        );
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLDeclare, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu(), no_mark_vars_declared());
    {
        Var a(kHalf4_Type, "a"), b(kHalf4_Type, "b", Half4(1));
        EXPECT_EQUAL(Declare(a), "half4 a;");
        EXPECT_EQUAL(Declare(b), "half4 b = half4(1.0);");
    }

    {
        DSLWriter::Reset();
        SkTArray<Var> vars;
        vars.push_back(Var(kBool_Type, "a", true));
        vars.push_back(Var(kFloat_Type, "b"));
        EXPECT_EQUAL(Declare(vars), "bool a = true; float b;");
    }

    {
        DSLWriter::Reset();
        REPORTER_ASSERT(r, SkSL::ThreadContext::ProgramElements().empty());
        GlobalVar a(kHalf4_Type, "a"), b(kHalf4_Type, "b", Half4(1));
        Declare(a);
        Declare(b);
        REPORTER_ASSERT(r, SkSL::ThreadContext::ProgramElements().size() == 2);
        EXPECT_EQUAL(*SkSL::ThreadContext::ProgramElements()[0], "half4 a;");
        EXPECT_EQUAL(*SkSL::ThreadContext::ProgramElements()[1], "half4 b = half4(1.0);");
    }

    {
        DSLWriter::Reset();
        REPORTER_ASSERT(r, SkSL::ThreadContext::ProgramElements().empty());
        SkTArray<GlobalVar> vars;
        vars.push_back(GlobalVar(kHalf4_Type, "a"));
        vars.push_back(GlobalVar(kHalf4_Type, "b", Half4(1)));
        Declare(vars);
        REPORTER_ASSERT(r, SkSL::ThreadContext::ProgramElements().size() == 2);
        EXPECT_EQUAL(*SkSL::ThreadContext::ProgramElements()[0], "half4 a;");
        EXPECT_EQUAL(*SkSL::ThreadContext::ProgramElements()[1], "half4 b = half4(1.0);");
    }

    {
        DSLWriter::Reset();
        Var a(kHalf4_Type, "a", 1);
        ExpectError error(r, "expected 'half4', but found 'int'");
        Declare(a).release();
    }

    {
        DSLWriter::Reset();
        Var a(kInt_Type, "a");
        Declare(a).release();
        ExpectError error(r, "variable has already been declared");
        Declare(a).release();
    }

    {
        DSLWriter::Reset();
        Var a(kUniform_Modifier, kInt_Type, "a");
        ExpectError error(r, "'uniform' is not permitted here");
        Declare(a).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLDeclareGlobal, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu(), no_mark_vars_declared());
    DSLGlobalVar x(kInt_Type, "x", 0);
    Declare(x);
    DSLGlobalVar y(kUniform_Modifier, kFloat2_Type, "y");
    Declare(y);
    REPORTER_ASSERT(r, SkSL::ThreadContext::ProgramElements().size() == 2);
    EXPECT_EQUAL(*SkSL::ThreadContext::ProgramElements()[0], "int x = 0;");
    EXPECT_EQUAL(*SkSL::ThreadContext::ProgramElements()[1], "uniform float2 y;");
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLDiscard, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var x(kFloat_Type, "x", 1);
    EXPECT_EQUAL(If(Sqrt(x) > 0, Discard()), "if ((sqrt(x) > 0.0)) discard;");
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLDo, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Statement x = Do(Block(), true);
    EXPECT_EQUAL(x, "do {} while (true);");

    Var a(kFloat_Type, "a"), b(kFloat_Type, "b");
    Statement y = Do(Block(a++, --b), a != b);
    EXPECT_EQUAL(y, "do { a++; --b; } while ((a != b));");

    {
        ExpectError error(r, "expected 'bool', but found 'int'");
        Do(Block(), 7).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLFor, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu(), no_mark_vars_declared());
    EXPECT_EQUAL(For(Statement(), Expression(), Expression(), Block()),
                "for (;;) {}");

    Var i(kInt_Type, "i", 0);
    EXPECT_EQUAL(For(Declare(i), i < 10, ++i, i += 5),
                "for (int i = 0; (i < 10); ++i) (i += 5);");

    Var j(kInt_Type, "j", 0);
    Var k(kInt_Type, "k", 10);
    EXPECT_EQUAL(For((Declare(j), Declare(k)), j < k, ++j, Block()), R"(
                 {
                     int j = 0;
                     int k = 10;
                     for (; (j < k); ++j) {}
                 }
    )");

    {
        ExpectError error(r, "expected 'bool', but found 'int'");
        For(i = 0, i + 10, ++i, i += 5).release();
    }

    {
        ExpectError error(r, "invalid for loop initializer");
        For(If(i == 0, i = 1), i < 10, ++i, i += 5).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLFunction, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu(), no_mark_vars_declared());
    Parameter coords(kFloat2_Type, "coords");
    DSLFunction(kVoid_Type, "main", coords).define(
        sk_FragColor() = Half4(coords, 0, 1)
    );
    REPORTER_ASSERT(r, SkSL::ThreadContext::ProgramElements().size() == 1);
    EXPECT_EQUAL(*SkSL::ThreadContext::ProgramElements()[0],
                 "void main(float2 coords) { (sk_FragColor = half4(half2(coords), 0.0, 1.0)); }");

    {
        DSLWriter::Reset();
        DSLParameter x(kFloat_Type, "x");
        DSLFunction sqr(kFloat_Type, "sqr", x);
        sqr.define(
            Return(x * x)
        );
        EXPECT_EQUAL(sqr(sk_FragCoord().x()), "sqr(sk_FragCoord.x)");
        REPORTER_ASSERT(r, SkSL::ThreadContext::ProgramElements().size() == 1);
        EXPECT_EQUAL(*SkSL::ThreadContext::ProgramElements()[0],
                "float sqr(float x) { return (x * x); }");
    }

    {
        DSLWriter::Reset();
        DSLParameter x(kFloat2_Type, "x");
        DSLParameter y(kFloat2_Type, "y");
        DSLFunction dot(kFloat2_Type, "dot", x, y);
        dot.define(
            Return(x * x + y * y)
        );
        EXPECT_EQUAL(dot(Float2(1.0f, 2.0f), Float2(3.0f, 4.0f)),
                     "dot(float2(1.0, 2.0), float2(3.0, 4.0))");
        REPORTER_ASSERT(r, SkSL::ThreadContext::ProgramElements().size() == 1);
        EXPECT_EQUAL(*SkSL::ThreadContext::ProgramElements()[0],
                "float2 dot(float2 x, float2 y) { return ((x * x) + (y * y)); }");
    }

    {
        DSLWriter::Reset();
        DSLParameter x(kFloat_Type, "x");
        DSLParameter y(kFloat_Type, "y");
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
        ExpectError error(r, "expected 'float', but found 'bool'");
        DSLWriter::Reset();
        DSLFunction(kFloat_Type, "broken").define(
            Return(true)
        );
    }

    {
        ExpectError error(r, "expected function to return 'float'");
        DSLWriter::Reset();
        DSLFunction(kFloat_Type, "broken").define(
            Return()
        );
    }

    {
        ExpectError error(r, "function 'broken' can exit without returning a value");
        DSLWriter::Reset();
        Var x(kFloat_Type, "x", 0);
        DSLFunction(kFloat_Type, "broken").define(
            Declare(x),
            If(x == 1, Return(x))
        );
    }

    {
        ExpectError error(r, "may not return a value from a void function");
        DSLWriter::Reset();
        DSLFunction(kVoid_Type, "broken").define(
            Return(0)
        );
    }

    {
        ExpectError error(r, "function 'broken' can exit without returning a value");
        DSLWriter::Reset();
        DSLFunction(kFloat_Type, "broken").define(
        );
    }

    {
        ExpectError error(r, "parameter has already been used in another function");
        DSLWriter::Reset();
        DSLParameter p(kFloat_Type);
        DSLFunction(kVoid_Type, "ok", p).define(
        );
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

    Statement z = StaticIf(a > b, a -= b, b -= a);
    EXPECT_EQUAL(z, "@if ((a > b)) (a -= b); else (b -= a);");

    {
        ExpectError error(r, "expected 'bool', but found 'float'");
        If(a + b, a -= b).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLInterfaceBlock, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    DSLGlobalVar intf = InterfaceBlock(kUniform_Modifier, "InterfaceBlock1",
                                       { Field(kFloat_Type, "a"), Field(kInt_Type, "b") });
    REPORTER_ASSERT(r, SkSL::ThreadContext::ProgramElements().size() == 1);
    EXPECT_EQUAL(*SkSL::ThreadContext::ProgramElements().back(),
                 "uniform InterfaceBlock1 { float a; int b; };");
    EXPECT_EQUAL(intf.field("a"), "InterfaceBlock1.a");

    DSLGlobalVar intf2 = InterfaceBlock(kUniform_Modifier, "InterfaceBlock2",
                                        { Field(kFloat2_Type, "x"), Field(kHalf2x2_Type, "y") },
                                  "blockVar");
    REPORTER_ASSERT(r, SkSL::ThreadContext::ProgramElements().size() == 2);
    EXPECT_EQUAL(*SkSL::ThreadContext::ProgramElements().back(),
                 "uniform InterfaceBlock2 { float2 x; half2x2 y; } blockVar;");
    EXPECT_EQUAL(intf2.field("x"), "blockVar.x");

    DSLGlobalVar intf3 = InterfaceBlock(kUniform_Modifier, "InterfaceBlock3",
                                        { Field(kFloat_Type, "z") },"arrayVar", 4);
    REPORTER_ASSERT(r, SkSL::ThreadContext::ProgramElements().size() == 3);
    EXPECT_EQUAL(*SkSL::ThreadContext::ProgramElements().back(),
                 "uniform InterfaceBlock3 { float z; } arrayVar[4];");
    EXPECT_EQUAL(intf3[1].field("z"), "arrayVar[1].z");
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
        ExpectError error(r, "expected 'bool', but found 'int'");
        Select(a, 1, -1).release();
    }

    {
        ExpectError error(r, "ternary operator result mismatch: 'float2', 'float3'");
        Select(a > 0, Float2(1), Float3(1)).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLSwitch, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());

    Var a(kFloat_Type, "a"), b(kInt_Type, "b");

    SkTArray<DSLStatement> caseStatements;
    caseStatements.push_back(a = 1);
    caseStatements.push_back(Continue());
    Statement x = Switch(b,
        Case(0, a = 0, Break()),
        Case(1, std::move(caseStatements)),
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

    Statement y = StaticSwitch(b,
        Case(0, a = 0, Break()),
        Case(1, a = 1, Continue()),
        Case(2, a = 2  /*Fallthrough*/),
        Default(Discard())
    );
    EXPECT_EQUAL(y, R"(
        @switch (b) {
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
        ExpectError error(r, "duplicate case value '0'");
        DSLStatement(Switch(0, Case(0), Case(0))).release();
    }

    {
        ExpectError error(r, "duplicate default case");
        DSLStatement(Switch(0, Default(a = 0), Default(a = 1))).release();
    }

    {
        ExpectError error(r, "case value must be a constant integer");
        Var c(kInt_Type);
        DSLStatement(Switch(0, Case(c))).release();
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


DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLVarSwap, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu(), no_mark_vars_declared());

    // We should be able to convert `a` into a proper var by swapping it, even from within a scope.
    Var a;
    if (true)
    {
        Var(kInt_Type, "a").swap(a);
    }

    EXPECT_EQUAL(Statement(Block(Declare(a), a = 123)),
                "{ int a; (a = 123); }");
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLWhile, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Statement x = While(true, Block());
    EXPECT_EQUAL(x, "for (; true;) {}");

    Var a(kFloat_Type, "a"), b(kFloat_Type, "b");
    Statement y = While(a != b, Block(a++, --b));
    EXPECT_EQUAL(y, "for (; (a != b);) { a++; --b; }");

    {
        ExpectError error(r, "expected 'bool', but found 'int'");
        While(7, Block()).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLIndex, r, ctxInfo) {
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
    EXPECT_EQUAL(Atan(a),                "atan(a)");
    EXPECT_EQUAL(Atan(a, b),             "atan(a, b)");
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
    EXPECT_EQUAL(Round(a),               "round(a)");
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
        ExpectError error(r, "no match for ceil(bool)");
        Ceil(a == b).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLModifiers, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu(), no_mark_vars_declared());

    Var v1(kConst_Modifier, kInt_Type, "v1", 0);
    Statement d1 = Declare(v1);
    EXPECT_EQUAL(d1, "const int v1 = 0;");

    // Most modifiers require an appropriate context to be legal. We can't yet give them that
    // context, so we can't as yet Declare() variables with these modifiers.
    // TODO: better tests when able
    Var v2(kIn_Modifier, kInt_Type, "v2");
    REPORTER_ASSERT(r, v2.modifiers().flags() == SkSL::Modifiers::kIn_Flag);
    DSLWriter::MarkDeclared(v2);

    Var v3(kOut_Modifier, kInt_Type, "v3");
    REPORTER_ASSERT(r, v3.modifiers().flags() == SkSL::Modifiers::kOut_Flag);
    DSLWriter::MarkDeclared(v3);

    Var v4(kFlat_Modifier, kInt_Type, "v4");
    REPORTER_ASSERT(r, v4.modifiers().flags() == SkSL::Modifiers::kFlat_Flag);
    DSLWriter::MarkDeclared(v4);

    Var v5(kNoPerspective_Modifier, kInt_Type, "v5");
    REPORTER_ASSERT(r, v5.modifiers().flags() == SkSL::Modifiers::kNoPerspective_Flag);
    DSLWriter::MarkDeclared(v5);

    Var v6(kIn_Modifier | kOut_Modifier, kInt_Type, "v6");
    REPORTER_ASSERT(r, v6.modifiers().flags() == (SkSL::Modifiers::kIn_Flag |
                                                  SkSL::Modifiers::kOut_Flag));
    DSLWriter::MarkDeclared(v6);

    Var v7(kInOut_Modifier, kInt_Type, "v7");
    REPORTER_ASSERT(r, v7.modifiers().flags() == (SkSL::Modifiers::kIn_Flag |
                                                  SkSL::Modifiers::kOut_Flag));
    DSLWriter::MarkDeclared(v7);

    Var v8(kUniform_Modifier, kInt_Type, "v8");
    REPORTER_ASSERT(r, v8.modifiers().flags() == SkSL::Modifiers::kUniform_Flag);
    DSLWriter::MarkDeclared(v8);
    // Uniforms do not need to be explicitly declared
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLLayout, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu(), no_mark_vars_declared());
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
        ExpectError error(r, "'srgb_unpremul' is only permitted in runtime effects");
        DSLGlobalVar v(DSLModifiers(DSLLayout().srgbUnpremul(), kUniform_Modifier), kHalf4_Type,
                       "v");
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
        ExpectError error(r, "layout qualifier 'srgb_unpremul' appears more than once");
        DSLLayout().srgbUnpremul().srgbUnpremul();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLSampleShader, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu(), default_settings(),
                           SkSL::ProgramKind::kRuntimeShader);
    DSLGlobalVar shader(kUniform_Modifier, kShader_Type, "child");
    DSLGlobalVar notShader(kUniform_Modifier, kFloat_Type, "x");
    EXPECT_EQUAL(shader.eval(Float2(0, 0)), "child.eval(float2(0.0, 0.0))");

    {
        ExpectError error(r, "no match for shader::eval(half4)");
        shader.eval(Half4(1)).release();
    }

    {
        ExpectError error(r, "type does not support method calls");
        notShader.eval(Half4(1)).release();
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLStruct, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu(), no_mark_vars_declared());

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
    REPORTER_ASSERT(r, SkSL::ThreadContext::ProgramElements().size() == 2);
    EXPECT_EQUAL(*SkSL::ThreadContext::ProgramElements()[0],
                 "struct SimpleStruct { float x; bool b; float[3] a; };");
    EXPECT_EQUAL(*SkSL::ThreadContext::ProgramElements()[1],
                 "SimpleStruct returnStruct() { SimpleStruct result; (result.x = 123.0);"
                 "(result.b = (result.x > 0.0)); (result.a[0] = result.x); return result; }");

    Struct("NestedStruct",
        Field(kInt_Type, "x"),
        Field(simpleStruct, "simple")
    );
    REPORTER_ASSERT(r, SkSL::ThreadContext::ProgramElements().size() == 3);
    EXPECT_EQUAL(*SkSL::ThreadContext::ProgramElements()[2],
                 "struct NestedStruct { int x; SimpleStruct simple; };");
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLWrapper, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    std::vector<Wrapper<DSLExpression>> exprs;
    exprs.push_back(DSLExpression(1));
    exprs.emplace_back(2.0);
    EXPECT_EQUAL(std::move(*exprs[0]), "1");
    EXPECT_EQUAL(std::move(*exprs[1]), "2.0");

    std::vector<Wrapper<DSLVar>> vars;
    vars.emplace_back(DSLVar(kInt_Type, "x"));
    REPORTER_ASSERT(r, DSLWriter::Var(*vars[0])->name() == "x");
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLRTAdjust, r, ctxInfo) {
    {
        AutoDSLContext context(ctxInfo.directContext()->priv().getGpu(), no_mark_vars_declared(),
                               SkSL::ProgramKind::kVertex);
        DSLGlobalVar rtAdjust(kUniform_Modifier, kFloat4_Type, "sk_RTAdjust");
        Declare(rtAdjust);
        DSLFunction(kVoid_Type, "main").define(
            sk_Position() = Half4(0)
        );
        REPORTER_ASSERT(r, SkSL::ThreadContext::ProgramElements().size() == 2);
        EXPECT_EQUAL(*SkSL::ThreadContext::ProgramElements()[1],
            "void main() {"
            "(sk_PerVertex.sk_Position = float4(0.0));"
            "(sk_PerVertex.sk_Position = float4(((sk_PerVertex.sk_Position.xy * sk_RTAdjust.xz) + "
            "(sk_PerVertex.sk_Position.ww * sk_RTAdjust.yw)), 0.0, sk_PerVertex.sk_Position.w));"
            "}");
    }

    {
        AutoDSLContext context(ctxInfo.directContext()->priv().getGpu(), no_mark_vars_declared(),
                               SkSL::ProgramKind::kVertex);
        REPORTER_ASSERT(r, !SkSL::ThreadContext::RTAdjustState().fInterfaceBlock);

        DSLGlobalVar intf = InterfaceBlock(kUniform_Modifier, "uniforms",
                                           { Field(kInt_Type, "unused"),
                                             Field(kFloat4_Type, "sk_RTAdjust") });
        REPORTER_ASSERT(r, SkSL::ThreadContext::RTAdjustState().fInterfaceBlock);
        REPORTER_ASSERT(r, SkSL::ThreadContext::RTAdjustState().fFieldIndex == 1);
    }

    {
        AutoDSLContext context(ctxInfo.directContext()->priv().getGpu(), no_mark_vars_declared(),
                               SkSL::ProgramKind::kVertex);
        ExpectError error(r, "sk_RTAdjust must have type 'float4'");
        InterfaceBlock(kUniform_Modifier, "uniforms",
                       { Field(kInt_Type, "unused"), Field(kHalf4_Type, "sk_RTAdjust") });
    }

    {
        AutoDSLContext context(ctxInfo.directContext()->priv().getGpu(), no_mark_vars_declared(),
                               SkSL::ProgramKind::kVertex);
        ExpectError error(r, "symbol 'sk_RTAdjust' was already defined");
        InterfaceBlock(kUniform_Modifier, "uniforms1",
                       { Field(kInt_Type, "unused1"), Field(kFloat4_Type, "sk_RTAdjust") });
        InterfaceBlock(kUniform_Modifier, "uniforms2",
                       { Field(kInt_Type, "unused2"), Field(kFloat4_Type, "sk_RTAdjust") });
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLInlining, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu(), no_mark_vars_declared());
    DSLParameter x(kFloat_Type, "x");
    DSLFunction sqr(kFloat_Type, "sqr", x);
    sqr.define(
        Return(x * x)
    );
    DSLFunction(kVoid_Type, "main").define(
        sk_FragColor() = (sqr(2), Half4(sqr(3)))
    );
    const char* source = "source test";
    std::unique_ptr<SkSL::Program> program = ReleaseProgram(std::make_unique<SkSL::String>(source));
    EXPECT_EQUAL(*program,
                 "layout(location = 0, index = 0, builtin = 10001) out half4 sk_FragColor;"
                 "layout(builtin = 17)in bool sk_Clockwise;"
                 "void main() {"
                 "/* inlined: sqr */;"
                 "/* inlined: sqr */;"
                 "(sk_FragColor = (4.0 , half4(half(9.0))));"
                 "}");
    REPORTER_ASSERT(r, *program->fSource == source);
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLReleaseUnused, r, ctxInfo) {
    SkSL::ProgramSettings settings = default_settings();
    settings.fAssertDSLObjectsReleased = false;
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu(), settings);
    If(Sqrt(1) > 0, Discard());
    // Ensure that we can safely destroy statements and expressions despite being unused while
    // settings.fAssertDSLObjectsReleased is disabled.
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLPrototypes, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu(), no_mark_vars_declared());
    {
        DSLParameter x(kFloat_Type, "x");
        DSLFunction sqr(kFloat_Type, "sqr", x);
        REPORTER_ASSERT(r, SkSL::ThreadContext::ProgramElements().size() == 1);
        EXPECT_EQUAL(*SkSL::ThreadContext::ProgramElements()[0], "float sqr(float x);");
        sqr.define(
            Return(x * x)
        );
        REPORTER_ASSERT(r, SkSL::ThreadContext::ProgramElements().size() == 1);
        EXPECT_EQUAL(*SkSL::ThreadContext::ProgramElements()[0],
                "float sqr(float x) { return (x * x); }");
    }

    {
        DSLWriter::Reset();
            DSLParameter x(kFloat_Type, "x");
        DSLFunction sqr(kFloat_Type, "sqr", x);
        REPORTER_ASSERT(r, SkSL::ThreadContext::ProgramElements().size() == 1);
        EXPECT_EQUAL(*SkSL::ThreadContext::ProgramElements()[0], "float sqr(float x);");
        DSLFunction(kVoid_Type, "main").define(sqr(5));
        REPORTER_ASSERT(r, SkSL::ThreadContext::ProgramElements().size() == 2);
        EXPECT_EQUAL(*SkSL::ThreadContext::ProgramElements()[0], "float sqr(float x);");
        EXPECT_EQUAL(*SkSL::ThreadContext::ProgramElements()[1], "void main() { sqr(5.0); }");
        sqr.define(
            Return(x * x)
        );
        REPORTER_ASSERT(r, SkSL::ThreadContext::ProgramElements().size() == 3);
        EXPECT_EQUAL(*SkSL::ThreadContext::ProgramElements()[2],
                "float sqr(float x) { return (x * x); }");

        const char* source = "source test";
        std::unique_ptr<SkSL::Program> p = ReleaseProgram(std::make_unique<SkSL::String>(source));
        EXPECT_EQUAL(*p,
            "layout (builtin = 17) in bool sk_Clockwise;"
            "float sqr(float x);"
            "void main() {"
            "/* inlined: sqr */;"
            "25.0;"
            "}");
    }
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLExtension, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    AddExtension("test_extension");
    REPORTER_ASSERT(r, SkSL::ThreadContext::ProgramElements().size() == 1);
    EXPECT_EQUAL(*SkSL::ThreadContext::ProgramElements()[0], "#extension test_extension : enable");
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLModifiersDeclaration, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Declare(Modifiers(Layout().blendSupportAllEquations(), kOut_Modifier));
    REPORTER_ASSERT(r, SkSL::ThreadContext::ProgramElements().size() == 1);
    EXPECT_EQUAL(*SkSL::ThreadContext::ProgramElements()[0],
            "layout(blend_support_all_equations) out;");
}

DEF_GPUTEST_FOR_MOCK_CONTEXT(DSLES3Types, r, ctxInfo) {
    StartRuntimeShader(ctxInfo.directContext()->priv().getGpu()->shaderCompiler());
    {
        ExpectError error(r, "type 'uint' is not supported");
        Var u(kUInt_Type, "u");
    }
    {
        ExpectError error(r, "type 'float3x2' is not supported");
        Float3x2(1).release();
    }
    {
        ExpectError error(r, "type 'uint' is not supported");
        Var u(kUInt_Type, "u");
    }
    {
        ExpectError error(r, "type '$genType' is private");
        Var g(DSLType("$genType"), "g");
    }
    Parameter p(kFloat2_Type, "p");
    Function(kHalf4_Type, "main", p).define(
        Return(Half4(0))
    );
    EndRuntimeShader();
}
