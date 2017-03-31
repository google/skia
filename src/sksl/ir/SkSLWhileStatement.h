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
    WhileStatement(Position position, std::unique_ptr<Expression> test,
                   std::unique_ptr<Statement> statement)
    : INHERITED(position, kWhile_Kind)
    , fTest(std::move(test))
    , fStatement(std::move(statement)) {}

    String description() const override {
        return "while (" + fTest->description() + ") " + fStatement->description();
    }

    std::unique_ptr<Expression> fTest;
    const std::unique_ptr<Statement> fStatement;

    typedef Statement INHERITED;
};

} // namespace

#endif
