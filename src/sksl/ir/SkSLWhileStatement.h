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
struct WhileStatement final : public Statement {
    static constexpr Kind kStatementKind = Kind::kWhile;

    WhileStatement(int offset, std::unique_ptr<Expression> test,
                   std::unique_ptr<Statement> statement)
        : INHERITED(offset, kStatementKind)
        , fTest(std::move(test))
        , fStatement(std::move(statement)) {}

    std::unique_ptr<Expression>& test() {
        return fTest;
    }

    const std::unique_ptr<Expression>& test() const {
        return fTest;
    }

    std::unique_ptr<Statement>& statement() {
        return fStatement;
    }

    const std::unique_ptr<Statement>& statement() const {
        return fStatement;
    }

    std::unique_ptr<Statement> clone() const override {
        return std::unique_ptr<Statement>(new WhileStatement(fOffset, this->test()->clone(),
                                                             this->statement()->clone()));
    }

    String description() const override {
        return "while (" + this->test()->description() + ") " + this->statement()->description();
    }

private:
    std::unique_ptr<Expression> fTest;
    std::unique_ptr<Statement> fStatement;

    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
