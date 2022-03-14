/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DISCARDSTATEMENT
#define SKSL_DISCARDSTATEMENT

#include "include/private/SkSLStatement.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * A 'discard' statement.
 */
class DiscardStatement final : public Statement {
public:
    inline static constexpr Kind kStatementKind = Kind::kDiscard;

    DiscardStatement(Position pos)
    : INHERITED(pos, kStatementKind) {}

    static std::unique_ptr<Statement> Make(Position pos) {
        return std::make_unique<DiscardStatement>(pos);
    }

    std::unique_ptr<Statement> clone() const override {
        return std::make_unique<DiscardStatement>(fPosition);
    }

    std::string description() const override {
        return "discard;";
    }

private:
    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
