/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_EXPRESSIONSTATEMENT
#define SKSL_EXPRESSIONSTATEMENT

#include "include/private/SkSLIRNode.h"
#include "include/private/SkSLStatement.h"
#include "src/sksl/ir/SkSLExpression.h"

#include <memory>
#include <string>
#include <utility>

namespace SkSL {

class Context;

/**
 * A lone expression being used as a statement.
 */
class ExpressionStatement final : public Statement {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kExpression;

    ExpressionStatement(std::unique_ptr<Expression> expression)
        : INHERITED(expression->fPosition, kIRNodeKind)
        , fExpression(std::move(expression)) {}

    // Creates an SkSL expression-statement; reports errors via ErrorReporter.
    static std::unique_ptr<Statement> Convert(const Context& context,
                                              std::unique_ptr<Expression> expr);

    // Creates an SkSL expression-statement; reports errors via assertion.
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

    std::string description() const override {
        return this->expression()->description() + ";";
    }

private:
    std::unique_ptr<Expression> fExpression;

    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
