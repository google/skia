/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FLOATLITERAL
#define SKSL_FLOATLITERAL

#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * A literal floating point number.
 */
struct FloatLiteral : public Expression {
    FloatLiteral(IRGenerator* irGenerator, int offset, double value, IRNode::ID type = IRNode::ID())
    : INHERITED(irGenerator, offset, kFloatLiteral_Kind,
                type ? type : irGenerator->fContext.fFloatLiteral_Type)
    , fValue(value) {}

    String description() const override {
        return to_string(fValue);
    }

    bool hasSideEffects() const override {
        return false;
    }

    bool isConstant() const override {
        return true;
    }

    int coercionCost(const Type& target) const override {
        if (target.isFloat()) {
            return 0;
        }
        return INHERITED::coercionCost(target);
    }

    bool compareConstant(const Expression& other) const override {
        FloatLiteral& f = (FloatLiteral&) other;
        return fValue == f.fValue;
    }

    double getConstantFloat() const override {
        return fValue;
    }

    IRNode::ID clone() const override {
        return fIRGenerator->createNode(new FloatLiteral(fIRGenerator, fOffset, fValue, fType));
    }

    const double fValue;

    typedef Expression INHERITED;
};

} // namespace

#endif
