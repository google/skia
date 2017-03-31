/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DOSTATEMENT
#define SKSL_DOSTATEMENT

#include "SkSLExpression.h"
#include "SkSLStatement.h"

namespace SkSL {

/**
 * A 'do' statement.
 */
struct DoStatement : public Statement {
    DoStatement(Position position, std::unique_ptr<Statement> statement,
                std::unique_ptr<Expression> test)
    : INHERITED(position, kDo_Kind)
    , fStatement(std::move(statement))
    , fTest(std::move(test)) {}

    String description() const override {
        return "do " + fStatement->description() + " while (" + fTest->description() + ");";
    }

    const std::unique_ptr<Statement> fStatement;
    std::unique_ptr<Expression> fTest;

    typedef Statement INHERITED;
};

} // namespace

#endif
