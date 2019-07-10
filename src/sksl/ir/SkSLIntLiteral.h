/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_INTLITERAL
#define SKSL_INTLITERAL

#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * A literal integer.
 */
struct IntLiteral : public Expression {
    // FIXME: we will need to revisit this if/when we add full support for both signed and unsigned
    // 64-bit integers, but for right now an int64_t will hold every value we care about
    IntLiteral(IRGenerator* irGenerator, int offset, int64_t value, IRNode::ID type = IRNode::ID())
    : INHERITED(irGenerator, offset, kIntLiteral_Kind,
                type ? type : irGenerator->fContext.fInt_Type)
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

    bool compareConstant(const Expression& other) const override {
        IntLiteral& i = (IntLiteral&) other;
        return fValue == i.fValue;
    }

    int coercionCost(const Type& target) const override {
        if (target.isSigned() || target.isUnsigned() || target.isFloat()) {
            return 0;
        }
        return INHERITED::coercionCost(target);
    }

    int64_t getConstantInt() const override {
        return fValue;
    }

    IRNode::ID clone() const override {
        return fIRGenerator->createNode(new IntLiteral(fIRGenerator, fOffset, fValue, fType));
    }

    const int64_t fValue;

    typedef Expression INHERITED;
};

} // namespace

#endif
