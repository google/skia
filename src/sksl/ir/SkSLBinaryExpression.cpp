/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLBinaryExpression.h"

#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/ir/SkSLVariableReference.h"

namespace SkSL {

using RefKind = VariableReference::RefKind;

bool BinaryExpression::IsVariableReferenceWritable(const Expression& expr) {
    switch (expr.kind()) {
        case Expression::Kind::kFieldAccess:
            return IsVariableReferenceWritable(*expr.as<FieldAccess>().base());

        case Expression::Kind::kIndex:
            return IsVariableReferenceWritable(*expr.as<IndexExpression>().base());

        case Expression::Kind::kSwizzle:
            return IsVariableReferenceWritable(*expr.as<Swizzle>().base());

        case Expression::Kind::kTernary: {
            const TernaryExpression& t = expr.as<TernaryExpression>();
            return IsVariableReferenceWritable(*t.ifTrue()) &&
                   IsVariableReferenceWritable(*t.ifFalse());
        }
        case Expression::Kind::kVariableReference: {
            const VariableReference& ref = expr.as<VariableReference>();
            return ref.refKind() == RefKind::kWrite ||
                   ref.refKind() == RefKind::kReadWrite;
        }
        default:
            return false;
    }
}

static std::unique_ptr<Expression> make_compound_assignment(const BinaryExpression& expr) {
    // Check for expressions of the form `(expr) = binary-expression`.
    if (expr.getOperator() != Token::Kind::TK_EQ || !expr.right()->is<BinaryExpression>()) {
        return nullptr;
    }

    // Check that the right-side op is not currently assignment, but does have an assignment form.
    const BinaryExpression& rightExpr = expr.right()->as<BinaryExpression>();
    Token::Kind rightOp = rightExpr.getOperator();
    Token::Kind assignmentOp = Operators::AddAssignment(rightOp);
    if (Operators::IsAssignment(rightOp) || assignmentOp == Token::Kind::TK_INVALID) {
        return nullptr;
    }

    // Now we've established that our expression is of the form `exprA = exprB OP exprC` and there
    // is a suitable compound OP we can use. Finally, we need to confirm that `exprA` and `exprB`
    // are actually equivalent expression trees (same variable reference/field access/array index).
    if (!Analysis::IsMatchingExpressionTree(*expr.left(), *rightExpr.left())) {
        return nullptr;
    }

    // Create a new binary compound-expression matching the original expression's intent.
    std::unique_ptr<Expression> leftSide = rightExpr.left()->clone();
    std::unique_ptr<Expression> rightSide = rightExpr.right()->clone();

    Analysis::UpdateRefKind(leftSide.get(), RefKind::kReadWrite);

    return std::make_unique<BinaryExpression>(expr.fOffset,
                                              std::move(leftSide),
                                              assignmentOp,
                                              std::move(rightSide),
                                              &expr.type());
}

std::unique_ptr<Expression> BinaryExpression::constantPropagate(const IRGenerator& irGenerator,
                                                                const DefinitionMap& definitions) {
    // Attempt to constant-fold this expression.
    if (std::unique_ptr<Expression> fold = ConstantFolder::Simplify(irGenerator.fContext, fOffset,
                                                                    *this->left(),
                                                                    this->getOperator(),
                                                                    *this->right())) {
        return fold;
    }

    // In the IR Generator, we expand out `x *= expr` into `x = x * expr` to allow these expressions
    // to constant-propagate. If we've gotten this far and we still have `x = x * expr`, we should
    // convert it back into `x *= expr.` (If the original code actually contained `x = x * expr`, we
    // will also rewrite that, but this is harmless.)
    if (std::unique_ptr<Expression> compound = make_compound_assignment(*this)) {
        return compound;
    }

    return nullptr;
}

}  // namespace SkSL
