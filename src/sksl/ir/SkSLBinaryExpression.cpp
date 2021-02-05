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

std::unique_ptr<Expression> BinaryExpression::constantPropagate(const IRGenerator& irGenerator,
                                                                const DefinitionMap& definitions) {
    // Transform self-assignment expressions (`x = x`) into just `x`.
    if (this->getOperator() == Token::Kind::TK_EQ &&
        this->left()->is<VariableReference>() &&
        this->right()->is<VariableReference>() &&
        (this->left()->as<VariableReference>().variable() ==
         this->right()->as<VariableReference>().variable())) {
        return this->right()->clone();
    }

    // Attempt to constant-fold this expression.
    return ConstantFolder::Simplify(irGenerator.fContext, *this->left(), this->getOperator(),
                                    *this->right());
}

}  // namespace SkSL
