/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_RETURNSTATEMENT
#define SKSL_RETURNSTATEMENT

#include "include/private/SkSLStatement.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * A 'return' statement.
 */
class ReturnStatement final : public Statement {
public:
    static constexpr Kind kStatementKind = Kind::kReturn;

    ReturnStatement(int offset, std::unique_ptr<Expression> expression)
        : INHERITED(offset, kStatementKind)
        , fExpression(std::move(expression)) {}

    static std::unique_ptr<Statement> Make(int offset, std::unique_ptr<Expression> expression) {
        return std::make_unique<ReturnStatement>(offset, std::move(expression));
    }

    std::unique_ptr<Expression>& expression() {
        return fExpression;
    }

    const std::unique_ptr<Expression>& expression() const {
        return fExpression;
    }

    void setExpression(std::unique_ptr<Expression> expr) {
        fExpression = std::move(expr);
    }

    std::unique_ptr<Statement> clone() const override {
        return std::make_unique<ReturnStatement>(fOffset, this->expression()->clone());
    }

    String description() const override {
        if (this->expression()) {
            return "return " + this->expression()->description() + ";";
        } else {
            return String("return;");
        }
    }

private:
    std::unique_ptr<Expression> fExpression;

    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
