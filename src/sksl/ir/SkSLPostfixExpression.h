/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_POSTFIXEXPRESSION
#define SKSL_POSTFIXEXPRESSION

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLLexer.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * An expression modified by a unary operator appearing after it, such as 'i++'.
 */
struct PostfixExpression : public Expression {
    PostfixExpression(std::unique_ptr<Expression> operand, Token::Kind op)
    : INHERITED(operand->fOffset, kPostfix_Kind, operand->fType)
    , fOperand(std::move(operand))
    , fOperator(op) {}

    bool hasSideEffects() const override {
        return true;
    }

    std::unique_ptr<Expression> clone() const override {
        return std::unique_ptr<Expression>(new PostfixExpression(fOperand->clone(), fOperator));
    }

    String description() const override {
        return fOperand->description() + Compiler::OperatorName(fOperator);
    }

    std::unique_ptr<Expression> fOperand;
    const Token::Kind fOperator;

    typedef Expression INHERITED;
};

} // namespace

#endif
