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
    ForStatement(Position position, sk_up<Statement> initializer, sk_up<Expression> test,
                 sk_up<Expression> next, sk_up<Statement> statement,
                 std::shared_ptr<SymbolTable> symbols)
            : INHERITED(position, kFor_Kind)
            , fInitializer(std::move(initializer))
            , fTest(std::move(test))
            , fNext(std::move(next))
            , fStatement(std::move(statement))
            , fSymbols(symbols) {}

    SkString description() const override {
        SkString result("for (");
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

    const sk_up<Statement> fInitializer;
    const sk_up<Expression> fTest;
    const sk_up<Expression> fNext;
    const sk_up<Statement> fStatement;
    const std::shared_ptr<SymbolTable> fSymbols;

    typedef Statement INHERITED;
};

} // namespace

#endif
