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
    PostfixExpression(IRGenerator* irGenerator, IRNode::ID operand, Token::Kind op)
    : INHERITED(irGenerator, operand.node().fOffset, kPostfix_Kind, operand.expressionNode().fType)
    , fOperand(operand)
    , fOperator(op) {}

    bool hasSideEffects() const override {
        return true;
    }

    IRNode::ID clone() const override {
        return fIRGenerator->createNode(new PostfixExpression(fIRGenerator, fOperand, fOperator));
    }

    String description() const override {
        return fOperand.node().description() + Compiler::OperatorName(fOperator);
    }

    IRNode::ID fOperand;
    const Token::Kind fOperator;

    typedef Expression INHERITED;
};

} // namespace

#endif
