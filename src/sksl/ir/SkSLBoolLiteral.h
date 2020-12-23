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

    Literal(const Context& context, int offset, bool value)
        : INHERITED(offset, kExpressionKind, context.fBool_Type.get())
        , fValue(value) {}

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

    ComparisonResult compareConstant(const Context& context,
                                     const Expression& other) const override {
        if (!other.is<BoolLiteral>()) {
            return ComparisonResult::kUnknown;
        }
        return this->value() == other.as<BoolLiteral>().value() ? ComparisonResult::kEqual
                                                                : ComparisonResult::kNotEqual;
    }

    std::unique_ptr<Expression> clone() const override {
        return std::unique_ptr<BoolLiteral>(new BoolLiteral(fOffset, this->value(), &this->type()));
    }

private:
    Literal(int offset, bool value, const Type* type)
        : INHERITED(offset, kExpressionKind, type)
        , fValue(value) {}

    bool fValue;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
