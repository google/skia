/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_VARIABLEREFERENCE
#define SKSL_VARIABLEREFERENCE

#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * A reference to a variable, through which it can be read or written. In the statement:
 *
 * x = x + 1;
 *
 * there is only one Variable 'x', but two VariableReferences to it.
 */
struct VariableReference : public Expression {
    VariableReference(IRGenerator* irGenerator, int offset, IRNode::ID variable,
                      RefKind refKind = kRead_RefKind);

    RefKind refKind() const {
        return fRefKind;
    }

    void setRefKind(RefKind refKind);

    bool hasSideEffects() const override {
        return false;
    }

    bool isConstant() const override {
        return (((Variable&) fVariable.node()).fModifiers.fFlags & Modifiers::kConst_Flag) != 0;
    }

    IRNode::ID clone() const override {
        return fIRGenerator->createNode(new VariableReference(fIRGenerator, fOffset, fVariable,
                                                              fRefKind));
    }

    String description() const override {
        return ((Variable&) fVariable.node()).fName;
    }

    static IRNode::ID CopyConstant(IRGenerator* irGenerator, const Expression& expr);

    IRNode::ID constantPropagate(const DefinitionMap& definitions) override;

    IRNode::ID fVariable;
    RefKind fRefKind;

private:
    typedef Expression INHERITED;
};

} // namespace

#endif
