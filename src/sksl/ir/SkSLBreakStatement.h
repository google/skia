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
    BreakStatement(IRGenerator* irGenerator, int offset)
    : INHERITED(irGenerator, offset, kBreak_Kind) {}

    IRNode::ID clone() const override {
        return fIRGenerator->createNode(new BreakStatement(fIRGenerator, fOffset));
    }

    String description() const override {
        return String("break;");
    }

    typedef Statement INHERITED;
};

} // namespace

#endif
