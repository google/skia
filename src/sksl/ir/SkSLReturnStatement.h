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
    inline static constexpr Kind kIRNodeKind = Kind::kReturn;

    ReturnStatement(Position pos, std::unique_ptr<Expression> expression)
        : INHERITED(pos, kIRNodeKind)
        , fExpression(std::move(expression)) {}

    static std::unique_ptr<Statement> Make(Position pos, std::unique_ptr<Expression> expression) {
        return std::make_unique<ReturnStatement>(pos, std::move(expression));
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

    std::string description() const override {
        if (this->expression()) {
            return "return " + this->expression()->description() + ";";
        } else {
            return "return;";
        }
    }

private:
    std::unique_ptr<Expression> fExpression;

    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
