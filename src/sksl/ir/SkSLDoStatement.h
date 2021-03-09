/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DOSTATEMENT
#define SKSL_DOSTATEMENT

#include "include/private/SkSLStatement.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * A 'do' statement.
 */
class DoStatement final : public Statement {
public:
    static constexpr Kind kStatementKind = Kind::kDo;

    DoStatement(int offset, std::unique_ptr<Statement> statement, std::unique_ptr<Expression> test)
        : INHERITED(offset, kStatementKind)
        , fStatement(std::move(statement))
        , fTest(std::move(test)) {}

    // Creates an SkSL do-while loop; uses the ErrorReporter to report errors.
    static std::unique_ptr<Statement> Convert(const Context& context,
                                              std::unique_ptr<Statement> stmt,
                                              std::unique_ptr<Expression> test);

    // Creates an SkSL do-while loop; reports errors via ASSERT.
    static std::unique_ptr<Statement> Make(const Context& context,
                                           std::unique_ptr<Statement> stmt,
                                           std::unique_ptr<Expression> test);

    std::unique_ptr<Statement>& statement() {
        return fStatement;
    }

    const std::unique_ptr<Statement>& statement() const {
        return fStatement;
    }

    std::unique_ptr<Expression>& test() {
        return fTest;
    }

    const std::unique_ptr<Expression>& test() const {
        return fTest;
    }

    std::unique_ptr<Statement> clone() const override;

    String description() const override;

private:
    std::unique_ptr<Statement> fStatement;
    std::unique_ptr<Expression> fTest;

    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
