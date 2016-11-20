/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_FORSTATEMENT
#define SKSL_FORSTATEMENT

#include "SkSLExpression.h"
#include "SkSLStatement.h"
#include "SkSLSymbolTable.h"

namespace SkSL {

/**
 * A 'for' statement.
 */
struct ForStatement : public Statement {
    ForStatement(Position position, std::unique_ptr<Statement> initializer, 
                 std::unique_ptr<Expression> test, std::unique_ptr<Expression> next, 
                 std::unique_ptr<Statement> statement, std::shared_ptr<SymbolTable> symbols)
    : INHERITED(position, kFor_Kind)
    , fInitializer(std::move(initializer))
    , fTest(std::move(test))
    , fNext(std::move(next))
    , fStatement(std::move(statement))
    , fSymbols(symbols) {}

    std::string description() const override {
        std::string result = "for (";
        if (fInitializer) {
            result += fInitializer->description();
        } 
        result += " ";
        if (fTest) {
            result += fTest->description();
        } 
        result += "; ";
        if (fNext) {
            result += fNext->description();
        }
        result += ") " + fStatement->description();
        return result;
    }

    const std::unique_ptr<Statement> fInitializer;
    const std::unique_ptr<Expression> fTest;
    const std::unique_ptr<Expression> fNext;
    const std::unique_ptr<Statement> fStatement;
    const std::shared_ptr<SymbolTable> fSymbols;

    typedef Statement INHERITED;
};

} // namespace

#endif
