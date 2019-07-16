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
    PrefixExpression(IRGenerator* irGenerator, Token::Kind op, IRNode::ID operand)
    : INHERITED(irGenerator, operand.node().fOffset, kPrefix_Kind, operand.expression().fType)
    , fOperand(operand)
    , fOperator(op) {}

    bool isConstant() const override {
        return fOperator == Token::MINUS && fOperand.expression().isConstant();
    }

    bool hasSideEffects() const override {
        return fOperator == Token::PLUSPLUS || fOperator == Token::MINUSMINUS ||
               fOperand.expression().hasSideEffects();
    }

    IRNode::ID constantPropagate(const DefinitionMap& definitions) override {
        if (fOperand.expression().fKind == Expression::kFloatLiteral_Kind) {
            return fIRGenerator->createNode(new FloatLiteral(
                                                        fIRGenerator,
                                                        fOffset,
                                                        -((FloatLiteral&) fOperand.node()).fValue));

        }
        return IRNode::ID();
    }

    SKSL_FLOAT getFVecComponent(int index) const override {
        SkASSERT(fOperator == Token::Kind::MINUS);
        return -fOperand.expression().getFVecComponent(index);
    }

    SKSL_INT getIVecComponent(int index) const override {
        SkASSERT(fOperator == Token::Kind::MINUS);
        return -fOperand.expression().getIVecComponent(index);
    }

    SKSL_FLOAT getMatComponent(int col, int row) const override {
        SkASSERT(fOperator == Token::Kind::MINUS);
        return -fOperand.expression().getMatComponent(col, row);
    }

    IRNode::ID clone() const override {
        return fIRGenerator->createNode(new PrefixExpression(fIRGenerator, fOperator, fOperand));
    }

    String description() const override {
        return Compiler::OperatorName(fOperator) + fOperand.expression().description();
    }

    IRNode::ID fOperand;
    const Token::Kind fOperator;

    typedef Expression INHERITED;
};

} // namespace

#endif
