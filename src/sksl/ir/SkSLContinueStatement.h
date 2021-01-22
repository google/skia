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
class ContinueStatement final : public Statement {
public:
    static constexpr Kind kStatementKind = Kind::kContinue;

    ContinueStatement(int offset)
    : INHERITED(offset, kStatementKind) {}

    std::unique_ptr<Statement> clone() const override {
        return std::unique_ptr<Statement>(new ContinueStatement(fOffset));
    }

    String description() const override {
        return String("continue;");
    }

private:
    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
