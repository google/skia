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
    static constexpr Kind kExpressionKind = Kind::kPostfix;

    PostfixExpression(std::unique_ptr<Expression> operand, Token::Kind op)
    : INHERITED(operand->fOffset, kExpressionKind, &operand->type())
    , fOperand(std::move(operand))
    , fOperator(op) {}

    bool hasProperty(Property property) const override {
        if (property == Property::kSideEffects) {
            return true;
        }
        return fOperand->hasProperty(property);
    }

    std::unique_ptr<Expression> clone() const override {
        return std::unique_ptr<Expression>(new PostfixExpression(fOperand->clone(), fOperator));
    }

    String description() const override {
        return fOperand->description() + Compiler::OperatorName(fOperator);
    }

    std::unique_ptr<Expression> fOperand;
    const Token::Kind fOperator;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
