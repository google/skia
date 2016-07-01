/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_TYPEREFERENCE
#define SKSL_TYPEREFERENCE

#include "SkSLExpression.h"

namespace SkSL {

/**
 * Represents an identifier referring to a type. This is an intermediate value: TypeReferences are 
 * always eventually replaced by Constructors in valid programs.
 */
struct TypeReference : public Expression {
    TypeReference(Position position, std::shared_ptr<Type> type)
    : INHERITED(position, kTypeReference_Kind, kInvalid_Type)
    , fValue(std::move(type)) {}

    std::string description() const override {
    	ASSERT(false);
    	return "<type>";
    }

    const std::shared_ptr<Type> fValue;

    typedef Expression INHERITED;
};

} // namespace

#endif
