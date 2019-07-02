/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DOSTATEMENT
#define SKSL_DOSTATEMENT

#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLStatement.h"

namespace SkSL {

/**
 * A 'do' statement.
 */
struct DoStatement : public Statement {
    DoStatement(IRGenerator* irGenerator, int offset, IRNode::ID statement, IRNode::ID test)
    : INHERITED(irGenerator, offset, kDo_Kind)
    , fStatement(std::move(statement))
    , fTest(std::move(test)) {}

    IRNode::ID clone() const override {
        return fIRGenerator->createNode(new DoStatement(fIRGenerator, fOffset, fStatement,
                                                        fTest));
    }

    String description() const override {
        return "do " + fStatement.node().description() + " while (" +
               fTest.node().description() + ");";
    }

    IRNode::ID fStatement;
    IRNode::ID fTest;

    typedef Statement INHERITED;
};

} // namespace

#endif
