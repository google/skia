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
    DiscardStatement(int offset)
    : INHERITED(offset, kDiscard_Kind) {}

    std::unique_ptr<Statement> clone() const override {
        return std::unique_ptr<Statement>(new DiscardStatement(fOffset));
    }

    String description() const override {
        return String("discard;");
    }

    typedef Statement INHERITED;
};

} // namespace

#endif
