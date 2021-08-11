/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLOperators.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLBoolLiteral.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"

namespace SkSL {

std::unique_ptr<Expression> TernaryExpression::Convert(const Context& context,
                                                       std::unique_ptr<Expression> test,
                                                       std::unique_ptr<Expression> ifTrue,
                                                       std::unique_ptr<Expression> ifFalse) {
    test = context.fTypes.fBool->coerceExpression(std::move(test), context);
    if (!test || !ifTrue || !ifFalse) {
        return nullptr;
    }
    int offset = test->fOffset;
    const Type* trueType;
    const Type* falseType;
    const Type* resultType;
    Operator equalityOp(Token::Kind::TK_EQEQ);
    if (!equalityOp.determineBinaryType(context, ifTrue->type(), ifFalse->type(),
                                        &trueType, &falseType, &resultType) ||
        (*trueType != *falseType)) {
        context.errors().error(offset, "ternary operator result mismatch: '" +
                                       ifTrue->type().displayName() + "', '" +
                                       ifFalse->type().displayName() + "'");
        return nullptr;
    }
    if (trueType->componentType().isOpaque()) {
        context.errors().error(offset, "ternary expression of opaque type '" +
                                       trueType->displayName() + "' not allowed");
        return nullptr;
    }
    if (context.fConfig->strictES2Mode() && trueType->isOrContainsArray()) {
        context.errors().error(offset, "ternary operator result may not be an array (or struct "
                                       "containing an array)");
        return nullptr;
    }
    ifTrue = trueType->coerceExpression(std::move(ifTrue), context);
    if (!ifTrue) {
        return nullptr;
    }
    ifFalse = falseType->coerceExpression(std::move(ifFalse), context);
    if (!ifFalse) {
        return nullptr;
    }
    return TernaryExpression::Make(context, std::move(test), std::move(ifTrue), std::move(ifFalse));
}

std::unique_ptr<Expression> TernaryExpression::Make(const Context& context,
                                                    std::unique_ptr<Expression> test,
                                                    std::unique_ptr<Expression> ifTrue,
                                                    std::unique_ptr<Expression> ifFalse) {
    SkASSERT(ifTrue->type() == ifFalse->type());
    SkASSERT(!ifTrue->type().componentType().isOpaque());
    SkASSERT(!context.fConfig->strictES2Mode() || !ifTrue->type().isOrContainsArray());

    if (context.fConfig->fSettings.fOptimize) {
        const Expression* testExpr = ConstantFolder::GetConstantValueForVariable(*test);
        if (testExpr->is<BoolLiteral>()) {
            // static boolean test, just return one of the branches
            return testExpr->as<BoolLiteral>().value() ? std::move(ifTrue)
                                                       : std::move(ifFalse);
        }
    }

    return std::make_unique<TernaryExpression>(test->fOffset, std::move(test),
                                               std::move(ifTrue), std::move(ifFalse));
}

}  // namespace SkSL
