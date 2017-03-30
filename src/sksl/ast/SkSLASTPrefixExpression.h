/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTPREFIXEXPRESSION
#define SKSL_ASTPREFIXEXPRESSION

#include "SkSLASTExpression.h"
#include "../SkSLToken.h"

namespace SkSL {

/**
 * An expression modified by a unary operator appearing in front of it, such as '-x' or '!inside'.
 */
struct ASTPrefixExpression : public ASTExpression {
    ASTPrefixExpression(Token op, std::unique_ptr<ASTExpression> operand)
    : INHERITED(op.fPosition, kPrefix_Kind)
    , fOperator(op.fKind)
    , fOperand(std::move(operand)) {}

    String description() const override {
        return Token::OperatorName(fOperator) + fOperand->description();
    }

    const Token::Kind fOperator;
    const std::unique_ptr<ASTExpression> fOperand;

    typedef ASTExpression INHERITED;
};

} // namespace

#endif
