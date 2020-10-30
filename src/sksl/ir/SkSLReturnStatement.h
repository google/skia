/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_RETURNSTATEMENT
#define SKSL_RETURNSTATEMENT

#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLStatement.h"

namespace SkSL {

/**
 * A 'return' statement.
 */
class ReturnStatement final : public Statement {
public:
    static constexpr Kind kStatementKind = Kind::kReturn;

    ReturnStatement(int offset)
        : INHERITED(offset, kStatementKind) {}

    ReturnStatement(std::unique_ptr<Expression> expression)
        : INHERITED(expression->fOffset, kStatementKind)
        , fExpression(std::move(expression)) {}

    std::unique_ptr<Expression>& expression() {
        return fExpression;
    }

    const std::unique_ptr<Expression>& expression() const {
        return fExpression;
    }

    std::unique_ptr<Statement> clone() const override {
        if (this->expression()) {
            return std::unique_ptr<Statement>(new ReturnStatement(this->expression()->clone()));
        }
        return std::unique_ptr<Statement>(new ReturnStatement(fOffset));
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
