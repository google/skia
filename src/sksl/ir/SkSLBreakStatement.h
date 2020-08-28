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
    static constexpr Kind kIRNodeKind = kBreak_Kind;

    BreakStatement(int offset)
    : INHERITED(offset, kIRNodeKind) {}

    std::unique_ptr<IRNode> clone() const override {
        return std::unique_ptr<IRNode>(new BreakStatement(fOffset));
    }

    String description() const override {
        return String("break;");
    }

    typedef Statement INHERITED;
};

}  // namespace SkSL

#endif
