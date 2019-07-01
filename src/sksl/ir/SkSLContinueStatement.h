/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONTINUESTATEMENT
#define SKSL_CONTINUESTATEMENT

#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLStatement.h"

namespace SkSL {

/**
 * A 'continue' statement.
 */
struct ContinueStatement : public Statement {
    ContinueStatement(IRGenerator* irGenerator, int offset)
    : INHERITED(irGenerator, offset, kContinue_Kind) {}

    IRNode::ID clone() const override {
        return fIRGenerator->createNode(new ContinueStatement(fIRGenerator, fOffset));
    }

    String description() const override {
        return String("continue;");
    }

    typedef Statement INHERITED;
};

} // namespace

#endif
