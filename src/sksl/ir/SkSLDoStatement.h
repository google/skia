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
class DoStatement : public Statement {
public:
    static constexpr Kind kStatementKind = Kind::kDo;

    DoStatement(int offset, std::unique_ptr<Statement> statement,
                std::unique_ptr<Expression> test)
    : INHERITED(offset, kStatementKind) {
        fStatementChildren.push_back(std::move(statement));
        fExpressionChildren.push_back(std::move(test));
    }

    std::unique_ptr<Statement>& statement() {
        return fStatementChildren[0];
    }

    const std::unique_ptr<Statement>& statement() const {
        return fStatementChildren[0];
    }

    std::unique_ptr<Expression>& test() {
        return fExpressionChildren[0];
    }

    const std::unique_ptr<Expression>& test() const {
        return fExpressionChildren[0];
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
    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
