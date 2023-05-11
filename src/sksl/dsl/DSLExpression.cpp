/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/DSLExpression.h"

#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "src/sksl/SkSLOperator.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/dsl/DSLType.h"
#include "src/sksl/dsl/DSLVar.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLLiteral.h"
#include "src/sksl/ir/SkSLPoison.h"
#include "src/sksl/ir/SkSLVariableReference.h"

#include <utility>

using namespace skia_private;

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

DSLType DSLExpression::type() const {
    if (!this->hasValue()) {
        return DSLType::Void();
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

DSLExpression DSLExpression::assign(DSLExpression right) {
    Position pos = this->position().rangeThrough(right.position());
    return DSLExpression(BinaryExpression::Convert(ThreadContext::Context(), pos, this->release(),
                                                   SkSL::Operator::Kind::EQ, right.release()));
}

} // namespace dsl
} // namespace SkSL
