/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTFLOATLITERAL
#define SKSL_ASTFLOATLITERAL

#include "SkSLASTExpression.h"

namespace SkSL {

/**
 * A literal floating point number.
 */
struct ASTFloatLiteral : public ASTExpression {
    ASTFloatLiteral(Position position, double value)
    : INHERITED(position, kFloat_Kind)
    , fValue(value) {}

    String description() const override {
        return to_string(fValue);
    }

    const double fValue;

    typedef ASTExpression INHERITED;
};

} // namespace

#endif
