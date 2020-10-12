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
class TypeReference : public Expression {
public:
    static constexpr Kind kExpressionKind = Kind::kTypeReference;

    TypeReference(const Context& context, int offset, const Type* value)
    : INHERITED(offset, TypeReferenceData{context.fInvalid_Type.get(), value}) {}

    const Type& type() const override {
        return *this->typeReferenceData().fType;
    }

    const Type& value() const {
        return *this->typeReferenceData().fValue;
    }

    bool hasProperty(Property property) const override {
        return false;
    }

    String description() const override {
        return String(this->value().name());
    }

    std::unique_ptr<Expression> clone() const override {
        return std::unique_ptr<Expression>(new TypeReference(fOffset, &this->value(),
                                                             &this->type()));
    }

private:
    TypeReference(int offset, const Type* value, const Type* type)
    : INHERITED(offset, TypeReferenceData{type, value}) {}

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
