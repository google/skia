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

    // Creates an SkSL for loop.
    static std::unique_ptr<Statement> Make(const Context& context, int offset,
                                           std::unique_ptr<Statement> initializer,
                                           std::unique_ptr<Expression> test,
                                           std::unique_ptr<Expression> next,
                                           std::unique_ptr<Statement> statement,
                                           std::shared_ptr<SymbolTable> symbolTable);

    // A while statement is represented in SkSL as a for loop with no init-stmt or next-expr.
    static std::unique_ptr<Statement> MakeWhile(const Context& context, int offset,
                                                std::unique_ptr<Expression> test,
                                                std::unique_ptr<Statement> statement,
                                                std::shared_ptr<SymbolTable> symbolTable);

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

    std::unique_ptr<Statement> clone() const override;

    String description() const override;

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
