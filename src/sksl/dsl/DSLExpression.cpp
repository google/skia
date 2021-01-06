/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/DSLExpression.h"

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLBoolLiteral.h"
#include "src/sksl/ir/SkSLFloatLiteral.h"
#include "src/sksl/ir/SkSLIntLiteral.h"

namespace SkSL {

namespace dsl {

DSLExpression::DSLExpression() {}

DSLExpression::DSLExpression(std::unique_ptr<SkSL::Expression> expression)
    : fExpression(DSLWriter::Check(std::move(expression))) {}

DSLExpression::DSLExpression(float value)
    : fExpression(std::make_unique<SkSL::FloatLiteral>(DSLWriter::Context(),
                                                       /*offset=*/-1,
                                                       value)) {}

DSLExpression::DSLExpression(int value)
    : fExpression(std::make_unique<SkSL::IntLiteral>(DSLWriter::Context(),
                                                     /*offset=*/-1,
                                                     value)) {}

DSLExpression::DSLExpression(bool value)
    : fExpression(std::make_unique<SkSL::BoolLiteral>(DSLWriter::Context(),
                                                     /*offset=*/-1,
                                                     value)) {}

DSLExpression::~DSLExpression() {
    SkASSERTF(fExpression == nullptr,
              "Expression destroyed without being incorporated into output tree");
}

std::unique_ptr<SkSL::Expression> DSLExpression::release() {
    return std::move(fExpression);
}

DSLExpression DSLExpression::operator=(DSLExpression&& right) {
    return DSLWriter::ConvertBinary(this->release(), SkSL::Token::Kind::TK_EQ, right.release());
}

#define OP(op, token)                                                                              \
DSLExpression operator op(DSLExpression left, DSLExpression right) {                               \
    return DSLWriter::ConvertBinary(left.release(), SkSL::Token::Kind::token, right.release());    \
}

#define PREFIXOP(op, token)                                                                        \
DSLExpression operator op(DSLExpression expr) {                                                    \
    return DSLWriter::ConvertPrefix(SkSL::Token::Kind::token, expr.release());                     \
}

#define POSTFIXOP(op, token)                                                                       \
DSLExpression operator op(DSLExpression expr, int) {                                               \
    return DSLWriter::ConvertPostfix(expr.release(), SkSL::Token::Kind::token);                    \
}

OP(+, TK_PLUS)
OP(-, TK_MINUS)
OP(*, TK_STAR)
OP(/, TK_SLASH)
OP(%, TK_PERCENT)
OP(<<, TK_SHL)
OP(>>, TK_SHR)
OP(&&, TK_LOGICALAND)
OP(||, TK_LOGICALOR)
OP(&, TK_BITWISEAND)
OP(|, TK_BITWISEOR)
OP(^, TK_BITWISEXOR)
OP(==, TK_EQEQ)
OP(!=, TK_NEQ)
OP(>, TK_GT)
OP(<, TK_LT)
OP(>=, TK_GTEQ)
OP(<=, TK_LTEQ)

PREFIXOP(!, TK_LOGICALNOT)
PREFIXOP(~, TK_BITWISENOT)
PREFIXOP(++, TK_PLUSPLUS)
POSTFIXOP(++, TK_PLUSPLUS)
PREFIXOP(--, TK_MINUSMINUS)
POSTFIXOP(--, TK_MINUSMINUS)

DSLExpression operator,(DSLExpression left, DSLExpression right) {
    return DSLWriter::ConvertBinary(left.release(), SkSL::Token::Kind::TK_COMMA, right.release());
}

std::unique_ptr<SkSL::Expression> DSLExpression::coerceAndRelease(const SkSL::Type& type) {
    // tripping this assert means we had an error occur somewhere else in DSL construction that
    // wasn't caught where it should have been
    SkASSERTF(!DSLWriter::Compiler().errorCount(), "Unexpected SkSL DSL error: %s",
              DSLWriter::Compiler().errorText().c_str());
    return DSLWriter::Coerce(this->release(), type).release();
}

static SkSL::String swizzle_component(SwizzleComponent c) {
    switch (c) {
        case R:
            return "r";
        case G:
            return "g";
        case B:
            return "b";
        case A:
            return "a";
        case X:
            return "x";
        case Y:
            return "y";
        case Z:
            return "z";
        case W:
            return "w";
        case ZERO:
            return "0";
        case ONE:
            return "1";
        default:
            SkUNREACHABLE;
    }
}

DSLExpression DSLExpression::Swizzle(DSLExpression base, SwizzleComponent a) {
    return DSLWriter::Check(DSLWriter::IRGenerator().convertSwizzle(base.release(),
                                                                    swizzle_component(a)));
}

DSLExpression DSLExpression::Swizzle(DSLExpression base, SwizzleComponent a, SwizzleComponent b) {
    return DSLWriter::Check(DSLWriter::IRGenerator().convertSwizzle(base.release(),
                                                                    swizzle_component(a) +
                                                                    swizzle_component(b)));
}

DSLExpression DSLExpression::Swizzle(DSLExpression base, SwizzleComponent a, SwizzleComponent b,
                      SwizzleComponent c) {
    return DSLWriter::Check(DSLWriter::IRGenerator().convertSwizzle(base.release(),
                                                                    swizzle_component(a) +
                                                                    swizzle_component(b) +
                                                                    swizzle_component(c)));
}

DSLExpression DSLExpression::Swizzle(DSLExpression base, SwizzleComponent a, SwizzleComponent b,
                      SwizzleComponent c, SwizzleComponent d) {
    return DSLWriter::Check(DSLWriter::IRGenerator().convertSwizzle(base.release(),
                                                                    swizzle_component(a) +
                                                                    swizzle_component(b) +
                                                                    swizzle_component(c) +
                                                                    swizzle_component(d)));
}

} // namespace dsl

} // namespace SkSL
