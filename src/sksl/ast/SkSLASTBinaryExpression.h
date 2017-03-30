/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTBINARYEXPRESSION
#define SKSL_ASTBINARYEXPRESSION

#include "SkSLASTExpression.h"
#include "../SkSLToken.h"

namespace SkSL {

/**
 * Represents a binary operation, with the operator represented by the token's type.
 */
struct ASTBinaryExpression : public ASTExpression {
    ASTBinaryExpression(std::unique_ptr<ASTExpression> left, Token op,
                        std::unique_ptr<ASTExpression> right)
    : INHERITED(op.fPosition, kBinary_Kind)
    , fLeft(std::move(left))
    , fOperator(op.fKind)
    , fRight(std::move(right)) {}

    String description() const override {
        return "(" + fLeft->description() + " " + Token::OperatorName(fOperator) + " " +
               fRight->description() + ")";
    }

    const std::unique_ptr<ASTExpression> fLeft;
    const Token::Kind fOperator;
    const std::unique_ptr<ASTExpression> fRight;

    typedef ASTExpression INHERITED;
};

} // namespace

#endif
