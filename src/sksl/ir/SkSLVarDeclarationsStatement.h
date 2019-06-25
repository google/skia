/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_VARDECLARATIONSSTATEMENT
#define SKSL_VARDECLARATIONSSTATEMENT

#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"

namespace SkSL {

/**
 * One or more variable declarations appearing as a statement within a function.
 */
struct VarDeclarationsStatement : public Statement {
    VarDeclarationsStatement(IRGenerator* irGenerator, IRNode::ID decl)
    : INHERITED(irGenerator, decl.node().fOffset, kVarDeclarations_Kind)
    , fDeclaration(decl) {}

    bool isEmpty() const override {
        for (const auto& s : ((VarDeclarations&) fDeclaration.node()).fVars) {
            if (!s.expressionNode().isEmpty()) {
                return false;
            }
        }
        return true;
    }

    IRNode::ID clone() const override {
        return fIRGenerator->createNode(new VarDeclarationsStatement(fIRGenerator, fDeclaration));
    }

    String description() const override {
        return fDeclaration.node().description() + ";";
    }

    IRNode::ID fDeclaration;

    typedef Statement INHERITED;
};

} // namespace

#endif
