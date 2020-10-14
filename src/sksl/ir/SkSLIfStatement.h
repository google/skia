/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_IFSTATEMENT
#define SKSL_IFSTATEMENT

#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLStatement.h"

namespace SkSL {

/**
 * An 'if' statement.
 */
struct IfStatement : public Statement {
    static constexpr Kind kStatementKind = Kind::kIf;

    IfStatement(int offset, bool isStatic, std::unique_ptr<Expression> test,
                std::unique_ptr<Statement> ifTrue, std::unique_ptr<Statement> ifFalse)
    : INHERITED(offset, IfStatementData{isStatic}) {
        fExpressionChildren.push_back(std::move(test));
        fStatementChildren.reserve_back(2);
        fStatementChildren.push_back(std::move(ifTrue));
        fStatementChildren.push_back(std::move(ifFalse));
    }

    bool isStatic() const {
        return this->ifStatementData().fIsStatic;
    }

    std::unique_ptr<Expression>& test() {
        return this->fExpressionChildren[0];
    }

    const std::unique_ptr<Expression>& test() const {
        return this->fExpressionChildren[0];
    }

    std::unique_ptr<Statement>& ifTrue() {
        return this->fStatementChildren[0];
    }

    const std::unique_ptr<Statement>& ifTrue() const {
        return this->fStatementChildren[0];
    }

    std::unique_ptr<Statement>& ifFalse() {
        return this->fStatementChildren[1];
    }

    const std::unique_ptr<Statement>& ifFalse() const {
        return this->fStatementChildren[1];
    }

    std::unique_ptr<Statement> clone() const override {
        return std::unique_ptr<Statement>(new IfStatement(fOffset, this->isStatic(),
                                                          this->test()->clone(),
                                                          this->ifTrue()->clone(),
                                                          this->ifFalse() ? this->ifFalse()->clone()
                                                                          : nullptr));
    }

    String description() const override {
        String result;
        if (this->isStatic()) {
            result += "@";
        }
        result += "if (" + this->test()->description() + ") " + this->ifTrue()->description();
        if (this->ifFalse()) {
            result += " else " + this->ifFalse()->description();
        }
        return result;
    }

    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
