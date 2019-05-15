/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_EXTERNALVALUEREFERENCE
#define SKSL_EXTERNALVALUEREFERENCE

#include "SkSLExpression.h"
#include "src/sksl/SkSLExternalValue.h"

namespace SkSL {

/**
 * Represents an identifier referring to an ExternalValue.
 */
struct ExternalValueReference : public Expression {
    ExternalValueReference(int offset, ExternalValue* ev)
    : INHERITED(offset, kExternalValue_Kind, ev->type())
    , fValue(ev) {}

    bool hasSideEffects() const override {
        return true;
    }

    String description() const override {
        return String(fValue->fName);
    }

    std::unique_ptr<Expression> clone() const override {
        return std::unique_ptr<Expression>(new ExternalValueReference(fOffset, fValue));
    }

    ExternalValue* fValue;

    typedef Expression INHERITED;
};

} // namespace

#endif
