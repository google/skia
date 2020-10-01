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

    FloatLiteral(const Context& context, int offset, float value)
    : INHERITED(offset, FloatLiteralData{context.fFloatLiteral_Type.get(), value}) {}

    FloatLiteral(int offset, float value, const Type* type)
    : INHERITED(offset, FloatLiteralData{type, value}) {}

    float value() const {
        return this->floatLiteralData().fValue;
    }

    String description() const override {
        return to_string(this->value());
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
        return this->value() == other.as<FloatLiteral>().value();
    }

    SKSL_FLOAT getConstantFloat() const override {
        return this->value();
    }

    std::unique_ptr<Expression> clone() const override {
        return std::unique_ptr<Expression>(new FloatLiteral(fOffset, this->value(), &this->type()));
    }

private:
    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
