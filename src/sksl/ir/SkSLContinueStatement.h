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
    inline static constexpr Kind kIRNodeKind = Kind::kContinue;

    ContinueStatement(Position pos)
    : INHERITED(pos, kIRNodeKind) {}

    static std::unique_ptr<Statement> Make(Position pos) {
        return std::make_unique<ContinueStatement>(pos);
    }

    std::string description() const override {
        return "continue;";
    }

private:
    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
