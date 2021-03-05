/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

std::unique_ptr<Expression> BinaryExpression::Convert(const Context& context,
                                                      std::unique_ptr<Expression> left,
                                                      Operator op,
                                                      std::unique_ptr<Expression> right) {
    if (!left || !right) {
        return nullptr;
    }
    const int offset = left->fOffset;

    const Type* rawLeftType;
    if (left->is<IntLiteral>() && right->type().isInteger()) {
        rawLeftType = &right->type();
    } else {
        rawLeftType = &left->type();
    }

    const Type* rawRightType;
    if (right->is<IntLiteral>() && left->type().isInteger()) {
        rawRightType = &left->type();
    } else {
        rawRightType = &right->type();
    }

    bool isAssignment = op.isAssignment();
    if (isAssignment &&
        !Analysis::MakeAssignmentExpr(left.get(),
                                      op.kind() != Token::Kind::TK_EQ
                                              ? VariableReference::RefKind::kReadWrite
                                              : VariableReference::RefKind::kWrite,
                                      &context.fErrors)) {
        return nullptr;
    }

    const Type* leftType;
    const Type* rightType;
    const Type* resultType;
    if (!op.determineBinaryType(context, *rawLeftType, *rawRightType,
                                &leftType, &rightType, &resultType)) {
        context.fErrors.error(offset, String("type mismatch: '") + op.operatorName() +
                                      "' cannot operate on '" + left->type().displayName() +
                                      "', '" + right->type().displayName() + "'");
        return nullptr;
    }

    if (isAssignment && leftType->componentType().isOpaque()) {
        context.fErrors.error(offset, "assignments to opaque type '" + left->type().displayName() +
                                      "' are not permitted");
        return nullptr;
    }
    if (context.fConfig->strictES2Mode()) {
        if (!op.isAllowedInStrictES2Mode()) {
            context.fErrors.error(offset, String("operator '") + op.operatorName() +
                                          "' is not allowed");
            return nullptr;
        }
        if (leftType->isOrContainsArray()) {
            // Most operators are already rejected on arrays, but GLSL ES 1.0 is very explicit that
            // the *only* operator allowed on arrays is subscripting (and the rules against
            // assignment, comparison, and even sequence apply to structs containing arrays as well)
            context.fErrors.error(offset, String("operator '") + op.operatorName() + "' can not "
                                          "operate on arrays (or structs containing arrays)");
            return nullptr;
        }
    }

    left = leftType->coerceExpression(std::move(left), context);
    right = rightType->coerceExpression(std::move(right), context);
    if (!left || !right) {
        return nullptr;
    }

    return BinaryExpression::Make(context, std::move(left), op, std::move(right), resultType);
}

std::unique_ptr<Expression> BinaryExpression::Make(const Context& context,
                                                   std::unique_ptr<Expression> left,
                                                   Operator op,
                                                   std::unique_ptr<Expression> right) {
    // Determine the result type of the binary expression.
    const Type* leftType;
    const Type* rightType;
    const Type* resultType;
    SkAssertResult(op.determineBinaryType(context, left->type(), right->type(),
                                          &leftType, &rightType, &resultType));

    return BinaryExpression::Make(context, std::move(left), op, std::move(right), resultType);
}

std::unique_ptr<Expression> BinaryExpression::Make(const Context& context,
                                                   std::unique_ptr<Expression> left,
                                                   Operator op,
                                                   std::unique_ptr<Expression> right,
                                                   const Type* resultType) {
    // We should have detected non-ES2 compliant behavior in Convert.
    SkASSERT(!context.fConfig->strictES2Mode() || op.isAllowedInStrictES2Mode());
    SkASSERT(!context.fConfig->strictES2Mode() || !left->type().isOrContainsArray());

    // We should have detected non-assignable assignment expressions in Convert.
    SkASSERT(!op.isAssignment() || Analysis::IsAssignable(*left));
    SkASSERT(!op.isAssignment() || !left->type().componentType().isOpaque());

    // If we can detect division-by-zero, we should synthesize an error, but our caller is still
    // expecting to receive a binary expression back; don't return nullptr.
    const int offset = left->fOffset;
    if (!ConstantFolder::ErrorOnDivideByZero(context, offset, op, *right)) {
        std::unique_ptr<Expression> result = ConstantFolder::Simplify(context, offset, *left,
                                                                      op, *right, *resultType);
        if (result) {
            return result;
        }
    }

    return std::make_unique<BinaryExpression>(offset, std::move(left), op, std::move(right),
                                              resultType);
}

bool BinaryExpression::CheckRef(const Expression& expr) {
    switch (expr.kind()) {
        case Expression::Kind::kFieldAccess:
            return CheckRef(*expr.as<FieldAccess>().base());

        case Expression::Kind::kIndex:
            return CheckRef(*expr.as<IndexExpression>().base());

        case Expression::Kind::kSwizzle:
            return CheckRef(*expr.as<Swizzle>().base());

        case Expression::Kind::kTernary: {
            const TernaryExpression& t = expr.as<TernaryExpression>();
            return CheckRef(*t.ifTrue()) && CheckRef(*t.ifFalse());
        }
        case Expression::Kind::kVariableReference: {
            const VariableReference& ref = expr.as<VariableReference>();
            return ref.refKind() == VariableRefKind::kWrite ||
                   ref.refKind() == VariableRefKind::kReadWrite;
        }
        default:
            return false;
    }
}

std::unique_ptr<Expression> BinaryExpression::clone() const {
    return std::make_unique<BinaryExpression>(fOffset,
                                              this->left()->clone(),
                                              this->getOperator(),
                                              this->right()->clone(),
                                              &this->type());
}

String BinaryExpression::description() const {
    return "(" + this->left()->description() +
           " " + this->getOperator().operatorName() +
           " " + this->right()->description() + ")";
}

}  // namespace SkSL
