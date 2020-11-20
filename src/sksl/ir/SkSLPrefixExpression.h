/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_PREFIXEXPRESSION
#define SKSL_PREFIXEXPRESSION

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/SkSLLexer.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFloatLiteral.h"

namespace SkSL {

/**
 * An expression modified by a unary operator appearing before it, such as '!flag'.
 */
class PrefixExpression final : public Expression {
public:
    static constexpr Kind kExpressionKind = Kind::kPrefix;

    PrefixExpression(Token::Kind op, std::unique_ptr<Expression> operand)
        : INHERITED(operand->fOffset, kExpressionKind, &operand->type())
        , fOperator(op)
        , fOperand(std::move(operand)) {}

    Token::Kind getOperator() const {
        return fOperator;
    }

    std::unique_ptr<Expression>& operand() {
        return fOperand;
    }

    const std::unique_ptr<Expression>& operand() const {
        return fOperand;
    }

    bool isNegationOfCompileTimeConstant() const {
        return this->getOperator() == Token::Kind::TK_MINUS &&
               this->operand()->isCompileTimeConstant();
    }

    bool isCompileTimeConstant() const override {
        return this->isNegationOfCompileTimeConstant();
    }

    bool hasProperty(Property property) const override {
        if (property == Property::kSideEffects &&
            (this->getOperator() == Token::Kind::TK_PLUSPLUS ||
             this->getOperator() == Token::Kind::TK_MINUSMINUS)) {
            return true;
        }
        return this->operand()->hasProperty(property);
    }

    std::unique_ptr<Expression> constantPropagate(const IRGenerator& irGenerator,
                                                  const DefinitionMap& definitions) override;

    SKSL_FLOAT getFVecComponent(int index) const override {
        SkASSERT(this->getOperator() == Token::Kind::TK_MINUS);
        return -this->operand()->getFVecComponent(index);
    }

    SKSL_INT getIVecComponent(int index) const override {
        SkASSERT(this->getOperator() == Token::Kind::TK_MINUS);
        return -this->operand()->getIVecComponent(index);
    }

    SKSL_FLOAT getMatComponent(int col, int row) const override {
        SkASSERT(this->getOperator() == Token::Kind::TK_MINUS);
        return -this->operand()->getMatComponent(col, row);
    }

    std::unique_ptr<Expression> clone() const override {
        return std::unique_ptr<Expression>(new PrefixExpression(this->getOperator(),
                                                                this->operand()->clone()));
    }

    String description() const override {
        return Compiler::OperatorName(this->getOperator()) + this->operand()->description();
    }

    int64_t getConstantInt() const override {
        SkASSERT(this->isNegationOfCompileTimeConstant());
        return -this->operand()->getConstantInt();
    }

    SKSL_FLOAT getConstantFloat() const override {
        SkASSERT(this->isNegationOfCompileTimeConstant());
        return -this->operand()->getConstantFloat();
    }

    bool compareConstant(const Context& context, const Expression& other) const override {
        // This expression and the other expression must be of the same kind. Since the only
        // compile-time PrefixExpression we optimize for is negation, that means we're comparing
        // -X == -Y. The negatives should cancel out, so we can just constant-compare the inner
        // expressions.
        SkASSERT(this->isNegationOfCompileTimeConstant());
        SkASSERT(other.as<PrefixExpression>().isNegationOfCompileTimeConstant());
        return this->operand()->compareConstant(context, *other.as<PrefixExpression>().operand());
    }

private:
    Token::Kind fOperator;
    std::unique_ptr<Expression> fOperand;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
