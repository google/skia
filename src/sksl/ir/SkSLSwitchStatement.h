/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SWITCHSTATEMENT
#define SKSL_SWITCHSTATEMENT

#include "src/sksl/SkSLDefines.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLStatement.h"

#include <memory>
#include <string>
#include <utility>

namespace SkSL {

class Context;
class SymbolTable;

/**
 * A 'switch' statement.
 */
class SwitchStatement final : public Statement {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kSwitch;

    SwitchStatement(Position pos,
                    std::unique_ptr<Expression> value,
                    std::unique_ptr<Statement> caseBlock)
            : INHERITED(pos, kIRNodeKind)
            , fValue(std::move(value))
            , fCaseBlock(std::move(caseBlock)) {}

    // Create a `switch` statement with an array of case-values and case-statements.
    // Coerces case values to the proper type and reports an error if cases are duplicated.
    // Reports errors via the ErrorReporter.
    static std::unique_ptr<Statement> Convert(const Context& context,
                                              Position pos,
                                              std::unique_ptr<Expression> value,
                                              ExpressionArray caseValues,
                                              StatementArray caseStatements,
                                              std::unique_ptr<SymbolTable> symbolTable);

    // Create a `switch` statement with a Block containing only SwitchCases. The SwitchCase block
    // must already contain non-overlapping, correctly-typed case values. Reports errors via ASSERT.
    static std::unique_ptr<Statement> Make(const Context& context,
                                           Position pos,
                                           std::unique_ptr<Expression> value,
                                           std::unique_ptr<Statement> caseBlock);

    std::unique_ptr<Expression>& value() {
        return fValue;
    }

    const std::unique_ptr<Expression>& value() const {
        return fValue;
    }

    std::unique_ptr<Statement>& caseBlock() {
        return fCaseBlock;
    }

    const std::unique_ptr<Statement>& caseBlock() const {
        return fCaseBlock;
    }

    StatementArray& cases() {
        return fCaseBlock->as<Block>().children();
    }

    const StatementArray& cases() const {
        return fCaseBlock->as<Block>().children();
    }

    std::string description() const override;

private:
    std::unique_ptr<Expression> fValue;
    std::unique_ptr<Statement> fCaseBlock; // must be a Block containing only SwitchCase statements

    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
