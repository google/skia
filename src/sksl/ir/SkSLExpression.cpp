/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLExpression.h"

#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLDefines.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLOperator.h"

namespace SkSL {

std::string Expression::description() const {
    return this->description(OperatorPrecedence::kExpression);
}

bool Expression::isIncomplete(const Context& context) const {
    switch (this->kind()) {
        case Kind::kFunctionReference:
            context.fErrors->error(fPosition.after(), "expected '(' to begin function call");
            return true;

        case Kind::kMethodReference:
            context.fErrors->error(fPosition.after(), "expected '(' to begin method call");
            return true;

        case Kind::kTypeReference:
            context.fErrors->error(fPosition.after(),
                                   "expected '(' to begin constructor invocation");
            return true;

        case Kind::kVariableReference:
            if (this->type().matches(*context.fTypes.fSkCaps)) {
                context.fErrors->error(fPosition, "invalid expression");
                return true;
            }
            return false;

        default:
            return false;
    }
}

ExpressionArray ExpressionArray::clone() const {
    ExpressionArray cloned;
    cloned.reserve_exact(this->size());
    for (const std::unique_ptr<Expression>& expr : *this) {
        cloned.push_back(expr ? expr->clone() : nullptr);
    }
    return cloned;
}

}  // namespace SkSL
