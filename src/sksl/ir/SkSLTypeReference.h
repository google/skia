/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_TYPEREFERENCE
#define SKSL_TYPEREFERENCE

#include "SkSLContext.h"
#include "SkSLExpression.h"

namespace SkSL {

/**
 * Represents an identifier referring to a type. This is an intermediate value: TypeReferences are
 * always eventually replaced by Constructors in valid programs.
 */
struct TypeReference : public Expression {
    TypeReference(const Context& context, Position position, const Type& type)
    : INHERITED(position, kTypeReference_Kind, *context.fInvalid_Type)
    , fValue(type) {}

    bool hasSideEffects() const override {
        return false;
    }

    String description() const override {
        return fValue.name();
    }

    const Type& fValue;

    typedef Expression INHERITED;
};

} // namespace

#endif
