/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_BREAKSTATEMENT
#define SKSL_BREAKSTATEMENT

#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLStatement.h"

namespace SkSL {

/**
 * A 'break' statement.
 */
struct BreakStatement : public Statement {
    BreakStatement(int offset)
    : INHERITED(offset, kBreak_Kind) {}

    std::unique_ptr<Statement> clone() const override {
        return std::unique_ptr<Statement>(new BreakStatement(fOffset));
    }

    String description() const override {
        return String("break;");
    }

    typedef Statement INHERITED;
};

} // namespace

#endif
