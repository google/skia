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
class ExpressionStatement final : public Statement {
public:
    static constexpr Kind kStatementKind = Kind::kExpression;

    ExpressionStatement(std::unique_ptr<Expression> expression)
        : INHERITED(expression->fOffset, kStatementKind)
        , fExpression(std::move(expression)) {}

    const std::unique_ptr<Expression>& expression() const {
        return fExpression;
    }

    std::unique_ptr<Expression>& expression() {
        return fExpression;
    }

    std::unique_ptr<Statement> clone() const override {
        return std::unique_ptr<Statement>(new ExpressionStatement(this->expression()->clone()));
    }

    String description() const override {
        return this->expression()->description() + ";";
    }

private:
    std::unique_ptr<Expression> fExpression;

    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
