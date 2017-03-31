/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_BREAKSTATEMENT
#define SKSL_BREAKSTATEMENT

#include "SkSLExpression.h"
#include "SkSLStatement.h"

namespace SkSL {

/**
 * A 'break' statement.
 */
struct BreakStatement : public Statement {
    BreakStatement(Position position)
    : INHERITED(position, kBreak_Kind) {}

    String description() const override {
        return String("break;");
    }

    typedef Statement INHERITED;
};

} // namespace

#endif
