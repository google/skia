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
class IntLiteral : public Expression {
public:
    static constexpr Kind kExpressionKind = Kind::kIntLiteral;

    // FIXME: we will need to revisit this if/when we add full support for both signed and unsigned
    // 64-bit integers, but for right now an int64_t will hold every value we care about
    IntLiteral(const Context& context, int offset, int64_t value)
    : INHERITED(offset, IntLiteralData{context.fInt_Type.get(), value}) {}

    IntLiteral(int offset, int64_t value, const Type* type = nullptr)
    : INHERITED(offset, IntLiteralData{type, value}) {}

    int64_t value() const {
        return this->intLiteralData().fValue;
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
        return std::unique_ptr<Expression>(new IntLiteral(fOffset, this->value(), &this->type()));
    }

private:
    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
