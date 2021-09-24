/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONTINUESTATEMENT
#define SKSL_CONTINUESTATEMENT

#include "include/private/SkSLStatement.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * A 'continue' statement.
 */
class ContinueStatement final : public Statement {
public:
    static constexpr Kind kStatementKind = Kind::kContinue;

    ContinueStatement(int offset)
    : INHERITED(offset, kStatementKind) {}

    static std::unique_ptr<Statement> Make(int offset) {
        return std::make_unique<ContinueStatement>(offset);
    }

    std::unique_ptr<Statement> clone() const override {
        return std::make_unique<ContinueStatement>(fOffset);
    }

    String description() const override {
        return String("continue;");
    }

private:
    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
