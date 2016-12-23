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
    ASTPrefixExpression(Token op, sk_up<ASTExpression> operand)
            : INHERITED(op.fPosition, kPrefix_Kind)
            , fOperator(op.fKind)
            , fOperand(std::move(operand)) {}

    SkString description() const override {
        return Token::OperatorName(fOperator) + fOperand->description();
    }

    const Token::Kind fOperator;
    const sk_up<ASTExpression> fOperand;

    typedef ASTExpression INHERITED;
};

} // namespace

#endif
