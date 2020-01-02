/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_EXPRESSIONSTATEMENT
#define SKSL_EXPRESSIONSTATEMENT

#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLStatement.h"

namespace SkSL {

/**
 * A lone expression being used as a statement.
 */
struct ExpressionStatement : public Statement {
    ExpressionStatement(std::unique_ptr<Expression> expression)
    : INHERITED(expression->fOffset, kExpression_Kind)
    , fExpression(std::move(expression)) {}

    std::unique_ptr<Statement> clone() const override {
        return std::unique_ptr<Statement>(new ExpressionStatement(fExpression->clone()));
    }

#ifdef SK_DEBUG
    String description() const override {
        return fExpression->description() + ";";
    }
#endif

    std::unique_ptr<Expression> fExpression;

    typedef Statement INHERITED;
};

} // namespace

#endif
