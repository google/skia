/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLBinaryExpression.h"

namespace SkSL {

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
            return ref.refKind() == VariableReference::RefKind::kWrite ||
                   ref.refKind() == VariableReference::RefKind::kReadWrite;
        }
        default:
            return false;
    }
}

static bool is_matching_expression_tree(const Expression& left, const Expression& right) {
    // This only needs to check for trees that can plausibly terminate in a variable, so some basic
    // candidates like `FloatLiteral` are missing.
    if (left.kind() != right.kind() || left.type() != right.type()) {
        return false;
    }

    switch (left.kind()) {
        case Expression::Kind::kIntLiteral:
            return left.as<IntLiteral>().value() == right.as<IntLiteral>().value();

        case Expression::Kind::kFieldAccess:
            return left.as<FieldAccess>().fieldIndex() == right.as<FieldAccess>().fieldIndex() &&
                   is_matching_expression_tree(*left.as<FieldAccess>().base(),
                                               *right.as<FieldAccess>().base());

        case Expression::Kind::kIndex:
            return is_matching_expression_tree(*left.as<IndexExpression>().index(),
                                               *right.as<IndexExpression>().index()) &&
                   is_matching_expression_tree(*left.as<IndexExpression>().base(),
                                               *right.as<IndexExpression>().base());

        case Expression::Kind::kSwizzle:
            return left.as<Swizzle>().components() == right.as<Swizzle>().components() &&
                   is_matching_expression_tree(*left.as<Swizzle>().base(),
                                               *right.as<Swizzle>().base());

        case Expression::Kind::kVariableReference:
            return left.as<VariableReference>().variable() ==
                   right.as<VariableReference>().variable();

        default:
            return false;
    }
}

std::unique_ptr<Expression> BinaryExpression::constantPropagate(const IRGenerator& irGenerator,
                                                                const DefinitionMap& definitions) {
    // Transform self-assignment expressions (`x = x`) into just `x`.
    if (this->getOperator() == Token::Kind::TK_EQ &&
        is_matching_expression_tree(*this->left(), *this->right())) {
        return this->right()->clone();
    }

    // Attempt to constant-fold this expression.
    return ConstantFolder::Simplify(irGenerator.fContext, *this->left(), this->getOperator(),
                                    *this->right());
}

}  // namespace SkSL
