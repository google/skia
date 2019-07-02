/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_EXTERNALVALUEREFERENCE
#define SKSL_EXTERNALVALUEREFERENCE

#include "src/sksl/SkSLExternalValue.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * Represents an identifier referring to an ExternalValue.
 */
struct ExternalValueReference : public Expression {
    ExternalValueReference(IRGenerator* irGenerator, int offset, ExternalValue* ev)
    : INHERITED(irGenerator, offset, kExternalValue_Kind, ev->type())
    , fValue(ev) {}

    bool hasSideEffects() const override {
        return true;
    }

    String description() const override {
        return String(fValue->fName);
    }

    IRNode::ID clone() const override {
        return fIRGenerator->createNode(new ExternalValueReference(fIRGenerator, fOffset, fValue));
    }

    ExternalValue* fValue;

    typedef Expression INHERITED;
};

} // namespace

#endif
