/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/DSLExpression.h"

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/dsl/DSLVar.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLBoolLiteral.h"
#include "src/sksl/ir/SkSLFloatLiteral.h"
#include "src/sksl/ir/SkSLIntLiteral.h"

#include "math.h"

namespace SkSL {

namespace dsl {

DSLExpression::DSLExpression() {}

DSLExpression::DSLExpression(std::unique_ptr<SkSL::Expression> expression)
    : fExpression(DSLWriter::Check(std::move(expression))) {}

DSLExpression::DSLExpression(float value)
    : fExpression(std::make_unique<SkSL::FloatLiteral>(DSLWriter::Context(),
                                                       /*offset=*/-1,
                                                       value)) {
    if (!isfinite(value)) {
        if (isinf(value)) {
            DSLWriter::ReportError("error: floating point value is infinite\n");
        } else if (isnan(value)) {
            DSLWriter::ReportError("error: floating point value is NaN\n");
        }
    }
}

DSLExpression::DSLExpression(int value)
    : fExpression(std::make_unique<SkSL::IntLiteral>(DSLWriter::Context(),
                                                     /*offset=*/-1,
                                                     value)) {}

DSLExpression::DSLExpression(bool value)
    : fExpression(std::make_unique<SkSL::BoolLiteral>(DSLWriter::Context(),
                                                     /*offset=*/-1,
                                                     value)) {}

DSLExpression::DSLExpression(const DSLVar& var)
    : fExpression(std::make_unique<SkSL::VariableReference>(
                                                        /*offset=*/-1,
                                                        var.var(),
                                                        SkSL::VariableReference::RefKind::kRead)) {}

DSLExpression::~DSLExpression() {
    SkASSERTF(fExpression == nullptr,
              "Expression destroyed without being incorporated into output tree");
}

std::unique_ptr<SkSL::Expression> DSLExpression::release() {
    return std::move(fExpression);
}

DSLExpression DSLExpression::operator=(DSLExpression right) {
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
OP(+=, TK_PLUSEQ)
OP(-, TK_MINUS)
OP(-=, TK_MINUSEQ)
OP(*, TK_STAR)
OP(*=, TK_STAREQ)
OP(/, TK_SLASH)
OP(/=, TK_SLASHEQ)
OP(%, TK_PERCENT)
OP(%=, TK_PERCENTEQ)
OP(<<, TK_SHL)
OP(<<=, TK_SHLEQ)
OP(>>, TK_SHR)
OP(>>=, TK_SHREQ)
OP(&&, TK_LOGICALAND)
OP(||, TK_LOGICALOR)
OP(&, TK_BITWISEAND)
OP(&=, TK_BITWISEANDEQ)
OP(|, TK_BITWISEOR)
OP(|=, TK_BITWISEOREQ)
OP(^, TK_BITWISEXOR)
OP(^=, TK_BITWISEXOREQ)
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

} // namespace dsl

} // namespace SkSL
