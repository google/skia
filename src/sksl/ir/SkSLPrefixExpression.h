/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_PREFIXEXPRESSION
#define SKSL_PREFIXEXPRESSION

#include "SkSLExpression.h"
#include "SkSLToken.h"

namespace SkSL {

/**
 * An expression modified by a unary operator appearing before it, such as '!flag'.
 */
struct PrefixExpression : public Expression {
    PrefixExpression(Token::Kind op, std::unique_ptr<Expression> operand)
    : INHERITED(operand->fPosition, kPrefix_Kind, operand->fType)
    , fOperand(std::move(operand))
    , fOperator(op) {}

    virtual String description() const override {
        return Token::OperatorName(fOperator) + fOperand->description();
    }

    std::unique_ptr<Expression> fOperand;
    const Token::Kind fOperator;

    typedef Expression INHERITED;
};

} // namespace

#endif
