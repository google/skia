/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_WHILESTATEMENT
#define SKSL_WHILESTATEMENT

#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLStatement.h"

namespace SkSL {

/**
 * A 'while' loop.
 */
struct WhileStatement : public Statement {
    static constexpr Kind kStatementKind = Kind::kWhile;

    WhileStatement(int offset, std::unique_ptr<Expression> test,
                   std::unique_ptr<Statement> statement)
    : INHERITED(offset, kStatementKind) {
        fExpressionChildren.push_back(std::move(test));
        fStatementChildren.push_back(std::move(statement));
    }

    std::unique_ptr<Expression>& test() {
        return fExpressionChildren[0];
    }

    const std::unique_ptr<Expression>& test() const {
        return fExpressionChildren[0];
    }

    std::unique_ptr<Statement>& statement() {
        return fStatementChildren[0];
    }

    const std::unique_ptr<Statement>& statement() const {
        return fStatementChildren[0];
    }

    std::unique_ptr<Statement> clone() const override {
        return std::unique_ptr<Statement>(new WhileStatement(fOffset, this->test()->clone(),
                                                             this->statement()->clone()));
    }

    String description() const override {
        return "while (" + this->test()->description() + ") " + this->statement()->description();
    }

    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
