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
    FloatLiteral(const Context& context, Position position, double value)
    : INHERITED(position, kFloatLiteral_Kind, *context.fFloat_Type)
    , fValue(value) {}

    virtual String description() const override {
        return to_string(fValue);
    }

    bool isConstant() const override {
        return true;
    }

    const double fValue;

    typedef Expression INHERITED;
};

} // namespace

#endif
