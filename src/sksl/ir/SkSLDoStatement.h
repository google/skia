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
    DoStatement(Position position, sk_up<Statement> statement, sk_up<Expression> test)
            : INHERITED(position, kDo_Kind)
            , fStatement(std::move(statement))
            , fTest(std::move(test)) {}

    SkString description() const override {
        return "do " + fStatement->description() + " while (" + fTest->description() + ");";
    }

    const sk_up<Statement> fStatement;
    const sk_up<Expression> fTest;

    typedef Statement INHERITED;
};

} // namespace

#endif
