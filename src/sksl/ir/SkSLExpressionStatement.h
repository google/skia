/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_EXPRESSIONSTATEMENT
#define SKSL_EXPRESSIONSTATEMENT

#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLStatement.h"

namespace SkSL {

/**
 * A lone expression being used as a statement.
 */
struct ExpressionStatement : public Statement {
    ExpressionStatement(IRGenerator* irGenerator, IRNode::ID expression)
    : INHERITED(irGenerator, -1, kExpression_Kind)
    , fExpression(expression) {}

    IRNode::ID clone() const override {
        return fIRGenerator->createNode(new ExpressionStatement(fIRGenerator, fExpression));
    }

    String description() const override {
        return fExpression.node().description() + ";";
    }

    IRNode::ID fExpression;

    typedef Statement INHERITED;
};

} // namespace

#endif
