/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLPostfixExpression.h"

#include "include/core/SkTypes.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLOperator.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVariableReference.h"

namespace SkSL {

std::unique_ptr<Expression> PostfixExpression::Convert(const Context& context,
                                                       Position pos,
                                                       std::unique_ptr<Expression> base,
                                                       Operator op) {
    if (base->type().isArray() || !base->type().componentType().isNumber()) {
        context.fErrors->error(pos, "'" + std::string(op.tightOperatorName()) +
                                    "' cannot operate on '" + base->type().displayName() + "'");
        return nullptr;
    }
    if (!Analysis::UpdateVariableRefKind(base.get(), VariableRefKind::kReadWrite,
                                         context.fErrors)) {
        return nullptr;
    }
    return PostfixExpression::Make(context, pos, std::move(base), op);
}

std::unique_ptr<Expression> PostfixExpression::Make(const Context& context,
                                                    Position pos,
                                                    std::unique_ptr<Expression> base,
                                                    Operator op) {
    SkASSERT(!base->type().isArray() && base->type().componentType().isNumber());
    SkASSERT(Analysis::IsAssignable(*base));
    return std::make_unique<PostfixExpression>(pos, std::move(base), op);
}

std::string PostfixExpression::description(OperatorPrecedence parentPrecedence) const {
    bool needsParens = (OperatorPrecedence::kPostfix >= parentPrecedence);
    return std::string(needsParens ? "(" : "") +
           this->operand()->description(OperatorPrecedence::kPostfix) +
           std::string(this->getOperator().tightOperatorName()) +
           std::string(needsParens ? ")" : "");
}

}  // namespace SkSL
