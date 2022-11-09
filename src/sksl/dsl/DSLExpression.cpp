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

#include <utility>

namespace SkSL {

namespace dsl {

DSLExpression::DSLExpression() {}

DSLExpression::DSLExpression(DSLExpression&& other)
    : fExpression(std::move(other.fExpression)) {}

DSLExpression::DSLExpression(std::unique_ptr<SkSL::Expression> expression, Position pos)
    : fExpression(expression ? std::move(expression)
                             : SkSL::Poison::Make(pos, ThreadContext::Context())) {
    // If a position was passed in, it must match the expression's position.
    SkASSERTF(!pos.valid() || this->position() == pos,
              "expected expression position (%d-%d), but received (%d-%d)",
              pos.startOffset(), pos.endOffset(),
              this->position().startOffset(), this->position().endOffset());
}

DSLExpression::DSLExpression(float value, Position pos)
    : fExpression(SkSL::Literal::MakeFloat(ThreadContext::Context(),
                                           pos,
                                           value)) {}

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

DSLExpression::DSLExpression(DSLVarBase& var, Position pos)
    : fExpression(std::make_unique<SkSL::VariableReference>(
                  pos, DSLWriter::Var(var), SkSL::VariableReference::RefKind::kRead)) {}

DSLExpression::DSLExpression(DSLVarBase&& var, Position pos)
    : DSLExpression(var) {}

DSLExpression::~DSLExpression() {}

DSLExpression DSLExpression::Poison(Position pos) {
    return DSLExpression(SkSL::Poison::Make(pos, ThreadContext::Context()));
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

DSLExpression DSLExpression::assign(DSLExpression right) {
    Position pos = this->position().rangeThrough(right.position());
    return DSLExpression(BinaryExpression::Convert(ThreadContext::Context(), pos, this->release(),
                                                   SkSL::Operator::Kind::EQ, right.release()));
}

DSLExpression DSLExpression::operator[](DSLExpression right) {
    Position pos = this->position().rangeThrough(right.position());
    return DSLExpression(IndexExpression::Convert(ThreadContext::Context(),
                                                  *ThreadContext::SymbolTable(), pos,
                                                  this->release(), right.release()));
}

DSLExpression DSLExpression::index(DSLExpression index, Position pos) {
    std::unique_ptr<SkSL::Expression> result = IndexExpression::Convert(ThreadContext::Context(),
            *ThreadContext::SymbolTable(), pos, this->release(), index.release());
    return DSLExpression(std::move(result), pos);
}

DSLExpression DSLExpression::operator()(SkTArray<DSLExpression> args, Position pos) {
    ExpressionArray converted;
    converted.reserve_back(args.size());
    for (DSLExpression& arg : args) {
        converted.push_back(arg.release());
    }
    return (*this)(std::move(converted), pos);
}

DSLExpression DSLExpression::operator()(ExpressionArray args, Position pos) {
    return DSLExpression(SkSL::FunctionCall::Convert(ThreadContext::Context(), pos, this->release(),
                                                     std::move(args)), pos);
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

#define OP(op, token)                                                        \
DSLExpression operator op(DSLExpression left, DSLExpression right) {         \
    return DSLExpression(BinaryExpression::Convert(ThreadContext::Context(), \
                                                   Position(),               \
                                                   left.release(),           \
                                                   Operator::Kind::token,    \
                                                   right.release()));        \
}

#define PREFIXOP(op, token)                                                  \
DSLExpression operator op(DSLExpression expr) {                              \
    return DSLExpression(PrefixExpression::Convert(ThreadContext::Context(), \
                                                   Position(),               \
                                                   Operator::Kind::token,    \
                                                   expr.release()));         \
}

#define POSTFIXOP(op, token)                                                  \
DSLExpression operator op(DSLExpression expr, int) {                          \
    return DSLExpression(PostfixExpression::Convert(ThreadContext::Context(), \
                                                    Position(),               \
                                                    expr.release(),           \
                                                    Operator::Kind::token));  \
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
DSLExpression LogicalXor(DSLExpression left, DSLExpression right) {
    return DSLExpression(BinaryExpression::Convert(ThreadContext::Context(),
                                                   Position(),
                                                   left.release(),
                                                   SkSL::Operator::Kind::LOGICALXOR,
                                                   right.release()));
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

DSLExpression operator,(DSLExpression left, DSLExpression right) {
    return DSLExpression(BinaryExpression::Convert(ThreadContext::Context(),
                                                   Position(),
                                                   left.release(),
                                                   SkSL::Operator::Kind::COMMA,
                                                   right.release()));
}

} // namespace dsl

} // namespace SkSL
