/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_IFSTATEMENT
#define SKSL_IFSTATEMENT

#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLStatement.h"

#include <memory>
#include <string>
#include <utility>

namespace SkSL {

class Context;

/**
 * An 'if' statement.
 */
class IfStatement final : public Statement {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kIf;

    IfStatement(Position pos, std::unique_ptr<Expression> test,
                std::unique_ptr<Statement> ifTrue, std::unique_ptr<Statement> ifFalse)
        : INHERITED(pos, kIRNodeKind)
        , fTest(std::move(test))
        , fIfTrue(std::move(ifTrue))
        , fIfFalse(std::move(ifFalse)) {}

    // Creates a potentially-simplified form of the if-statement. Typechecks and coerces the test
    // expression; reports errors via ErrorReporter.
    static std::unique_ptr<Statement> Convert(const Context& context,
                                              Position pos,
                                              std::unique_ptr<Expression> test,
                                              std::unique_ptr<Statement> ifTrue,
                                              std::unique_ptr<Statement> ifFalse);

    // Creates a potentially-simplified form of the if-statement; reports errors via ASSERT.
    static std::unique_ptr<Statement> Make(const Context& context,
                                           Position pos,
                                           std::unique_ptr<Expression> test,
                                           std::unique_ptr<Statement> ifTrue,
                                           std::unique_ptr<Statement> ifFalse);

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

    std::string description() const override;

private:
    std::unique_ptr<Expression> fTest;
    std::unique_ptr<Statement> fIfTrue;
    std::unique_ptr<Statement> fIfFalse;

    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
