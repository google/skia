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

    BreakStatement(int line)
    : INHERITED(line, kStatementKind) {}

    static std::unique_ptr<Statement> Make(int line) {
        return std::make_unique<BreakStatement>(line);
    }

    std::unique_ptr<Statement> clone() const override {
        return std::make_unique<BreakStatement>(fLine);
    }

    String description() const override {
        return String("break;");
    }

private:
    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
