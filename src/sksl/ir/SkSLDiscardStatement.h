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
    static constexpr Kind kIRNodeKind = kDiscard_Kind;

    DiscardStatement(int offset)
    : INHERITED(offset, kIRNodeKind) {}

    std::unique_ptr<IRNode> clone() const override {
        return std::unique_ptr<IRNode>(new DiscardStatement(fOffset));
    }

    String description() const override {
        return String("discard;");
    }

    typedef Statement INHERITED;
};

}  // namespace SkSL

#endif
