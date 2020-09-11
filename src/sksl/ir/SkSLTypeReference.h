/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_TYPEREFERENCE
#define SKSL_TYPEREFERENCE

#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * Represents an identifier referring to a type. This is an intermediate value: TypeReferences are
 * always eventually replaced by Constructors in valid programs.
 */
struct TypeReference : public Expression {
    static constexpr Kind kExpressionKind = Kind::kTypeReference;

    TypeReference(const Context& context, int offset, const Type* value)
    : INHERITED(offset, kExpressionKind, context.fInvalid_Type.get())
    , fValue(*value) {}

    bool hasProperty(Property property) const override {
        return false;
    }

    String description() const override {
        return String(fValue.fName);
    }

    std::unique_ptr<Expression> clone() const override {
        return std::unique_ptr<Expression>(new TypeReference(fOffset, fValue, &this->type()));
    }

    const Type& fValue;

    using INHERITED = Expression;

private:
    TypeReference(int offset, const Type& value, const Type* type)
    : INHERITED(offset, kExpressionKind, type)
    , fValue(value) {}
};

}  // namespace SkSL

#endif
