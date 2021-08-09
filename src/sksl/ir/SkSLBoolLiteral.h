/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_BOOLLITERAL
#define SKSL_BOOLLITERAL

#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * Represents 'true' or 'false'. These are generally referred to as BoolLiteral, but Literal<bool>
 * is also available for use with template code.
 */
template <typename T> class Literal;
using BoolLiteral = Literal<bool>;

template <>
class Literal<bool> final : public Expression {
public:
    static constexpr Kind kExpressionKind = Kind::kBoolLiteral;

    Literal(int offset, bool value, const Type* type)
        : INHERITED(offset, kExpressionKind, type)
        , fValue(value) {}

    // Makes a literal of boolean type.
    static std::unique_ptr<BoolLiteral> Make(const Context& context, int offset, bool value) {
        return std::make_unique<BoolLiteral>(offset, value, context.fTypes.fBool.get());
    }

    // Makes a literal of boolean type. (Functionally identical to the above, but useful if you
    // don't have access to the Context.)
    static std::unique_ptr<BoolLiteral> Make(int offset, bool value, const Type* type) {
        SkASSERT(type->isBoolean());
        return std::make_unique<BoolLiteral>(offset, value, type);
    }

    bool value() const {
        return fValue;
    }

    String description() const override {
        return String(this->value() ? "true" : "false");
    }

    bool hasProperty(Property property) const override {
        return false;
    }

    bool isCompileTimeConstant() const override {
        return true;
    }

    ComparisonResult compareConstant(const Expression& other) const override {
        if (!other.is<BoolLiteral>()) {
            return ComparisonResult::kUnknown;
        }
        return this->value() == other.as<BoolLiteral>().value() ? ComparisonResult::kEqual
                                                                : ComparisonResult::kNotEqual;
    }

    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<BoolLiteral>(fOffset, this->value(), &this->type());
    }

    const Expression* getConstantSubexpression(int n) const override {
        SkASSERT(n == 0);
        return this;
    }

private:
    bool fValue;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
