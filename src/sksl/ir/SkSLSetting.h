/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SETTING
#define SKSL_SETTING

#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

class IRGenerator;

/**
 * Represents a compile-time constant setting, such as sk_Caps.fbFetchSupport. These are generally
 * collapsed down to their constant representations during the compilation process.
 */
struct Setting : public Expression {
    Setting(IRGenerator* irGenerator, int offset, String name, IRNode::ID value)
    : INHERITED(fIRGenerator, offset, kSetting_Kind, value.expressionNode().fType)
    , fName(std::move(name))
    , fValue(value) {
        SkASSERT(fValue.expressionNode().isConstant());
    }

    IRNode::ID constantPropagate(const DefinitionMap& definitions) override;

    IRNode::ID clone() const override {
        return fIRGenerator->createNode(new Setting(fIRGenerator, fOffset, fName, fValue));
    }

    String description() const override {
        return fName;
    }

    bool hasSideEffects() const override {
        return false;
    }

    bool isConstant() const override {
        return true;
    }

    const String fName;
    IRNode::ID fValue;

    typedef Expression INHERITED;
};

} // namespace

#endif
