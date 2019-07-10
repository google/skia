/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_TERNARYEXPRESSION
#define SKSL_TERNARYEXPRESSION

#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * A ternary expression (test ? ifTrue : ifFalse).
 */
struct TernaryExpression : public Expression {
    TernaryExpression(IRGenerator* irGenerator, int offset, IRNode::ID test,
                      IRNode::ID ifTrue, IRNode::ID ifFalse)
    : INHERITED(irGenerator, offset, kTernary_Kind, ifTrue.expressionNode().fType)
    , fTest(test)
    , fIfTrue(ifTrue)
    , fIfFalse(ifFalse) {
        SkASSERT(fIfTrue.expressionNode().fType == fIfFalse.expressionNode().fType);
    }

    bool hasSideEffects() const override {
        return fTest.expressionNode().hasSideEffects() ||
               fIfTrue.expressionNode().hasSideEffects() ||
               fIfFalse.expressionNode().hasSideEffects();
    }

    IRNode::ID clone() const override {
        return fIRGenerator->createNode(new TernaryExpression(fIRGenerator, fOffset, fTest, fIfTrue,
                                                              fIfFalse));
    }

    String description() const override {
        return "(" + fTest.node().description() + " ? " + fIfTrue.node().description() + " : " +
               fIfFalse.node().description() + ")";
    }

    IRNode::ID fTest;
    IRNode::ID fIfTrue;
    IRNode::ID fIfFalse;

    typedef Expression INHERITED;
};

} // namespace

#endif
