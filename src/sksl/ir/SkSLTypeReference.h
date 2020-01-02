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
    TypeReference(const Context& context, int offset, const Type& value)
    : INHERITED(offset, kTypeReference_Kind, *context.fInvalid_Type)
    , fValue(value) {}

    bool hasProperty(Property property) const override {
        return false;
    }

#ifdef SK_DEBUG
    String description() const override {
        return String(fValue.fName);
    }
#endif

    std::unique_ptr<Expression> clone() const override {
        return std::unique_ptr<Expression>(new TypeReference(fOffset, fValue, &fType));
    }

    const Type& fValue;

    typedef Expression INHERITED;

private:
    TypeReference(int offset, const Type& value, const Type* type)
    : INHERITED(offset, kTypeReference_Kind, *type)
    , fValue(value) {}
};

} // namespace

#endif
