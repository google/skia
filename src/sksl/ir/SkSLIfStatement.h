/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_IFSTATEMENT
#define SKSL_IFSTATEMENT

#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLStatement.h"

namespace SkSL {

/**
 * An 'if' statement.
 */
struct IfStatement : public Statement {
    IfStatement(IRGenerator* irGenerator, int offset, bool isStatic, IRNode::ID test,
                IRNode::ID ifTrue, IRNode::ID ifFalse)
    : INHERITED(irGenerator, offset, kIf_Kind)
    , fIsStatic(isStatic)
    , fTest(test)
    , fIfTrue(ifTrue)
    , fIfFalse(ifFalse) {}

    IRNode::ID clone() const override {
        return fIRGenerator->createNode(new IfStatement(fIRGenerator, fOffset, fIsStatic, fTest,
                                                        fIfTrue, fIfFalse));
    }

    String description() const override {
        String result;
        if (fIsStatic) {
            result += "@";
        }
        result += "if (" + fTest.node().description() + ") " + fIfTrue.node().description();
        if (fIfFalse) {
            result += " else " + fIfFalse.node().description();
        }
        return result;
    }

    bool fIsStatic;
    IRNode::ID fTest;
    IRNode::ID fIfTrue;
    // may be null
    IRNode::ID fIfFalse;

    typedef Statement INHERITED;
};

} // namespace

#endif
