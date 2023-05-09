/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrDirectContext.h"
#include "include/private/SkSLDefines.h"
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
#include <cstdint>
#include <cstdlib>
#include <limits>
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

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLFloat, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Expression e1 = Float(std::numeric_limits<float>::max());

    // We can't use stof here, because old versions of libc++ can throw out_of_range on edge case
    // inputs like these. (This causes test failure on old Android devices.)
    REPORTER_ASSERT(r, std::strtof(e1.release()->description().c_str(), nullptr) ==
                       std::numeric_limits<float>::max());

    Expression e2 = Float(std::numeric_limits<float>::min());
    REPORTER_ASSERT(r, std::strtof(e2.release()->description().c_str(), nullptr) ==
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
    EXPECT_EQUAL(x.assign(1.0), "x = 1.0");
    EXPECT_EQUAL(x.assign(1.0f), "x = 1.0");

    DSLVar y(kFloat2_Type, "y");
    EXPECT_EQUAL(y.x().assign(1.0), "y.x = 1.0");
    EXPECT_EQUAL(y.x().assign(1.0f), "y.x = 1.0");

    {
        ExpectError error(r, "value is out of range for type 'float': inf");
        Float(std::numeric_limits<float>::infinity()).release();
    }

    {
        ExpectError error(r, "value is out of range for type 'float': nan");
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

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLHalf, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());

    // We can't use stof here, because old versions of libc++ can throw out_of_range on edge case
    // inputs like these. (This causes test failure on old Android devices.)
    Expression e1 = Half(std::numeric_limits<float>::max());
    REPORTER_ASSERT(r, std::strtof(e1.release()->description().c_str(), nullptr) ==
                       std::numeric_limits<float>::max());

    Expression e2 = Half(std::numeric_limits<float>::min());
    REPORTER_ASSERT(r, std::strtof(e2.release()->description().c_str(), nullptr) ==
                       std::numeric_limits<float>::min());

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
        ExpectError error(r, "value is out of range for type 'half': inf");
        Half(std::numeric_limits<float>::infinity()).release();
    }

    {
        ExpectError error(r, "value is out of range for type 'half': nan");
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

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLInt, r, ctxInfo) {
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

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLUInt, r, ctxInfo) {
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
        ExpectError error(r, "value is out of range for type 'uint': -2");
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

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLShort, r, ctxInfo) {
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

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLUShort, r, ctxInfo) {
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
        ExpectError error(r, "value is out of range for type 'ushort': -2");
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

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLBool, r, ctxInfo) {
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

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLMatrices, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var f22(kFloat2x2_Type, "f22");
    EXPECT_EQUAL(f22.assign(Float2x2(1)), "f22 = float2x2(1.0)");
    Var f32(kFloat3x2_Type, "f32");
    EXPECT_EQUAL(f32.assign(Float3x2(1, 2, 3, 4, 5, 6)),
                 "f32 = float3x2(1.0, 2.0, 3.0, 4.0, 5.0, 6.0)");
    Var f42(kFloat4x2_Type, "f42");
    EXPECT_EQUAL(f42.assign(Float4x2(Float4(1, 2, 3, 4), 5, 6, 7, 8)),
                 "f42 = float4x2(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0)");
    Var f23(kFloat2x3_Type, "f23");
    EXPECT_EQUAL(f23.assign(Float2x3(1, Float2(2, 3), 4, Float2(5, 6))),
                 "f23 = float2x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0)");
    Var f33(kFloat3x3_Type, "f33");
    EXPECT_EQUAL(f33.assign(Float3x3(Float3(1, 2, 3), 4, Float2(5, 6), 7, 8, 9)),
                 "f33 = float3x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0)");
    Var f43(kFloat4x3_Type, "f43");
    EXPECT_EQUAL(f43.assign(Float4x3(Float4(1, 2, 3, 4), Float4(5, 6, 7, 8), Float4(9, 10, 11,12))),
                 "f43 = float4x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0)");
    Var f24(kFloat2x4_Type, "f24");
    EXPECT_EQUAL(f24.assign(Float2x4(1, 2, 3, 4, 5, 6, 7, 8)),
                 "f24 = float2x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0)");
    Var f34(kFloat3x4_Type, "f34");
    EXPECT_EQUAL(f34.assign(Float3x4(1, 2, 3, 4, 5, 6, 7, 8, 9, Float3(10, 11, 12))),
                 "f34 = float3x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0)");
    Var f44(kFloat4x4_Type, "f44");
    EXPECT_EQUAL(f44.assign(Float4x4(1)), "f44 = float4x4(1.0)");

    Var h22(kHalf2x2_Type, "h22");
    EXPECT_EQUAL(h22.assign(Half2x2(1)), "h22 = half2x2(1.0)");
    Var h32(kHalf3x2_Type, "h32");
    EXPECT_EQUAL(h32.assign(Half3x2(1, 2, 3, 4, 5, 6)),
                 "h32 = half3x2(1.0, 2.0, 3.0, 4.0, 5.0, 6.0)");
    Var h42(kHalf4x2_Type, "h42");
    EXPECT_EQUAL(h42.assign(Half4x2(Half4(1, 2, 3, 4), 5, 6, 7, 8)),
                 "h42 = half4x2(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0)");
    Var h23(kHalf2x3_Type, "h23");
    EXPECT_EQUAL(h23.assign(Half2x3(1, Half2(2, 3), 4, Half2(5, 6))),
                 "h23 = half2x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0)");
    Var h33(kHalf3x3_Type, "h33");
    EXPECT_EQUAL(h33.assign(Half3x3(Half3(1, 2, 3), 4, Half2(5, 6), 7, 8, 9)),
                 "h33 = half3x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0)");
    Var h43(kHalf4x3_Type, "h43");
    EXPECT_EQUAL(h43.assign(Half4x3(Half4(1, 2, 3, 4), Half4(5, 6, 7, 8), Half4(9, 10, 11, 12))),
                 "h43 = half4x3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0)");
    Var h24(kHalf2x4_Type, "h24");
    EXPECT_EQUAL(h24.assign(Half2x4(1, 2, 3, 4, 5, 6, 7, 8)),
                 "h24 = half2x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0)");
    Var h34(kHalf3x4_Type, "h34");
    EXPECT_EQUAL(h34.assign(Half3x4(1, 2, 3, 4, 5, 6, 7, 8, 9, Half3(10, 11, 12))),
                 "h34 = half3x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0)");
    Var h44(kHalf4x4_Type, "h44");
    EXPECT_EQUAL(h44.assign(Half4x4(1)), "h44 = half4x4(1.0)");

    EXPECT_EQUAL(f22 * 2, "f22 * 2.0");
    EXPECT_EQUAL(f22 == Float2x2(1), "f22 == float2x2(1.0)");
    EXPECT_EQUAL(h42[0][1], "h42[0].y");
    EXPECT_EQUAL(f43 * Float4(0), "float3(0.0)");
    EXPECT_EQUAL(h23 * 2, "h23 * 2.0");

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
        DSLExpression(f43.assign(f33)).release();
    }

    {
        ExpectError error(r, "type mismatch: '=' cannot operate on 'half2x2', 'float2x2'");
        DSLExpression(h22.assign(f22)).release();
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

    Expression e2 = (a += b, b += b, Int2(a));
    EXPECT_EQUAL(e2, "((a += b, b += b), int2(a))");
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLEqual, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kInt_Type, "b");
    Expression e1 = a == b;
    EXPECT_EQUAL(e1, "a == b");

    Expression e2 = a == 5;
    EXPECT_EQUAL(e2, "a == 5");

    {
        ExpectError error(r, "type mismatch: '==' cannot operate on 'int', 'bool2'");
        DSLExpression(a == Bool2(true)).release();
    }
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLNotEqual, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kInt_Type, "b");
    Expression e1 = a != b;
    EXPECT_EQUAL(e1, "a != b");

    Expression e2 = a != 5;
    EXPECT_EQUAL(e2, "a != 5");

    {
        ExpectError error(r, "type mismatch: '!=' cannot operate on 'int', 'bool2'");
        DSLExpression(a != Bool2(true)).release();
    }
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLGreaterThan, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kInt_Type, "b");
    Expression e1 = a > b;
    EXPECT_EQUAL(e1, "a > b");

    Expression e2 = a > 5;
    EXPECT_EQUAL(e2, "a > 5");

    {
        ExpectError error(r, "type mismatch: '>' cannot operate on 'int', 'bool2'");
        DSLExpression(a > Bool2(true)).release();
    }
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLGreaterThanOrEqual, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kInt_Type, "b");
    Expression e1 = a >= b;
    EXPECT_EQUAL(e1, "a >= b");

    Expression e2 = a >= 5;
    EXPECT_EQUAL(e2, "a >= 5");

    {
        ExpectError error(r, "type mismatch: '>=' cannot operate on 'int', 'bool2'");
        DSLExpression(a >= Bool2(true)).release();
    }
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLLessThan, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kInt_Type, "b");
    Expression e1 = a < b;
    EXPECT_EQUAL(e1, "a < b");

    Expression e2 = a < 5;
    EXPECT_EQUAL(e2, "a < 5");

    {
        ExpectError error(r, "type mismatch: '<' cannot operate on 'int', 'bool2'");
        DSLExpression(a < Bool2(true)).release();
    }
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLLessThanOrEqual, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Var a(kInt_Type, "a"), b(kInt_Type, "b");
    Expression e1 = a <= b;
    EXPECT_EQUAL(e1, "a <= b");

    Expression e2 = a <= 5;
    EXPECT_EQUAL(e2, "a <= 5");

    {
        ExpectError error(r, "type mismatch: '<=' cannot operate on 'int', 'bool2'");
        DSLExpression(a <= Bool2(true)).release();
    }
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
        Var a(kHalf4_Type, "a"), b(kHalf4_Type, "b", Half4(1));
        EXPECT_EQUAL(Declare(a), "half4 a;");
        EXPECT_EQUAL(Declare(b), "half4 b = half4(1.0);");
    }

    {
        DSLWriter::Reset();
        TArray<Var> vars;
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
        TArray<GlobalVar> vars;
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

    {
        ExpectError error(r, "ternary operator result mismatch: 'float2', 'float3'");
        Select(a > 0, Float2(1), Float3(1)).release();
    }
}

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLSwizzle, r, ctxInfo) {
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

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLSampleShader, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu(), default_settings(),
                           SkSL::ProgramKind::kRuntimeShader);
    DSLGlobalVar shader(kUniform_Modifier, kShader_Type, "child");
    DSLGlobalVar notShader(kUniform_Modifier, kFloat_Type, "x");
    EXPECT_EQUAL(shader.eval(Float2(0, 0)), "child.eval(float2(0.0))");

    {
        ExpectError error(r, "no match for shader::eval(half4)");
        shader.eval(Half4(1)).release();
    }

    {
        ExpectError error(r, "type does not support method calls");
        notShader.eval(Half4(1)).release();
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

DEF_GANESH_TEST_FOR_MOCK_CONTEXT(DSLModifiersDeclaration, r, ctxInfo) {
    AutoDSLContext context(ctxInfo.directContext()->priv().getGpu());
    Declare(Modifiers(Layout().blendSupportAllEquations(), kOut_Modifier));
    REPORTER_ASSERT(r, SkSL::ThreadContext::ProgramElements().size() == 1);
    EXPECT_EQUAL(*SkSL::ThreadContext::ProgramElements()[0],
            "layout(blend_support_all_equations) out;");
}
