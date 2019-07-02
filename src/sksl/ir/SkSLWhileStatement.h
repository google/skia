/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_WHILESTATEMENT
#define SKSL_WHILESTATEMENT

#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLStatement.h"

namespace SkSL {

/**
 * A 'while' loop.
 */
struct WhileStatement : public Statement {
    WhileStatement(IRGenerator* irGenerator, int offset, IRNode::ID test,
                   IRNode::ID statement)
    : INHERITED(irGenerator, offset, kWhile_Kind)
    , fTest(test)
    , fStatement(statement) {}

    IRNode::ID clone() const override {
        return fIRGenerator->createNode(new WhileStatement(fIRGenerator, fOffset, fTest,
                                                           fStatement));
    }

    String description() const override {
        return "while (" + fTest.node().description() + ") " + fStatement.node().description();
    }

    IRNode::ID fTest;
    IRNode::ID fStatement;

    typedef Statement INHERITED;
};

} // namespace

#endif
