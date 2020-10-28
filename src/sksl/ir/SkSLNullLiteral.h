/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_NULLLITERAL
#define SKSL_NULLLITERAL

#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * Represents 'null'.
 */
class NullLiteral final : public Expression {
public:
    static constexpr Kind kExpressionKind = Kind::kNullLiteral;

    NullLiteral(const Context& context, int offset)
    : INHERITED(offset, kExpressionKind, context.fNull_Type.get()) {}

    NullLiteral(int offset, const Type* type)
    : INHERITED(offset, kExpressionKind, type) {}

    String description() const override {
        return "null";
    }

    bool hasProperty(Property property) const override {
        return false;
    }

    bool isCompileTimeConstant() const override {
        return true;
    }

    bool compareConstant(const Context& context, const Expression& other) const override {
        return true;
    }

    std::unique_ptr<Expression> clone() const override {
        return std::unique_ptr<Expression>(new NullLiteral(fOffset, &this->type()));
    }

private:
    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
