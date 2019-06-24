/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DISCARDSTATEMENT
#define SKSL_DISCARDSTATEMENT

#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLStatement.h"

namespace SkSL {

/**
 * A 'discard' statement.
 */
struct DiscardStatement : public Statement {
    DiscardStatement(IRGenerator* irGenerator, int offset)
    : INHERITED(irGenerator, offset, kDiscard_Kind) {}

    IRNode::ID clone() const override {
        return fIRGenerator->createNode(new DiscardStatement(fIRGenerator, fOffset));
    }

    String description() const override {
        return String("discard;");
    }

    typedef Statement INHERITED;
};

} // namespace

#endif
