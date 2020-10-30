/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DOSTATEMENT
#define SKSL_DOSTATEMENT

#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLStatement.h"

namespace SkSL {

/**
 * A 'do' statement.
 */
class DoStatement final : public Statement {
public:
    static constexpr Kind kStatementKind = Kind::kDo;

    DoStatement(int offset, std::unique_ptr<Statement> statement,
                std::unique_ptr<Expression> test)
        : INHERITED(offset, kStatementKind)
        , fStatement(std::move(statement))
        , fTest(std::move(test)) {}

    std::unique_ptr<Statement>& statement() {
        return fStatement;
    }

    const std::unique_ptr<Statement>& statement() const {
        return fStatement;
    }

    std::unique_ptr<Expression>& test() {
        return fTest;
    }

    const std::unique_ptr<Expression>& test() const {
        return fTest;
    }

    std::unique_ptr<Statement> clone() const override {
        return std::unique_ptr<Statement>(new DoStatement(fOffset, this->statement()->clone(),
                                                          this->test()->clone()));
    }

    String description() const override {
        return "do " + this->statement()->description() + " while (" + this->test()->description() +
               ");";
    }

private:
    std::unique_ptr<Statement> fStatement;
    std::unique_ptr<Expression> fTest;

    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
