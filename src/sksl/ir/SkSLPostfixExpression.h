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
class PostfixExpression final : public Expression {
public:
    static constexpr Kind kExpressionKind = Kind::kPostfix;

    PostfixExpression(std::unique_ptr<Expression> operand, Token::Kind op)
        : INHERITED(operand->fOffset, kExpressionKind, &operand->type())
        , fOperand(std::move(operand))
        , fOperator(op) {}

    Token::Kind getOperator() const {
        return fOperator;
    }

    std::unique_ptr<Expression>& operand() {
        return fOperand;
    }

    const std::unique_ptr<Expression>& operand() const {
        return fOperand;
    }

    bool hasProperty(Property property) const override {
        if (property == Property::kSideEffects) {
            return true;
        }
        return this->operand()->hasProperty(property);
    }

    std::unique_ptr<Expression> clone() const override {
        return std::unique_ptr<Expression>(new PostfixExpression(this->operand()->clone(),
                                                                 this->getOperator()));
    }

    String description() const override {
        return this->operand()->description() + Compiler::OperatorName(this->getOperator());
    }

private:
    std::unique_ptr<Expression> fOperand;
    Token::Kind fOperator;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
