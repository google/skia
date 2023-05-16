/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/DSLExpression.h"

#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/dsl/DSLType.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLPoison.h"

#include <utility>

using namespace skia_private;

namespace SkSL {
namespace dsl {

DSLExpression::DSLExpression(std::unique_ptr<SkSL::Expression> expression, Position pos)
        : fExpression(expression ? std::move(expression)
                                 : SkSL::Poison::Make(pos, ThreadContext::Context())) {
    // If a position was passed in, it must match the expression's position.
    SkASSERTF(!pos.valid() || this->position() == pos,
              "expected expression position (%d-%d), but received (%d-%d)",
              pos.startOffset(), pos.endOffset(),
              this->position().startOffset(), this->position().endOffset());
}

DSLExpression DSLExpression::Poison(Position pos) {
    return DSLExpression(SkSL::Poison::Make(pos, ThreadContext::Context()));
}

bool DSLExpression::isValid() const {
    return this->hasValue() && !fExpression->is<SkSL::Poison>();
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

} // namespace dsl
} // namespace SkSL
