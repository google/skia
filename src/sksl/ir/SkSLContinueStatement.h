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
    static constexpr Kind kIRNodeKind = kContinue_Kind;

    ContinueStatement(int offset)
    : INHERITED(offset, kIRNodeKind) {}

    std::unique_ptr<IRNode> clone() const override {
        return std::unique_ptr<IRNode>(new ContinueStatement(fOffset));
    }

    String description() const override {
        return String("continue;");
    }

    typedef Statement INHERITED;
};

}  // namespace SkSL

#endif
