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
#include "src/sksl/ir/SkSLPoison.h"

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
    SkASSERT(this->valid());
    DSLWriter::ReportErrors();
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

DSLExpression::DSLExpression(int64_t value)
    : fExpression(SkSL::IntLiteral::Make(DSLWriter::Context(),
                                         /*offset=*/-1,
                                         value)) {}

DSLExpression::DSLExpression(unsigned int value)
    : fExpression(SkSL::IntLiteral::Make(DSLWriter::Context(),
                                         /*offset=*/-1,
                                         value)) {}

DSLExpression::DSLExpression(bool value)
    : fExpression(SkSL::BoolLiteral::Make(DSLWriter::Context(),
                                          /*offset=*/-1,
                                          value)) {}

DSLExpression::DSLExpression(DSLVarBase& var) {
    fExpression = std::make_unique<SkSL::VariableReference>(/*offset=*/-1, DSLWriter::Var(var),
            SkSL::VariableReference::RefKind::kRead);
}

DSLExpression::DSLExpression(DSLVarBase&& var)
    : DSLExpression(var) {}

DSLExpression::DSLExpression(DSLPossibleExpression expr, PositionInfo pos) {
    DSLWriter::ReportErrors(pos);
    if (expr.valid()) {
        fExpression = std::move(expr.fExpression);
    } else {
        fExpression = SkSL::Poison::Make(DSLWriter::Context());
    }
}

DSLExpression::~DSLExpression() {
#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
    if (fExpression && DSLWriter::InFragmentProcessor()) {
        DSLWriter::CurrentEmitArgs()->fFragBuilder->codeAppend(
                DSLStatement(this->release()).release());
        return;
    }
#endif
    SkASSERTF(!fExpression || !DSLWriter::Settings().fAssertDSLObjectsReleased,
              "Expression destroyed without being incorporated into program (see "
              "ProgramSettings::fAssertDSLObjectsReleased)");
}

void DSLExpression::swap(DSLExpression& other) {
    std::swap(fExpression, other.fExpression);
}

std::unique_ptr<SkSL::Expression> DSLExpression::release() {
    SkASSERT(this->valid());
    return std::move(fExpression);
}

std::unique_ptr<SkSL::Expression> DSLExpression::releaseIfValid() {
    return std::move(fExpression);
}

DSLType DSLExpression::type() {
    if (!this->valid()) {
        return kVoid_Type;
    }
    return &fExpression->type();
}

DSLExpression DSLExpression::x(PositionInfo pos) {
    return Swizzle(std::move(*this), X, pos);
}

DSLExpression DSLExpression::y(PositionInfo pos) {
    return Swizzle(std::move(*this), Y, pos);
}

DSLExpression DSLExpression::z(PositionInfo pos) {
    return Swizzle(std::move(*this), Z, pos);
}

DSLExpression DSLExpression::w(PositionInfo pos) {
    return Swizzle(std::move(*this), W, pos);
}

DSLExpression DSLExpression::r(PositionInfo pos) {
    return Swizzle(std::move(*this), R, pos);
}

DSLExpression DSLExpression::g(PositionInfo pos) {
    return Swizzle(std::move(*this), G, pos);
}

DSLExpression DSLExpression::b(PositionInfo pos) {
    return Swizzle(std::move(*this), B, pos);
}

DSLExpression DSLExpression::a(PositionInfo pos) {
    return Swizzle(std::move(*this), A, pos);
}

DSLExpression DSLExpression::field(skstd::string_view name, PositionInfo pos) {
    return DSLExpression(DSLWriter::ConvertField(this->release(), name), pos);
}

DSLPossibleExpression DSLExpression::operator=(DSLExpression right) {
    return DSLWriter::ConvertBinary(this->release(), SkSL::Token::Kind::TK_EQ, right.release());
}

DSLPossibleExpression DSLExpression::operator[](DSLExpression right) {
    return DSLWriter::ConvertIndex(this->release(), right.release());
}

DSLPossibleExpression DSLExpression::operator()(SkTArray<DSLWrapper<DSLExpression>> args) {
    ExpressionArray converted;
    converted.reserve_back(args.count());
    for (DSLWrapper<DSLExpression>& arg : args) {
        converted.push_back(arg->release());
    }
    return DSLWriter::Call(this->release(), std::move(converted));
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
DSLPossibleExpression LogicalXor(DSLExpression left, DSLExpression right) {
    return DSLWriter::ConvertBinary(left.release(), SkSL::Token::Kind::TK_LOGICALXOR,
                                    right.release());
}
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

DSLPossibleExpression operator,(DSLPossibleExpression left, DSLExpression right) {
    return DSLWriter::ConvertBinary(DSLExpression(std::move(left)).release(),
                                    SkSL::Token::Kind::TK_COMMA, right.release());
}

DSLPossibleExpression operator,(DSLExpression left, DSLPossibleExpression right) {
    return DSLWriter::ConvertBinary(left.release(), SkSL::Token::Kind::TK_COMMA,
                                    DSLExpression(std::move(right)).release());
}

DSLPossibleExpression operator,(DSLPossibleExpression left, DSLPossibleExpression right) {
    return DSLWriter::ConvertBinary(DSLExpression(std::move(left)).release(),
                                    SkSL::Token::Kind::TK_COMMA,
                                    DSLExpression(std::move(right)).release());
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

void DSLPossibleExpression::reportErrors(PositionInfo pos) {
    SkASSERT(!this->valid());
    DSLWriter::ReportErrors(pos);
}

DSLType DSLPossibleExpression::type() {
    if (!this->valid()) {
        return kVoid_Type;
    }
    return &fExpression->type();
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

DSLExpression DSLPossibleExpression::field(skstd::string_view name, PositionInfo pos) {
    return DSLExpression(this->release()).field(name, pos);
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

DSLPossibleExpression DSLPossibleExpression::operator=(double expr) {
    return this->operator=(DSLExpression(expr));
}

DSLPossibleExpression DSLPossibleExpression::operator[](DSLExpression index) {
    return DSLExpression(this->release())[std::move(index)];
}

DSLPossibleExpression DSLPossibleExpression::operator()(SkTArray<DSLWrapper<DSLExpression>> args) {
    return DSLExpression(this->release())(std::move(args));
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

std::unique_ptr<SkSL::Expression> DSLPossibleExpression::release(PositionInfo pos) {
    return DSLExpression(std::move(*this), pos).release();
}

} // namespace dsl

} // namespace SkSL
