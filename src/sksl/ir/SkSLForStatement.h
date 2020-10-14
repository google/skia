/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FORSTATEMENT
#define SKSL_FORSTATEMENT

#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLSymbolTable.h"

namespace SkSL {

/**
 * A 'for' statement.
 */
class ForStatement : public Statement {
public:
    static constexpr Kind kStatementKind = Kind::kFor;

    ForStatement(int offset, std::unique_ptr<Statement> initializer,
                 std::unique_ptr<Expression> test, std::unique_ptr<Expression> next,
                 std::unique_ptr<Statement> statement, std::shared_ptr<SymbolTable> symbols)
    : INHERITED(offset, ForStatementData{std::move(symbols)}) {
        fStatementChildren.reserve_back(2);
        fStatementChildren.push_back(std::move(initializer));
        fStatementChildren.push_back(std::move(statement));
        fExpressionChildren.reserve_back(2);
        fExpressionChildren.push_back(std::move(test));
        fExpressionChildren.push_back(std::move(next));
    }

    std::unique_ptr<Statement>& initializer() {
        return fStatementChildren[0];
    }

    const std::unique_ptr<Statement>& initializer() const {
        return fStatementChildren[0];
    }

    std::unique_ptr<Expression>& test() {
        return fExpressionChildren[0];
    }

    const std::unique_ptr<Expression>& test() const {
        return fExpressionChildren[0];
    }

    std::unique_ptr<Expression>& next() {
        return fExpressionChildren[1];
    }

    const std::unique_ptr<Expression>& next() const {
        return fExpressionChildren[1];
    }

    std::unique_ptr<Statement>& statement() {
        return fStatementChildren[1];
    }

    const std::unique_ptr<Statement>& statement() const {
        return fStatementChildren[1];
    }

    std::shared_ptr<SymbolTable> symbols() const {
        return this->forStatementData().fSymbolTable;
    }

    std::unique_ptr<Statement> clone() const override {
        return std::unique_ptr<Statement>(new ForStatement(
                                       fOffset,
                                       this->initializer() ? this->initializer()->clone() : nullptr,
                                       this->test() ? this->test()->clone() : nullptr,
                                       this->next() ? this->next()->clone() : nullptr,
                                       this->statement()->clone(),
                                       this->symbols()));
    }

    String description() const override {
        String result("for (");
        if (this->initializer()) {
            result += this->initializer()->description();
        } else {
            result += ";";
        }
        result += " ";
        if (this->test()) {
            result += this->test()->description();
        }
        result += "; ";
        if (this->next()) {
            result += this->next()->description();
        }
        result += ") " + this->statement()->description();
        return result;
    }

private:
    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
