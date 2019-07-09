/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FORSTATEMENT
#define SKSL_FORSTATEMENT

#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLSymbolTable.h"

namespace SkSL {

/**
 * A 'for' statement.
 */
struct ForStatement : public Statement {
    ForStatement(IRGenerator* irGenerator, int offset, IRNode::ID initializer, IRNode::ID test,
                 IRNode::ID next, IRNode::ID statement, std::shared_ptr<SymbolTable> symbols)
    : INHERITED(irGenerator, offset, kFor_Kind)
    , fSymbols(symbols)
    , fInitializer(initializer)
    , fTest(test)
    , fNextExpression(next)
    , fStatement(statement) {}

    IRNode::ID clone() const override {
        return fIRGenerator->createNode(new ForStatement(fIRGenerator, fOffset, fInitializer,
                                                         fTest, fNextExpression, fStatement,
                                                         fSymbols));
    }

    String description() const override {
        String result("for (");
        if (fInitializer) {
            result += fInitializer.node().description();
        }
        result += " ";
        if (fTest) {
            result += fTest.node().description();
        }
        result += "; ";
        if (fNextExpression) {
            result += fNextExpression.node().description();
        }
        result += ") " + fStatement.node().description();
        return result;
    }

    // it's important to keep fSymbols defined first (and thus destroyed last) because destroying
    // the other fields can update symbol reference counts
    const std::shared_ptr<SymbolTable> fSymbols;
    IRNode::ID fInitializer;
    IRNode::ID fTest;
    IRNode::ID fNextExpression;
    IRNode::ID fStatement;

    typedef Statement INHERITED;
};

} // namespace

#endif
