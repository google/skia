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
class BreakStatement final : public Statement {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kBreak;

    BreakStatement(Position pos)
        : INHERITED(pos, kIRNodeKind) {}

    static std::unique_ptr<Statement> Make(Position pos) {
        return std::make_unique<BreakStatement>(pos);
    }

    std::string description() const override {
        return "break;";
    }

private:
    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
