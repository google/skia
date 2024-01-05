/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_EXPRESSIONSTATEMENT
#define SKSL_EXPRESSIONSTATEMENT

#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLStatement.h"

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

    std::string description() const override;

private:
    std::unique_ptr<Expression> fExpression;

    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
