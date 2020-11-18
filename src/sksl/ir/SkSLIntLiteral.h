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

    // FIXME: we will need to revisit this if/when we add full support for both signed and unsigned
    // 64-bit integers, but for right now an int64_t will hold every value we care about
    Literal(const Context& context, int offset, int64_t value)
        : INHERITED(offset, kExpressionKind, context.fInt_Type.get())
        , fValue(value) {}

    Literal(int offset, int64_t value, const Type* type)
        : INHERITED(offset, kExpressionKind, type)
        , fValue(value) {}

    int64_t value() const {
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

    bool compareConstant(const Context& context, const Expression& other) const override {
        return this->value() == other.as<IntLiteral>().value();
    }

    CoercionCost coercionCost(const Type& target) const override {
        if (target.isSigned() || target.isUnsigned() || target.isFloat() ||
            target.typeKind() == Type::TypeKind::kEnum) {
            return CoercionCost::Free();
        }
        return INHERITED::coercionCost(target);
    }

    int64_t getConstantInt() const override {
        return this->value();
    }

    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<IntLiteral>(fOffset, this->value(), &this->type());
    }

private:
    int64_t fValue;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
