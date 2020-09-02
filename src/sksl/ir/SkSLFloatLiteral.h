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
    static constexpr Kind kExpressionKind = Kind::kFloatLiteral;

    FloatLiteral(const Context& context, int offset, double value)
    : INHERITED(offset, kExpressionKind, context.fFloatLiteral_Type.get())
    , fValue(value) {}

    FloatLiteral(int offset, double value, const Type* type)
    : INHERITED(offset, kExpressionKind, type)
    , fValue(value) {}

    String description() const override {
        return to_string(fValue);
    }

    bool hasProperty(Property property) const override {
        return false;
    }

    bool isCompileTimeConstant() const override {
        return true;
    }

    CoercionCost coercionCost(const Type& target) const override {
        if (target.isFloat()) {
            return CoercionCost::Free();
        }
        return INHERITED::coercionCost(target);
    }

    bool compareConstant(const Context& context, const Expression& other) const override {
        return fValue == other.as<FloatLiteral>().fValue;
    }

    double getConstantFloat() const override {
        return fValue;
    }

    std::unique_ptr<Expression> clone() const override {
        return std::unique_ptr<Expression>(new FloatLiteral(fOffset, fValue, &this->type()));
    }

    const double fValue;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
