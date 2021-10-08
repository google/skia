/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_IFSTATEMENT
#define SKSL_IFSTATEMENT

#include "include/private/SkSLStatement.h"
#include "src/sksl/ir/SkSLExpression.h"

#include <memory>

namespace SkSL {

/**
 * An 'if' statement.
 */
class IfStatement final : public Statement {
public:
    inline static constexpr Kind kStatementKind = Kind::kIf;

    IfStatement(int line, bool isStatic, std::unique_ptr<Expression> test,
                std::unique_ptr<Statement> ifTrue, std::unique_ptr<Statement> ifFalse)
        : INHERITED(line, kStatementKind)
        , fTest(std::move(test))
        , fIfTrue(std::move(ifTrue))
        , fIfFalse(std::move(ifFalse))
        , fIsStatic(isStatic) {}

    // Creates a potentially-simplified form of the if-statement. Typechecks and coerces the test
    // expression; reports errors via ErrorReporter.
    static std::unique_ptr<Statement> Convert(const Context& context, int line, bool isStatic,
                                              std::unique_ptr<Expression> test,
                                              std::unique_ptr<Statement> ifTrue,
                                              std::unique_ptr<Statement> ifFalse);

    // Creates a potentially-simplified form of the if-statement; reports errors via ASSERT.
    static std::unique_ptr<Statement> Make(const Context& context, int line, bool isStatic,
                                           std::unique_ptr<Expression> test,
                                           std::unique_ptr<Statement> ifTrue,
                                           std::unique_ptr<Statement> ifFalse);

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

    std::unique_ptr<Statement> clone() const override;

    String description() const override;

private:
    std::unique_ptr<Expression> fTest;
    std::unique_ptr<Statement> fIfTrue;
    std::unique_ptr<Statement> fIfFalse;
    bool fIsStatic;

    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
