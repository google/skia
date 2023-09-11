/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLExpressionStatement.h"

#include "include/core/SkTypes.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLOperator.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLVariableReference.h"

namespace SkSL {

std::unique_ptr<Statement> ExpressionStatement::Convert(const Context& context,
                                                        std::unique_ptr<Expression> expr) {
    // Expression-statements need to represent a complete expression.
    // Report an error on intermediate expressions, like FunctionReference or TypeReference.
    if (expr->isIncomplete(context)) {
        return nullptr;
    }
    return ExpressionStatement::Make(context, std::move(expr));
}

std::unique_ptr<Statement> ExpressionStatement::Make(const Context& context,
                                                     std::unique_ptr<Expression> expr) {
    SkASSERT(!expr->isIncomplete(context));

    if (context.fConfig->fSettings.fOptimize) {
        // Expression-statements without any side effect can be replaced with a Nop.
        if (!Analysis::HasSideEffects(*expr)) {
            return Nop::Make();
        }

        // If this is an assignment statement like `a += b;`, the ref-kind of `a` will be set as
        // read-write; `a` is written-to by the +=, and read-from by the consumer of the expression.
        // We can demote the ref-kind to "write" safely, because the result of the expression is
        // discarded; that is, `a` is never actually read-from.
        if (expr->is<BinaryExpression>()) {
            BinaryExpression& binary = expr->as<BinaryExpression>();
            if (VariableReference* assignedVar = binary.isAssignmentIntoVariable()) {
                if (assignedVar->refKind() == VariableRefKind::kReadWrite) {
                    assignedVar->setRefKind(VariableRefKind::kWrite);
                }
            }
        }
    }

    return std::make_unique<ExpressionStatement>(std::move(expr));
}

std::string ExpressionStatement::description() const {
    return this->expression()->description(OperatorPrecedence::kStatement) + ";";
}

}  // namespace SkSL
