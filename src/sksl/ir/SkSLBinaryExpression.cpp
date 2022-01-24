/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLLiteral.h"
#include "src/sksl/ir/SkSLSetting.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVariableReference.h"

namespace SkSL {

static bool is_low_precision_matrix_vector_multiply(const Expression& left,
                                                    const Operator& op,
                                                    const Expression& right,
                                                    const Type& resultType) {
    return !resultType.highPrecision() &&
           op.kind() == Token::Kind::TK_STAR &&
           left.type().isMatrix() &&
           right.type().isVector() &&
           left.type().rows() == right.type().columns() &&
           Analysis::IsTrivialExpression(left) &&
           Analysis::IsTrivialExpression(right);
}

static std::unique_ptr<Expression> rewrite_matrix_vector_multiply(const Context& context,
                                                                  const Expression& left,
                                                                  const Operator& op,
                                                                  const Expression& right,
                                                                  const Type& resultType) {
    // Rewrite m33 * v3 as (m[0] * v[0] + m[1] * v[1] + m[2] * v[2])
    std::unique_ptr<Expression> sum;
    for (int n = 0; n < left.type().rows(); ++n) {
        // Get mat[N] with an index expression.
        std::unique_ptr<Expression> matN = IndexExpression::Make(
                context, left.clone(), Literal::MakeInt(context, left.fLine, n));
        // Get vec[N] with a swizzle expression.
        std::unique_ptr<Expression> vecN = Swizzle::Make(
                context, right.clone(), ComponentArray{(SkSL::SwizzleComponent::Type)n});
        // Multiply them together.
        const Type* matNType = &matN->type();
        std::unique_ptr<Expression> product =
                BinaryExpression::Make(context, std::move(matN), op, std::move(vecN), matNType);
        // Sum all the components together.
        if (!sum) {
            sum = std::move(product);
        } else {
            sum = BinaryExpression::Make(context,
                                         std::move(sum),
                                         Operator(Token::Kind::TK_PLUS),
                                         std::move(product),
                                         matNType);
        }
    }

    return sum;
}

std::unique_ptr<Expression> BinaryExpression::Convert(const Context& context,
                                                      std::unique_ptr<Expression> left,
                                                      Operator op,
                                                      std::unique_ptr<Expression> right) {
    if (!left || !right) {
        return nullptr;
    }
    const int line = left->fLine;

    const Type* rawLeftType = (left->isIntLiteral() && right->type().isInteger())
            ? &right->type()
            : &left->type();
    const Type* rawRightType = (right->isIntLiteral() && left->type().isInteger())
            ? &left->type()
            : &right->type();

    bool isAssignment = op.isAssignment();
    if (isAssignment &&
        !Analysis::UpdateVariableRefKind(left.get(),
                                         op.kind() != Token::Kind::TK_EQ
                                                 ? VariableReference::RefKind::kReadWrite
                                                 : VariableReference::RefKind::kWrite,
                                         context.fErrors)) {
        return nullptr;
    }

    const Type* leftType;
    const Type* rightType;
    const Type* resultType;
    if (!op.determineBinaryType(context, *rawLeftType, *rawRightType,
                                &leftType, &rightType, &resultType)) {
        context.fErrors->error(line, String("type mismatch: '") + op.tightOperatorName() +
                                     "' cannot operate on '" + left->type().displayName() +
                                     "', '" + right->type().displayName() + "'");
        return nullptr;
    }

    if (isAssignment && leftType->componentType().isOpaque()) {
        context.fErrors->error(line, "assignments to opaque type '" + left->type().displayName() +
                                     "' are not permitted");
        return nullptr;
    }
    if (context.fConfig->strictES2Mode()) {
        if (!op.isAllowedInStrictES2Mode()) {
            context.fErrors->error(line, String("operator '") + op.tightOperatorName() +
                                         "' is not allowed");
            return nullptr;
        }
        if (leftType->isOrContainsArray()) {
            // Most operators are already rejected on arrays, but GLSL ES 1.0 is very explicit that
            // the *only* operator allowed on arrays is subscripting (and the rules against
            // assignment, comparison, and even sequence apply to structs containing arrays as well)
            context.fErrors->error(line, String("operator '") + op.tightOperatorName() + "' can "
                                         "not operate on arrays (or structs containing arrays)");
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

    // For simple assignments, detect and report out-of-range literal values.
    if (op.kind() == Token::Kind::TK_EQ) {
        left->type().checkForOutOfRangeLiteral(context, *right);
    }

    // Perform constant-folding on the expression.
    const int line = left->fLine;
    if (std::unique_ptr<Expression> result = ConstantFolder::Simplify(context, line, *left,
                                                                      op, *right, *resultType)) {
        return result;
    }

    if (context.fConfig->fSettings.fOptimize) {
        // When sk_Caps.rewriteMatrixVectorMultiply is set, we rewrite medium-precision
        // matrix * vector multiplication as:
        //   (sk_Caps.rewriteMatrixVectorMultiply ? (mat[0]*vec[0] + ... + mat[N]*vec[N])
        //                                        : mat * vec)
        if (is_low_precision_matrix_vector_multiply(*left, op, *right, *resultType)) {
            // Look up `sk_Caps.rewriteMatrixVectorMultiply`.
            auto caps = Setting::Convert(context, line, "rewriteMatrixVectorMultiply");

            bool capsBitIsTrue = caps->isBoolLiteral() && caps->as<Literal>().boolValue();
            if (capsBitIsTrue || !caps->isBoolLiteral()) {
                // Rewrite the multiplication as a sum of vector-scalar products.
                std::unique_ptr<Expression> rewrite =
                        rewrite_matrix_vector_multiply(context, *left, op, *right, *resultType);

                // If we know the caps bit is true, return the rewritten expression directly.
                if (capsBitIsTrue) {
                    return rewrite;
                }

                // Return a ternary expression:
                //     sk_Caps.rewriteMatrixVectorMultiply ? (rewrite) : (mat * vec)
                return TernaryExpression::Make(
                        context,
                        std::move(caps),
                        std::move(rewrite),
                        std::make_unique<BinaryExpression>(line, std::move(left), op,
                                                           std::move(right), resultType));
            }
        }
    }

    return std::make_unique<BinaryExpression>(line, std::move(left), op,
                                              std::move(right), resultType);
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
    return std::make_unique<BinaryExpression>(fLine,
                                              this->left()->clone(),
                                              this->getOperator(),
                                              this->right()->clone(),
                                              &this->type());
}

String BinaryExpression::description() const {
    return "(" + this->left()->description() +
                 this->getOperator().operatorName() +
                 this->right()->description() + ")";
}

}  // namespace SkSL
