/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/DSLExpression.h"

#include "include/core/SkTypes.h"
#include "include/private/SkSLDefines.h"
#include "include/sksl/DSLCore.h"
#include "include/sksl/DSLType.h"
#include "include/sksl/DSLVar.h"
#include "include/sksl/DSLWrapper.h"
#include "include/sksl/SkSLOperator.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLLiteral.h"
#include "src/sksl/ir/SkSLPoison.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLVariableReference.h"

#include <math.h>
#include <utility>

namespace SkSL {

namespace dsl {

DSLExpression::DSLExpression() {}

DSLExpression::DSLExpression(DSLExpression&& other)
    : fExpression(std::move(other.fExpression)) {}

DSLExpression::DSLExpression(std::unique_ptr<SkSL::Expression> expression)
    : fExpression(std::move(expression)) {
    SkASSERT(this->hasValue());
}

DSLExpression::DSLExpression(std::unique_ptr<SkSL::Expression> expression, Position pos)
    : fExpression(expression ? std::move(expression)
                             : SkSL::Poison::Make(pos, ThreadContext::Context())) {
    ThreadContext::ReportErrors(pos);
    SkASSERTF(this->position() == pos, "expected expression position (%d-%d), but received (%d-%d)",
            pos.startOffset(), pos.endOffset(), this->position().startOffset(),
            this->position().endOffset());
}

DSLExpression::DSLExpression(float value, Position pos)
    : fExpression(SkSL::Literal::MakeFloat(ThreadContext::Context(),
                                           pos,
                                           value)) {
    if (!isfinite(value)) {
        if (isinf(value)) {
            ThreadContext::ReportError("floating point value is infinite");
        } else if (isnan(value)) {
            ThreadContext::ReportError("floating point value is NaN");
        }
    }
}

DSLExpression::DSLExpression(int value, Position pos)
    : fExpression(SkSL::Literal::MakeInt(ThreadContext::Context(),
                                         pos,
                                         value)) {}

DSLExpression::DSLExpression(int64_t value, Position pos)
    : fExpression(SkSL::Literal::MakeInt(ThreadContext::Context(),
                                         pos,
                                         value)) {}

DSLExpression::DSLExpression(unsigned int value, Position pos)
    : fExpression(SkSL::Literal::MakeInt(ThreadContext::Context(),
                                         pos,
                                         value)) {}

DSLExpression::DSLExpression(bool value, Position pos)
    : fExpression(SkSL::Literal::MakeBool(ThreadContext::Context(),
                                          pos,
                                          value)) {}

DSLExpression::DSLExpression(DSLVarBase& var, Position pos) {
    fExpression = std::make_unique<SkSL::VariableReference>(pos, DSLWriter::Var(var),
            SkSL::VariableReference::RefKind::kRead);
}

DSLExpression::DSLExpression(DSLVarBase&& var, Position pos)
    : DSLExpression(var) {}

DSLExpression::DSLExpression(DSLPossibleExpression expr, Position pos) {
    ThreadContext::ReportErrors(pos);
    if (expr.valid()) {
        fExpression = std::move(expr.fExpression);
        if (!this->position().valid()) {
            fExpression->fPosition = pos;
        }
    } else {
        fExpression = SkSL::Poison::Make(pos, ThreadContext::Context());
    }
}

DSLExpression DSLExpression::Poison(Position pos) {
    return DSLExpression(SkSL::Poison::Make(pos, ThreadContext::Context()));
}

DSLExpression::~DSLExpression() {
    SkASSERTF(!fExpression || !ThreadContext::Settings().fAssertDSLObjectsReleased,
              "Expression destroyed without being incorporated into program (see "
              "ProgramSettings::fAssertDSLObjectsReleased)");
}

bool DSLExpression::isValid() const {
    return this->hasValue() && !fExpression->is<SkSL::Poison>();
}

void DSLExpression::swap(DSLExpression& other) {
    std::swap(fExpression, other.fExpression);
}

std::unique_ptr<SkSL::Expression> DSLExpression::release() {
    SkASSERT(this->hasValue());
    return std::move(fExpression);
}

std::unique_ptr<SkSL::Expression> DSLExpression::releaseIfPossible() {
    return std::move(fExpression);
}

DSLType DSLExpression::type() const {
    if (!this->hasValue()) {
        return kVoid_Type;
    }
    return &fExpression->type();
}

std::string DSLExpression::description() const {
    SkASSERT(this->hasValue());
    return fExpression->description();
}

Position DSLExpression::position() const {
    SkASSERT(this->hasValue());
    return fExpression->fPosition;
}

void DSLExpression::setPosition(Position pos) {
    SkASSERT(this->hasValue());
    fExpression->fPosition = pos;
}

DSLExpression DSLExpression::x(Position pos) {
    return Swizzle(std::move(*this), X, pos);
}

DSLExpression DSLExpression::y(Position pos) {
    return Swizzle(std::move(*this), Y, pos);
}

DSLExpression DSLExpression::z(Position pos) {
    return Swizzle(std::move(*this), Z, pos);
}

DSLExpression DSLExpression::w(Position pos) {
    return Swizzle(std::move(*this), W, pos);
}

DSLExpression DSLExpression::r(Position pos) {
    return Swizzle(std::move(*this), R, pos);
}

DSLExpression DSLExpression::g(Position pos) {
    return Swizzle(std::move(*this), G, pos);
}

DSLExpression DSLExpression::b(Position pos) {
    return Swizzle(std::move(*this), B, pos);
}

DSLExpression DSLExpression::a(Position pos) {
    return Swizzle(std::move(*this), A, pos);
}

DSLExpression DSLExpression::field(std::string_view name, Position pos) {
    return DSLExpression(FieldAccess::Convert(ThreadContext::Context(), pos,
            *ThreadContext::SymbolTable(), this->release(), name), pos);
}

DSLPossibleExpression DSLExpression::operator=(DSLExpression right) {
    Position pos = this->position().rangeThrough(right.position());
    return BinaryExpression::Convert(ThreadContext::Context(), pos, this->release(),
            SkSL::Operator::Kind::EQ, right.release());
}

DSLPossibleExpression DSLExpression::operator[](DSLExpression right) {
    Position pos = this->position().rangeThrough(right.position());
    return IndexExpression::Convert(ThreadContext::Context(), *ThreadContext::SymbolTable(), pos,
            this->release(), right.release());
}

DSLExpression DSLExpression::index(DSLExpression index, Position pos) {
    std::unique_ptr<SkSL::Expression> result = IndexExpression::Convert(ThreadContext::Context(),
            *ThreadContext::SymbolTable(), pos, this->release(), index.release());
    return DSLExpression(std::move(result), pos);
}

DSLPossibleExpression DSLExpression::operator()(SkTArray<DSLWrapper<DSLExpression>> args,
                                                Position pos) {
    ExpressionArray converted;
    converted.reserve_back(args.count());
    for (DSLWrapper<DSLExpression>& arg : args) {
        converted.push_back(arg->release());
    }
    return (*this)(std::move(converted), pos);
}

DSLPossibleExpression DSLExpression::operator()(ExpressionArray args, Position pos) {
    return SkSL::FunctionCall::Convert(ThreadContext::Context(), pos, this->release(),
            std::move(args));
}

DSLExpression DSLExpression::prefix(Operator::Kind op, Position pos) {
    std::unique_ptr<SkSL::Expression> result = PrefixExpression::Convert(ThreadContext::Context(),
            pos, op, this->release());
    return DSLExpression(std::move(result), pos);
}

DSLExpression DSLExpression::postfix(Operator::Kind op, Position pos) {
    std::unique_ptr<SkSL::Expression> result = PostfixExpression::Convert(ThreadContext::Context(),
            pos, this->release(), op);
    return DSLExpression(std::move(result), pos);
}

DSLExpression DSLExpression::binary(Operator::Kind op, DSLExpression right, Position pos) {
    std::unique_ptr<SkSL::Expression> result = BinaryExpression::Convert(ThreadContext::Context(),
            pos, this->release(), op, right.release());
    return DSLExpression(std::move(result), pos);
}

#define OP(op, token)                                                                              \
DSLPossibleExpression operator op(DSLExpression left, DSLExpression right) {                       \
    return BinaryExpression::Convert(ThreadContext::Context(), Position(), left.release(),         \
            Operator::Kind::token, right.release());                                               \
}

#define PREFIXOP(op, token)                                                                        \
DSLPossibleExpression operator op(DSLExpression expr) {                                            \
    return PrefixExpression::Convert(ThreadContext::Context(), Position(), Operator::Kind::token,  \
            expr.release());                                                                       \
}

#define POSTFIXOP(op, token)                                                                       \
DSLPossibleExpression operator op(DSLExpression expr, int) {                                       \
    return PostfixExpression::Convert(ThreadContext::Context(), Position(), expr.release(),        \
            Operator::Kind::token);                                                                \
}

OP(+, PLUS)
OP(+=, PLUSEQ)
OP(-, MINUS)
OP(-=, MINUSEQ)
OP(*, STAR)
OP(*=, STAREQ)
OP(/, SLASH)
OP(/=, SLASHEQ)
OP(%, PERCENT)
OP(%=, PERCENTEQ)
OP(<<, SHL)
OP(<<=, SHLEQ)
OP(>>, SHR)
OP(>>=, SHREQ)
OP(&&, LOGICALAND)
OP(||, LOGICALOR)
OP(&, BITWISEAND)
OP(&=, BITWISEANDEQ)
OP(|, BITWISEOR)
OP(|=, BITWISEOREQ)
OP(^, BITWISEXOR)
OP(^=, BITWISEXOREQ)
DSLPossibleExpression LogicalXor(DSLExpression left, DSLExpression right) {
    return BinaryExpression::Convert(ThreadContext::Context(), Position(), left.release(),
            SkSL::Operator::Kind::LOGICALXOR, right.release());
}
OP(==, EQEQ)
OP(!=, NEQ)
OP(>, GT)
OP(<, LT)
OP(>=, GTEQ)
OP(<=, LTEQ)

PREFIXOP(+, PLUS)
PREFIXOP(-, MINUS)
PREFIXOP(!, LOGICALNOT)
PREFIXOP(~, BITWISENOT)
PREFIXOP(++, PLUSPLUS)
POSTFIXOP(++, PLUSPLUS)
PREFIXOP(--, MINUSMINUS)
POSTFIXOP(--, MINUSMINUS)

DSLPossibleExpression operator,(DSLExpression left, DSLExpression right) {
    return BinaryExpression::Convert(ThreadContext::Context(), Position(), left.release(),
            SkSL::Operator::Kind::COMMA, right.release());
}

DSLPossibleExpression operator,(DSLPossibleExpression left, DSLExpression right) {
    return BinaryExpression::Convert(ThreadContext::Context(), Position(),
            DSLExpression(std::move(left)).release(), SkSL::Operator::Kind::COMMA, right.release());
}

DSLPossibleExpression operator,(DSLExpression left, DSLPossibleExpression right) {
    return BinaryExpression::Convert(ThreadContext::Context(), Position(), left.release(),
            SkSL::Operator::Kind::COMMA, DSLExpression(std::move(right)).release());
}

DSLPossibleExpression operator,(DSLPossibleExpression left, DSLPossibleExpression right) {
    return BinaryExpression::Convert(ThreadContext::Context(), Position(),
            DSLExpression(std::move(left)).release(), SkSL::Operator::Kind::COMMA,
            DSLExpression(std::move(right)).release());
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

void DSLPossibleExpression::reportErrors(Position pos) {
    SkASSERT(!this->valid());
    ThreadContext::ReportErrors(pos);
}

DSLType DSLPossibleExpression::type() const {
    if (!this->valid()) {
        return kVoid_Type;
    }
    return &fExpression->type();
}

std::string DSLPossibleExpression::description() const {
    SkASSERT(this->valid());
    return fExpression->description();
}

Position DSLPossibleExpression::position() const {
    SkASSERT(this->valid());
    return fExpression->fPosition;
}

DSLExpression DSLPossibleExpression::x(Position pos) {
    return DSLExpression(this->release()).x(pos);
}

DSLExpression DSLPossibleExpression::y(Position pos) {
    return DSLExpression(this->release()).y(pos);
}

DSLExpression DSLPossibleExpression::z(Position pos) {
    return DSLExpression(this->release()).z(pos);
}

DSLExpression DSLPossibleExpression::w(Position pos) {
    return DSLExpression(this->release()).w(pos);
}

DSLExpression DSLPossibleExpression::r(Position pos) {
    return DSLExpression(this->release()).r(pos);
}

DSLExpression DSLPossibleExpression::g(Position pos) {
    return DSLExpression(this->release()).g(pos);
}

DSLExpression DSLPossibleExpression::b(Position pos) {
    return DSLExpression(this->release()).b(pos);
}

DSLExpression DSLPossibleExpression::a(Position pos) {
    return DSLExpression(this->release()).a(pos);
}

DSLExpression DSLPossibleExpression::field(std::string_view name, Position pos) {
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

DSLPossibleExpression DSLPossibleExpression::operator()(SkTArray<DSLWrapper<DSLExpression>> args,
                                                        Position pos) {
    return DSLExpression(this->release())(std::move(args), pos);
}

DSLPossibleExpression DSLPossibleExpression::operator()(ExpressionArray args, Position pos) {
    return DSLExpression(this->release())(std::move(args), pos);
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

std::unique_ptr<SkSL::Expression> DSLPossibleExpression::release(Position pos) {
    return DSLExpression(std::move(*this), pos).release();
}

} // namespace dsl

} // namespace SkSL
