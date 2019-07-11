/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FUNCTIONREFERENCE
#define SKSL_FUNCTIONREFERENCE

#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"

namespace SkSL {

/**
 * An identifier referring to a function name. This is an intermediate value: FunctionReferences are
 * always eventually replaced by FunctionCalls in valid programs.
 */
struct FunctionReference : public Expression {
    FunctionReference(IRGenerator* irGenerator, int offset, std::vector<IRNode::ID> function,
                      IRNode::ID type = IRNode::ID())
    : INHERITED(irGenerator, offset, kFunctionReference_Kind, type)
    , fFunctions(function) {}

    bool hasSideEffects() const override {
        return false;
    }

    IRNode::ID clone() const override {
        return fIRGenerator->createNode(new FunctionReference(fIRGenerator, fOffset, fFunctions,
                                                              fType));
    }

    String description() const override {
        return String("<function>");
    }

    const std::vector<IRNode::ID> fFunctions;

    typedef Expression INHERITED;
};

} // namespace

#endif
