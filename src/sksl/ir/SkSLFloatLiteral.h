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
 * A literal floating point number. These are generally referred to as FloatLiteral, but
 * Literal<SKSL_FLOAT> is also available for use with template code.
 */
template <typename T> class Literal;
using FloatLiteral = Literal<SKSL_FLOAT>;

template <>
class Literal<SKSL_FLOAT> final : public Expression {
public:
    static constexpr Kind kExpressionKind = Kind::kFloatLiteral;

    Literal(const Context& context, int offset, float value)
        : INHERITED(offset, kExpressionKind, context.fFloatLiteral_Type.get())
        , fValue(value) {}

    Literal(int offset, float value, const Type* type)
        : INHERITED(offset, kExpressionKind, type)
        , fValue(value) {}

    float value() const {
        return fValue;
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
        return std::make_unique<FloatLiteral>(fOffset, this->value(), &this->type());
    }

private:
    float fValue;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
