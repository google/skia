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
struct PrefixExpression : public Expression {
    static constexpr Kind kExpressionKind = kPrefix_Kind;

    PrefixExpression(Token::Kind op, std::unique_ptr<Expression> operand)
    : INHERITED(operand->fOffset, kExpressionKind, operand->fType)
    , fOperand(std::move(operand))
    , fOperator(op) {}

    bool isCompileTimeConstant() const override {
        return fOperator == Token::Kind::TK_MINUS && fOperand->isCompileTimeConstant();
    }

    bool hasProperty(Property property) const override {
        if (property == Property::kSideEffects && (fOperator == Token::Kind::TK_PLUSPLUS ||
                                                   fOperator == Token::Kind::TK_MINUSMINUS)) {
            return true;
        }
        return fOperand->hasProperty(property);
    }

    std::unique_ptr<Expression> constantPropagate(const IRGenerator& irGenerator,
                                                  const DefinitionMap& definitions) override {
        if (fOperand->fKind == Expression::kFloatLiteral_Kind) {
            return std::unique_ptr<Expression>(new FloatLiteral(
                                                             irGenerator.fContext,
                                                             fOffset,
                                                             -fOperand->as<FloatLiteral>().fValue));

        }
        return nullptr;
    }

    SKSL_FLOAT getFVecComponent(int index) const override {
        SkASSERT(fOperator == Token::Kind::TK_MINUS);
        return -fOperand->getFVecComponent(index);
    }

    SKSL_INT getIVecComponent(int index) const override {
        SkASSERT(fOperator == Token::Kind::TK_MINUS);
        return -fOperand->getIVecComponent(index);
    }

    SKSL_FLOAT getMatComponent(int col, int row) const override {
        SkASSERT(fOperator == Token::Kind::TK_MINUS);
        return -fOperand->getMatComponent(col, row);
    }

    std::unique_ptr<Expression> clone() const override {
        return std::unique_ptr<Expression>(new PrefixExpression(fOperator, fOperand->clone()));
    }

    String description() const override {
        return Compiler::OperatorName(fOperator) + fOperand->description();
    }

    std::unique_ptr<Expression> fOperand;
    const Token::Kind fOperator;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
