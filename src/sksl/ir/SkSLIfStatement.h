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
class IfStatement final : public Statement {
public:
    static constexpr Kind kStatementKind = Kind::kIf;

    IfStatement(int offset, bool isStatic, std::unique_ptr<Expression> test,
                std::unique_ptr<Statement> ifTrue, std::unique_ptr<Statement> ifFalse)
        : INHERITED(offset, kStatementKind)
        , fTest(std::move(test))
        , fIfTrue(std::move(ifTrue))
        , fIfFalse(std::move(ifFalse))
        , fIsStatic(isStatic) {}

    bool isStatic() const {
        return fIsStatic;
    }

    std::unique_ptr<Expression>& test() {
        return fTest;
    }

    const std::unique_ptr<Expression>& test() const {
        return fTest;
    }

    std::unique_ptr<Statement>& ifTrue() {
        return fIfTrue;
    }

    const std::unique_ptr<Statement>& ifTrue() const {
        return fIfTrue;
    }

    std::unique_ptr<Statement>& ifFalse() {
        return fIfFalse;
    }

    const std::unique_ptr<Statement>& ifFalse() const {
        return fIfFalse;
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

private:
    std::unique_ptr<Expression> fTest;
    std::unique_ptr<Statement> fIfTrue;
    std::unique_ptr<Statement> fIfFalse;
    bool fIsStatic;

    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
