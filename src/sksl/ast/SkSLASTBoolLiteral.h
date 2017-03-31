/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTBOOLLITERAL
#define SKSL_ASTBOOLLITERAL

#include "SkSLASTExpression.h"

namespace SkSL {

/**
 * Represents "true" or "false".
 */
struct ASTBoolLiteral : public ASTExpression {
    ASTBoolLiteral(Position position, bool value)
    : INHERITED(position, kBool_Kind)
    , fValue(value) {}

    String description() const override {
        return String(fValue ? "true" : "false");
    }

    const bool fValue;

    typedef ASTExpression INHERITED;
};

} // namespace

#endif
