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
 * Represents 'true' or 'false'.
 */
class BoolLiteral : public Expression {
public:
    static constexpr Kind kExpressionKind = Kind::kBoolLiteral;

    BoolLiteral(const Context& context, int offset, bool value)
    : INHERITED(offset, BoolLiteralData{context.fBool_Type.get(), value}) {}

    bool value() const {
        return this->boolLiteralData().fValue;
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

    bool compareConstant(const Context& context, const Expression& other) const override {
        const BoolLiteral& b = other.as<BoolLiteral>();
        return this->value() == b.value();
    }

    std::unique_ptr<Expression> clone() const override {
        return std::unique_ptr<Expression>(new BoolLiteral(fOffset, this->value(), &this->type()));
    }

private:
    BoolLiteral(int offset, bool value, const Type* type)
    : INHERITED(offset, BoolLiteralData{type, value}) {}

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
