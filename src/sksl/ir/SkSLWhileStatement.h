/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_WHILESTATEMENT
#define SKSL_WHILESTATEMENT

#include "SkSLExpression.h"
#include "SkSLStatement.h"

namespace SkSL {

/**
 * A 'while' loop.
 */
struct WhileStatement : public Statement {
    WhileStatement(Position position, sk_up<Expression> test, sk_up<Statement> statement)
            : INHERITED(position, kWhile_Kind)
            , fTest(std::move(test))
            , fStatement(std::move(statement)) {}

    SkString description() const override {
        return "while (" + fTest->description() + ") " + fStatement->description();
    }

    const sk_up<Expression> fTest;
    const sk_up<Statement> fStatement;

    typedef Statement INHERITED;
};

} // namespace

#endif
