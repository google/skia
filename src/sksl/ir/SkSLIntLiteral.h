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
 * A literal integer. These are generally referred to as IntLiteral, but Literal<SKSL_INT> is
 * also available for use with template code.
 */
template <typename T> class Literal;
using IntLiteral = Literal<SKSL_INT>;

template <>
class Literal<SKSL_INT> final : public Expression {
public:
    static constexpr Kind kExpressionKind = Kind::kIntLiteral;

    // We will need to revisit this if we want full support for unsigned 64-bit integers,
    // but for now an SKSL_INT (int64_t) will hold every value we care about.
    Literal(const Context& context, int offset, SKSL_INT value)
        : Literal(offset, value, context.fTypes.fIntLiteral.get()) {}

    Literal(int offset, int64_t value, const Type* type)
        : INHERITED(offset, kExpressionKind, type)
        , fValue(value) {}

    SKSL_INT value() const {
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

    ComparisonResult compareConstant(const Expression& other) const override {
        if (!other.is<IntLiteral>()) {
            return ComparisonResult::kUnknown;
        }
        return this->value() == other.as<IntLiteral>().value() ? ComparisonResult::kEqual
                                                               : ComparisonResult::kNotEqual;
    }

    CoercionCost coercionCost(const Type& target) const override {
        if (target.isSigned() || target.isUnsigned() || target.isFloat() || target.isEnum()) {
            return CoercionCost::Free();
        }
        return INHERITED::coercionCost(target);
    }

    SKSL_INT getConstantInt() const override {
        return this->value();
    }

    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<IntLiteral>(fOffset, this->value(), &this->type());
    }

private:
    SKSL_INT fValue;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
