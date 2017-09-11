/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_BOOLLITERAL
#define SKSL_BOOLLITERAL

#include "SkSLContext.h"
#include "SkSLExpression.h"

namespace SkSL {

/**
 * Represents 'true' or 'false'.
 */
struct BoolLiteral : public Expression {
    BoolLiteral(const Context& context, Position position, bool value)
    : INHERITED(position, kBoolLiteral_Kind, *context.fBool_Type)
    , fValue(value) {}

    String description() const override {
        return String(fValue ? "true" : "false");
    }

    bool hasSideEffects() const override {
        return false;
    }

    bool isConstant() const override {
        return true;
    }

    bool compareConstant(const Context& context, const Expression& other) const override {
        BoolLiteral& b = (BoolLiteral&) other;
        return fValue == b.fValue;
    }

    const bool fValue;

    typedef Expression INHERITED;
};

} // namespace

#endif
