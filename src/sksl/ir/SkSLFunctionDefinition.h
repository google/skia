/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FUNCTIONDEFINITION
#define SKSL_FUNCTIONDEFINITION

#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLProgramElement.h"

namespace SkSL {

/**
 * A function definition (a declaration plus an associated block of code).
 */
struct FunctionDefinition : public ProgramElement {
    FunctionDefinition(IRGenerator* irGenerator, int offset, IRNode::ID declaration,
                       IRNode::ID body)
    : INHERITED(irGenerator, offset, kFunction_Kind)
    , fDeclaration(declaration)
    , fBody(std::move(body)) {}

    IRNode::ID clone() const override {
        return fIRGenerator->createNode(new FunctionDefinition(fIRGenerator, fOffset, fDeclaration,
                                                               fBody));
    }

    String description() const override {
        return fDeclaration.node().description() + " " + fBody.node().description();
    }

    IRNode::ID fDeclaration;
    IRNode::ID fBody;

    typedef ProgramElement INHERITED;
};

} // namespace

#endif
