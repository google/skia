/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FLOATLITERAL
#define SKSL_FLOATLITERAL

#include "SkSLContext.h"
#include "SkSLExpression.h"

namespace SkSL {

/**
 * A literal floating point number.
 */
struct FloatLiteral : public Expression {
    FloatLiteral(const Context& context, Position position, double value,
                 const Type* type = nullptr)
    : INHERITED(position, kFloatLiteral_Kind, type ? *type : *context.fFloat_Type)
    , fValue(value) {}

    String description() const override {
        return to_string(fValue);
    }

    bool hasSideEffects() const override {
        return false;
    }

    bool isConstant() const override {
        return true;
    }

    bool compareConstant(const Context& context, const Expression& other) const override {
        FloatLiteral& f = (FloatLiteral&) other;
        return fValue == f.fValue;
    }

    const double fValue;

    typedef Expression INHERITED;
};

} // namespace

#endif
