/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_EXPRESSIONSTATEMENT
#define SKSL_EXPRESSIONSTATEMENT

#include "include/private/SkSLStatement.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * A lone expression being used as a statement.
 */
class ExpressionStatement final : public Statement {
public:
    inline static constexpr Kind kStatementKind = Kind::kExpression;

    ExpressionStatement(std::unique_ptr<Expression> expression)
        : INHERITED(expression->fLine, kStatementKind)
        , fExpression(std::move(expression)) {}

    // Creates an SkSL expression-statement. Note that there is never any type-coercion and no error
    // cases are reported; any Expression can be an ExpressionStatement.
    static std::unique_ptr<Statement> Make(const Context& context,
                                           std::unique_ptr<Expression> expr);

    const std::unique_ptr<Expression>& expression() const {
        return fExpression;
    }

    std::unique_ptr<Expression>& expression() {
        return fExpression;
    }

    std::unique_ptr<Statement> clone() const override {
        return std::make_unique<ExpressionStatement>(this->expression()->clone());
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
