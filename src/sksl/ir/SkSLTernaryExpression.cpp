/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLTernaryExpression.h"

#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLOperator.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLConstructorScalarCast.h"
#include "src/sksl/ir/SkSLLiteral.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"

namespace SkSL {

std::unique_ptr<Expression> TernaryExpression::Convert(const Context& context,
                                                       Position pos,
                                                       std::unique_ptr<Expression> test,
                                                       std::unique_ptr<Expression> ifTrue,
                                                       std::unique_ptr<Expression> ifFalse) {
    test = context.fTypes.fBool->coerceExpression(std::move(test), context);
    if (!test || !ifTrue || !ifFalse) {
        return nullptr;
    }
    if (ifTrue->type().componentType().isOpaque()) {
        context.fErrors->error(pos, "ternary expression of opaque type '" +
                                    ifTrue->type().displayName() + "' is not allowed");
        return nullptr;
    }
    const Type* trueType;
    const Type* falseType;
    const Type* resultType;
    Operator equalityOp(Operator::Kind::EQEQ);
    if (!equalityOp.determineBinaryType(context, ifTrue->type(), ifFalse->type(),
                                        &trueType, &falseType, &resultType) ||
        !trueType->matches(*falseType)) {
        Position errorPos = ifTrue->fPosition.rangeThrough(ifFalse->fPosition);
        if (ifTrue->type().isVoid()) {
            context.fErrors->error(errorPos, "ternary expression of type 'void' is not allowed");
        } else {
            context.fErrors->error(errorPos, "ternary operator result mismatch: '" +
                                             ifTrue->type().displayName() + "', '" +
                                             ifFalse->type().displayName() + "'");
        }
        return nullptr;
    }
    if (trueType->isOrContainsArray()) {
        context.fErrors->error(pos, "ternary operator result may not be an array (or struct "
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
    return TernaryExpression::Make(context, pos, std::move(test), std::move(ifTrue),
                                   std::move(ifFalse));
}

std::unique_ptr<Expression> TernaryExpression::Make(const Context& context,
                                                    Position pos,
                                                    std::unique_ptr<Expression> test,
                                                    std::unique_ptr<Expression> ifTrue,
                                                    std::unique_ptr<Expression> ifFalse) {
    SkASSERT(ifTrue->type().matches(ifFalse->type()));
    SkASSERT(!ifTrue->type().componentType().isOpaque());
    SkASSERT(!context.fConfig->strictES2Mode() || !ifTrue->type().isOrContainsArray());

    const Expression* testExpr = ConstantFolder::GetConstantValueForVariable(*test);
    if (testExpr->isBoolLiteral()) {
        // static boolean test, just return one of the branches
        if (testExpr->as<Literal>().boolValue()) {
            ifTrue->fPosition = pos;
            return ifTrue;
        } else {
            ifFalse->fPosition = pos;
            return ifFalse;
        }
    }

    if (context.fConfig->fSettings.fOptimize) {
        const Expression* ifTrueExpr  = ConstantFolder::GetConstantValueForVariable(*ifTrue);
        const Expression* ifFalseExpr = ConstantFolder::GetConstantValueForVariable(*ifFalse);

        // A ternary with matching true- and false-cases does not need to branch.
        if (Analysis::IsSameExpressionTree(*ifTrueExpr, *ifFalseExpr)) {
            // If `test` has no side-effects, we can eliminate it too, and just return `ifTrue`.
            if (!Analysis::HasSideEffects(*test)) {
                ifTrue->fPosition = pos;
                return ifTrue;
            }
            // Return a comma-expression containing `(test, ifTrue)`.
            return BinaryExpression::Make(context, pos, std::move(test),
                                          Operator::Kind::COMMA, std::move(ifTrue));
        }

        // A ternary of the form `test ? expr : false` can be simplified to `test && expr`.
        if (ifFalseExpr->isBoolLiteral() && !ifFalseExpr->as<Literal>().boolValue()) {
            return BinaryExpression::Make(context, pos, std::move(test),
                                          Operator::Kind::LOGICALAND, std::move(ifTrue));
        }

        // A ternary of the form `test ? true : expr` can be simplified to `test || expr`.
        if (ifTrueExpr->isBoolLiteral() && ifTrueExpr->as<Literal>().boolValue()) {
            return BinaryExpression::Make(context, pos, std::move(test),
                                          Operator::Kind::LOGICALOR, std::move(ifFalse));
        }

        // A ternary of the form `test ? false : true` can be simplified to `!test`.
        if (ifTrueExpr->isBoolLiteral() && !ifTrueExpr->as<Literal>().boolValue() &&
            ifFalseExpr->isBoolLiteral() && ifFalseExpr->as<Literal>().boolValue()) {
            return PrefixExpression::Make(context, pos, Operator::Kind::LOGICALNOT,
                                          std::move(test));
        }

        // A ternary of the form `test ? 1 : 0` can be simplified to `cast(test)`.
        if (ifTrueExpr->is<Literal>() && ifTrueExpr->as<Literal>().value() == 1.0 &&
            ifFalseExpr->is<Literal>() && ifFalseExpr->as<Literal>().value() == 0.0) {
            return ConstructorScalarCast::Make(context, pos, ifTrue->type(), std::move(test));
        }
    }

    return std::make_unique<TernaryExpression>(pos, std::move(test), std::move(ifTrue),
                                               std::move(ifFalse));
}

std::string TernaryExpression::description(OperatorPrecedence parentPrecedence) const {
    bool needsParens = (OperatorPrecedence::kTernary >= parentPrecedence);
    return std::string(needsParens ? "(" : "") +
           this->test()->description(OperatorPrecedence::kTernary) + " ? " +
           this->ifTrue()->description(OperatorPrecedence::kTernary) + " : " +
           this->ifFalse()->description(OperatorPrecedence::kTernary) +
           std::string(needsParens ? ")" : "");
}

}  // namespace SkSL
