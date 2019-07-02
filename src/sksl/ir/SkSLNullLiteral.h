/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_NULLLITERAL
#define SKSL_NULLLITERAL

#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * Represents 'null'.
 */
struct NullLiteral : public Expression {
    NullLiteral(IRGenerator* irGenerator, int offset, IRNode::ID type = IRNode::ID())
    : INHERITED(irGenerator, offset, kNullLiteral_Kind,
                type ? type : irGenerator->fContext.fNull_Type) {}

    String description() const override {
        return "null";
    }

    bool hasSideEffects() const override {
        return false;
    }

    bool isConstant() const override {
        return true;
    }

    bool compareConstant(const Expression& other) const override {
        return true;
    }

    IRNode::ID clone() const override {
        return fIRGenerator->createNode(new NullLiteral(fIRGenerator, fOffset, fType));
    }

    typedef Expression INHERITED;
};

} // namespace

#endif
