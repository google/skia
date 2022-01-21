/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

bool Expression::isIncomplete(const Context& context) const {
    switch (this->kind()) {
        case Kind::kFunctionReference:
        case Kind::kExternalFunctionReference:
            context.fErrors->error(fLine, "expected '(' to begin function call");
            return true;

        case Kind::kMethodReference:
            context.fErrors->error(fLine, "expected '(' to begin method call");
            return true;

        case Kind::kTypeReference:
            context.fErrors->error(fLine, "expected '(' to begin constructor invocation");
            return true;

        default:
            return false;
    }
}

ExpressionArray ExpressionArray::clone() const {
    ExpressionArray cloned;
    cloned.reserve_back(this->count());
    for (const std::unique_ptr<Expression>& expr : *this) {
        cloned.push_back(expr ? expr->clone() : nullptr);
    }
    return cloned;
}

}  // namespace SkSL
