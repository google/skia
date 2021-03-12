/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_BREAKSTATEMENT
#define SKSL_BREAKSTATEMENT

#include "include/private/SkSLStatement.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * A 'break' statement.
 */
class BreakStatement final : public Statement {
public:
    static constexpr Kind kStatementKind = Kind::kBreak;

    BreakStatement(int offset)
    : INHERITED(offset, kStatementKind) {}

    static std::unique_ptr<Statement> Make(int offset) {
        return std::make_unique<BreakStatement>(offset);
    }

    std::unique_ptr<Statement> clone() const override {
        return std::make_unique<BreakStatement>(fOffset);
    }

    String description() const override {
        return String("break;");
    }

private:
    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
