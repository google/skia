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
struct ForStatement : public Statement {
    static constexpr Kind kIRNodeKind = kFor_Kind;

    ForStatement(int offset, std::unique_ptr<Statement> initializer,
                 std::unique_ptr<Expression> test, std::unique_ptr<Expression> next,
                 std::unique_ptr<Statement> statement, std::shared_ptr<SymbolTable> symbols)
    : INHERITED(offset, kIRNodeKind)
    , fSymbols(symbols)
    , fInitializer(std::move(initializer))
    , fTest(std::move(test))
    , fNext(std::move(next))
    , fStatement(std::move(statement)) {}

    std::unique_ptr<IRNode> clone() const override {
        return std::unique_ptr<IRNode>(new ForStatement(
                                                     fOffset,
                                                     fInitializer ? fInitializer->cloneStatement()
                                                                  : nullptr,
                                                     fTest ? fTest->cloneExpression() : nullptr,
                                                     fNext ? fNext->cloneExpression() : nullptr,
                                                     fStatement->cloneStatement(),
                                                     fSymbols));
    }

    String description() const override {
        String result("for (");
        if (fInitializer) {
            result += fInitializer->description();
        } else {
            result += ";";
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

    // it's important to keep fSymbols defined first (and thus destroyed last) because destroying
    // the other fields can update symbol reference counts
    const std::shared_ptr<SymbolTable> fSymbols;
    std::unique_ptr<Statement> fInitializer;
    std::unique_ptr<Expression> fTest;
    std::unique_ptr<Expression> fNext;
    std::unique_ptr<Statement> fStatement;

    typedef Statement INHERITED;
};

}  // namespace SkSL

#endif
