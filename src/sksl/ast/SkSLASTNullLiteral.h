/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTNULLLITERAL
#define SKSL_ASTNULLLITERAL

#include "SkSLASTExpression.h"

namespace SkSL {

/**
 * Represents "null".
 */
struct ASTNullLiteral : public ASTExpression {
    ASTNullLiteral(int offset)
    : INHERITED(offset, kNull_Kind) {}

    String description() const override {
        return "null";
    }

    typedef ASTExpression INHERITED;
};

} // namespace

#endif
