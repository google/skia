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
    IntLiteral(const Context& context, int offset, int64_t value)
    : INHERITED(offset, kIntLiteral_Kind, *context.fInt_Type)
    , fValue(value) {}

    IntLiteral(int offset, int64_t value, const Type* type = nullptr)
    : INHERITED(offset, kIntLiteral_Kind, *type)
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

    bool compareConstant(const Context& context, const Expression& other) const override {
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

    std::unique_ptr<Expression> clone() const override {
        return std::unique_ptr<Expression>(new IntLiteral(fOffset, fValue, &fType));
    }

    const int64_t fValue;

    typedef Expression INHERITED;
};

} // namespace

#endif
