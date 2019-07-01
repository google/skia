/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_BINARYEXPRESSION
#define SKSL_BINARYEXPRESSION

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/SkSLLexer.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * A binary operation.
 */
struct BinaryExpression : public Expression {
    BinaryExpression(IRGenerator* irGenerator, int offset, IRNode::ID left, Token::Kind op,
                     IRNode::ID right, IRNode::ID type)
    : INHERITED(irGenerator, offset, kBinary_Kind, type)
    , fLeft(left)
    , fOperator(op)
    , fRight(right) {
        SkASSERT(type != irGenerator->fContext.fInvalid_Type);
    }

    IRNode::ID constantPropagate(const DefinitionMap& definitions) override {
        return fIRGenerator->constantFold(fLeft, fOperator, fRight);
    }

    bool hasSideEffects() const override {
        return Compiler::IsAssignment(fOperator) || fLeft.node().hasSideEffects() ||
               fRight.node().hasSideEffects();
    }

    IRNode::ID clone() const override {
        return fIRGenerator->createNode(new BinaryExpression(fIRGenerator, fOffset, fLeft,
                                                             fOperator, fRight, fType));
    }

    String description() const override {
        return "(" + fLeft.node().description() + " " +
               Compiler::OperatorName(fOperator) + " " +
               fRight.node().description() + ")";
    }

    IRNode::ID fLeft;
    const Token::Kind fOperator;
    IRNode::ID fRight;

    typedef Expression INHERITED;
};

} // namespace

#endif
