/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_EXPRESSIONSTATEMENT
#define SKSL_EXPRESSIONSTATEMENT

#include "SkSLExpression.h"
#include "SkSLStatement.h"

namespace SkSL {

/**
 * A lone expression being used as a statement.
 */
struct ExpressionStatement : public Statement {
    ExpressionStatement(std::unique_ptr<Expression> expression)
    : INHERITED(expression->fPosition, kExpression_Kind)
    , fExpression(std::move(expression)) {}

    String description() const override {
        return fExpression->description() + ";";
    }

    std::unique_ptr<Expression> fExpression;

    typedef Statement INHERITED;
};

} // namespace

#endif
