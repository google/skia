/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/DSLExpression.h"

#include "include/sksl/DSLCore.h"
#include "include/sksl/DSLVar.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLBoolLiteral.h"
#include "src/sksl/ir/SkSLFloatLiteral.h"
#include "src/sksl/ir/SkSLIntLiteral.h"

#include "math.h"

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#endif

namespace SkSL {

namespace dsl {

DSLExpression::DSLExpression() {}

DSLExpression::DSLExpression(DSLExpression&& other)
    : fExpression(std::move(other.fExpression)) {}

DSLExpression::DSLExpression(std::unique_ptr<SkSL::Expression> expression)
    : fExpression(std::move(expression)) {
    if (DSLWriter::Compiler().errorCount()) {
        DSLWriter::ReportError(DSLWriter::Compiler().errorText(/*showCount=*/false).c_str());
        DSLWriter::Compiler().setErrorCount(0);
    }
}

DSLExpression::DSLExpression(float value)
    : fExpression(SkSL::FloatLiteral::Make(DSLWriter::Context(),
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
        : fExpression(SkSL::IntLiteral::Make(DSLWriter::Context(),
                                             /*offset=*/-1,
                                             value)) {}

DSLExpression::DSLExpression(bool value)
    : fExpression(SkSL::BoolLiteral::Make(DSLWriter::Context(),
                                          /*offset=*/-1,
                                          value)) {}

DSLExpression::DSLExpression(const DSLVar& var)
    : fExpression(std::make_unique<SkSL::VariableReference>(
                                                        /*offset=*/-1,
                                                        var.fVar,
                                                        SkSL::VariableReference::RefKind::kRead)) {}

DSLExpression::DSLExpression(DSLPossibleExpression expr, PositionInfo pos) {
    if (DSLWriter::Compiler().errorCount()) {
        DSLWriter::ReportError(DSLWriter::Compiler().errorText(/*showCount=*/false).c_str(), &pos);
        DSLWriter::Compiler().setErrorCount(0);
    }
    fExpression = std::move(expr.fExpression);
}

DSLExpression::~DSLExpression() {
#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
    if (fExpression && DSLWriter::InFragmentProcessor()) {
        DSLWriter::CurrentEmitArgs()->fFragBuilder->codeAppend(
                DSLStatement(this->release()).release());
        return;
    }
#endif
    SkASSERTF(fExpression == nullptr,
              "Expression destroyed without being incorporated into program");
}

std::unique_ptr<SkSL::Expression> DSLExpression::release() {
    return std::move(fExpression);
}

DSLExpression DSLExpression::x(PositionInfo pos) {
    return Swizzle(this->release(), X, pos);
}

DSLExpression DSLExpression::y(PositionInfo pos) {
    return Swizzle(this->release(), Y, pos);
}

DSLExpression DSLExpression::z(PositionInfo pos) {
    return Swizzle(this->release(), Z, pos);
}

DSLExpression DSLExpression::w(PositionInfo pos) {
    return Swizzle(this->release(), W, pos);
}

DSLExpression DSLExpression::r(PositionInfo pos) {
    return Swizzle(this->release(), R, pos);
}

DSLExpression DSLExpression::g(PositionInfo pos) {
    return Swizzle(this->release(), G, pos);
}

DSLExpression DSLExpression::b(PositionInfo pos) {
    return Swizzle(this->release(), B, pos);
}

DSLExpression DSLExpression::a(PositionInfo pos) {
    return Swizzle(this->release(), A, pos);
}

DSLExpression DSLExpression::field(const char* name, PositionInfo pos) {
    return DSLExpression(DSLWriter::ConvertField(this->release(), name), pos);
}

DSLPossibleExpression DSLExpression::operator=(DSLExpression right) {
    return DSLWriter::ConvertBinary(this->release(), SkSL::Token::Kind::TK_EQ, right.release());
}

DSLPossibleExpression DSLExpression::operator[](DSLExpression right) {
    return DSLWriter::ConvertIndex(this->release(), right.release());
}

#define OP(op, token)                                                                              \
DSLPossibleExpression operator op(DSLExpression left, DSLExpression right) {                       \
    return DSLWriter::ConvertBinary(left.release(), SkSL::Token::Kind::token, right.release());    \
}

#define PREFIXOP(op, token)                                                                        \
DSLPossibleExpression operator op(DSLExpression expr) {                                            \
    return DSLWriter::ConvertPrefix(SkSL::Token::Kind::token, expr.release());                     \
}

#define POSTFIXOP(op, token)                                                                       \
DSLPossibleExpression operator op(DSLExpression expr, int) {                                       \
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

PREFIXOP(+, TK_PLUS)
PREFIXOP(-, TK_MINUS)
PREFIXOP(!, TK_LOGICALNOT)
PREFIXOP(~, TK_BITWISENOT)
PREFIXOP(++, TK_PLUSPLUS)
POSTFIXOP(++, TK_PLUSPLUS)
PREFIXOP(--, TK_MINUSMINUS)
POSTFIXOP(--, TK_MINUSMINUS)

DSLPossibleExpression operator,(DSLExpression left, DSLExpression right) {
    return DSLWriter::ConvertBinary(left.release(), SkSL::Token::Kind::TK_COMMA,
                                    right.release());
}

std::unique_ptr<SkSL::Expression> DSLExpression::coerceAndRelease(const SkSL::Type& type) {
    // tripping this assert means we had an error occur somewhere else in DSL construction that
    // wasn't caught where it should have been
    SkASSERTF(!DSLWriter::Compiler().errorCount(), "Unexpected SkSL DSL error: %s",
              DSLWriter::Compiler().errorText().c_str());
    return DSLWriter::Coerce(this->release(), type).release();
}

DSLPossibleExpression::DSLPossibleExpression(std::unique_ptr<SkSL::Expression> expr)
    : fExpression(std::move(expr)) {}

DSLPossibleExpression::DSLPossibleExpression(DSLPossibleExpression&& other)
    : fExpression(std::move(other.fExpression)) {}

DSLPossibleExpression::~DSLPossibleExpression() {
    if (fExpression) {
        // this handles incorporating the expression into the output tree
        DSLExpression(std::move(fExpression));
    }
}

DSLExpression DSLPossibleExpression::x(PositionInfo pos) {
    return DSLExpression(this->release()).x(pos);
}

DSLExpression DSLPossibleExpression::y(PositionInfo pos) {
    return DSLExpression(this->release()).y(pos);
}

DSLExpression DSLPossibleExpression::z(PositionInfo pos) {
    return DSLExpression(this->release()).z(pos);
}

DSLExpression DSLPossibleExpression::w(PositionInfo pos) {
    return DSLExpression(this->release()).w(pos);
}

DSLExpression DSLPossibleExpression::r(PositionInfo pos) {
    return DSLExpression(this->release()).r(pos);
}

DSLExpression DSLPossibleExpression::g(PositionInfo pos) {
    return DSLExpression(this->release()).g(pos);
}

DSLExpression DSLPossibleExpression::b(PositionInfo pos) {
    return DSLExpression(this->release()).b(pos);
}

DSLExpression DSLPossibleExpression::a(PositionInfo pos) {
    return DSLExpression(this->release()).a(pos);
}

DSLExpression DSLPossibleExpression::field(const char* name, PositionInfo pos) {
    return DSLExpression(this->release()).field(name, pos);
}

DSLPossibleExpression DSLPossibleExpression::operator=(const DSLVar& var) {
    return this->operator=(DSLExpression(var));
}

DSLPossibleExpression DSLPossibleExpression::operator=(DSLExpression expr) {
    return DSLExpression(this->release()) = std::move(expr);
}

DSLPossibleExpression DSLPossibleExpression::operator=(int expr) {
    return this->operator=(DSLExpression(expr));
}

DSLPossibleExpression DSLPossibleExpression::operator=(float expr) {
    return this->operator=(DSLExpression(expr));
}

DSLPossibleExpression DSLPossibleExpression::operator[](DSLExpression index) {
    return DSLExpression(this->release())[std::move(index)];
}

DSLPossibleExpression DSLPossibleExpression::operator++() {
    return ++DSLExpression(this->release());
}

DSLPossibleExpression DSLPossibleExpression::operator++(int) {
    return DSLExpression(this->release())++;
}

DSLPossibleExpression DSLPossibleExpression::operator--() {
    return --DSLExpression(this->release());
}

DSLPossibleExpression DSLPossibleExpression::operator--(int) {
    return DSLExpression(this->release())--;
}

std::unique_ptr<SkSL::Expression> DSLPossibleExpression::release() {
    return std::move(fExpression);
}

} // namespace dsl

} // namespace SkSL
