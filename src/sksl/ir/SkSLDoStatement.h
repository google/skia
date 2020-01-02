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
struct DoStatement : public Statement {
    DoStatement(int offset, std::unique_ptr<Statement> statement,
                std::unique_ptr<Expression> test)
    : INHERITED(offset, kDo_Kind)
    , fStatement(std::move(statement))
    , fTest(std::move(test)) {}

    std::unique_ptr<Statement> clone() const override {
        return std::unique_ptr<Statement>(new DoStatement(fOffset, fStatement->clone(),
                                                          fTest->clone()));
    }

#ifdef SK_DEBUG
    String description() const override {
        return "do " + fStatement->description() + " while (" + fTest->description() + ");";
    }
#endif

    std::unique_ptr<Statement> fStatement;
    std::unique_ptr<Expression> fTest;

    typedef Statement INHERITED;
};

} // namespace

#endif
