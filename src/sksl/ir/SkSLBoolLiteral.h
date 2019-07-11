/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_BOOLLITERAL
#define SKSL_BOOLLITERAL

#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * Represents 'true' or 'false'.
 */
struct BoolLiteral : public Expression {
    BoolLiteral(IRGenerator* irGenerator, int offset, bool value)
    : INHERITED(irGenerator, offset, kBoolLiteral_Kind, irGenerator->fContext.fBool_Type)
    , fValue(value) {}

    BoolLiteral(IRGenerator* irGenerator, int offset, bool value, IRNode::ID type)
    : INHERITED(irGenerator, offset, kBoolLiteral_Kind, type)
    , fValue(value) {}

    String description() const override {
        return String(fValue ? "true" : "false");
    }

    bool hasSideEffects() const override {
        return false;
    }

    bool isConstant() const override {
        return true;
    }

    bool compareConstant(const Expression& other) const override {
        BoolLiteral& b = (BoolLiteral&) other;
        return fValue == b.fValue;
    }

    IRNode::ID clone() const override {
        return fIRGenerator->createNode(new BoolLiteral(fIRGenerator, fOffset, fValue, fType));
    }

    const bool fValue;

    typedef Expression INHERITED;
};

} // namespace

#endif
