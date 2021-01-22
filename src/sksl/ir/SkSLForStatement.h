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
class ForStatement final : public Statement {
public:
    static constexpr Kind kStatementKind = Kind::kFor;

    ForStatement(int offset, std::unique_ptr<Statement> initializer,
                 std::unique_ptr<Expression> test, std::unique_ptr<Expression> next,
                 std::unique_ptr<Statement> statement, std::shared_ptr<SymbolTable> symbols)
    : INHERITED(offset, kStatementKind)
    , fSymbolTable(std::move(symbols))
    , fInitializer(std::move(initializer))
    , fTest(std::move(test))
    , fNext(std::move(next))
    , fStatement(std::move(statement)) {}

    std::unique_ptr<Statement>& initializer() {
        return fInitializer;
    }

    const std::unique_ptr<Statement>& initializer() const {
        return fInitializer;
    }

    std::unique_ptr<Expression>& test() {
        return fTest;
    }

    const std::unique_ptr<Expression>& test() const {
        return fTest;
    }

    std::unique_ptr<Expression>& next() {
        return fNext;
    }

    const std::unique_ptr<Expression>& next() const {
        return fNext;
    }

    std::unique_ptr<Statement>& statement() {
        return fStatement;
    }

    const std::unique_ptr<Statement>& statement() const {
        return fStatement;
    }

    const std::shared_ptr<SymbolTable>& symbols() const {
        return fSymbolTable;
    }

    std::unique_ptr<Statement> clone() const override {
        return std::make_unique<ForStatement>(
                fOffset,
                this->initializer() ? this->initializer()->clone() : nullptr,
                this->test() ? this->test()->clone() : nullptr,
                this->next() ? this->next()->clone() : nullptr,
                this->statement()->clone(),
                SymbolTable::WrapIfBuiltin(this->symbols()));
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
    std::shared_ptr<SymbolTable> fSymbolTable;
    std::unique_ptr<Statement> fInitializer;
    std::unique_ptr<Expression> fTest;
    std::unique_ptr<Expression> fNext;
    std::unique_ptr<Statement> fStatement;

    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
