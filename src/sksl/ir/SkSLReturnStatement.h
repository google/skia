/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_RETURNSTATEMENT
#define SKSL_RETURNSTATEMENT

#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLStatement.h"

namespace SkSL {

/**
 * A 'return' statement.
 */
struct ReturnStatement : public Statement {
    ReturnStatement(IRGenerator* irGenerator, int offset)
    : INHERITED(irGenerator, offset, kReturn_Kind) {}

    ReturnStatement(IRGenerator* irGenerator, IRNode::ID expression)
    : INHERITED(irGenerator, expression.node().fOffset, kReturn_Kind)
    , fExpression(expression) {}

    IRNode::ID clone() const override {
        if (fExpression) {
            return fIRGenerator->createNode(new ReturnStatement(fIRGenerator, fExpression));
        }
        return fIRGenerator->createNode(new ReturnStatement(fIRGenerator, fOffset));
    }

    String description() const override {
        if (fExpression) {
            return "return " + fExpression.node().description() + ";";
        } else {
            return String("return;");
        }
    }

    IRNode::ID fExpression;

    typedef Statement INHERITED;
};

} // namespace

#endif
