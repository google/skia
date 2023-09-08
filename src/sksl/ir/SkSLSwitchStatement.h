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
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLStatement.h"

#include <memory>
#include <string>
#include <utility>

namespace SkSL {

class Context;
class SwitchCase;
class SymbolTable;

/**
 * A 'switch' statement.
 */
class SwitchStatement final : public Statement {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kSwitch;

    SwitchStatement(Position pos,
                    std::unique_ptr<Expression> value,
                    StatementArray cases,
                    std::shared_ptr<SymbolTable> symbols)
            : INHERITED(pos, kIRNodeKind)
            , fValue(std::move(value))
            , fCases(std::move(cases))
            , fSymbols(std::move(symbols)) {}

    // Create a `switch` statement with an array of case-values and case-statements.
    // Coerces case values to the proper type and reports an error if cases are duplicated.
    // Reports errors via the ErrorReporter.
    static std::unique_ptr<Statement> Convert(const Context& context,
                                              Position pos,
                                              std::unique_ptr<Expression> value,
                                              ExpressionArray caseValues,
                                              StatementArray caseStatements);

    // Create a `switch` statement with an array of SwitchCases. The array of SwitchCases must
    // already contain non-overlapping, correctly-typed case values. Reports errors via ASSERT.
    static std::unique_ptr<Statement> Make(const Context& context,
                                           Position pos,
                                           std::unique_ptr<Expression> value,
                                           StatementArray cases,
                                           std::shared_ptr<SymbolTable> symbolTable);

    // Returns a block containing all of the statements that will be run if the given case matches
    // (which, owing to the statements being owned by unique_ptrs, means the switch itself will be
    // disassembled by this call and must then be discarded).
    // Returns null (and leaves the switch unmodified) if no such simple reduction is possible, such
    // as when break statements appear inside conditionals.
    static std::unique_ptr<Statement> BlockForCase(StatementArray* cases,
                                                   SwitchCase* caseToCapture,
                                                   std::shared_ptr<SymbolTable> symbolTable);

    std::unique_ptr<Expression>& value() {
        return fValue;
    }

    const std::unique_ptr<Expression>& value() const {
        return fValue;
    }

    StatementArray& cases() {
        return fCases;
    }

    const StatementArray& cases() const {
        return fCases;
    }

    const std::shared_ptr<SymbolTable>& symbols() const {
        return fSymbols;
    }

    std::unique_ptr<Statement> clone() const override;

    std::string description() const override;

private:
    std::unique_ptr<Expression> fValue;
    StatementArray fCases;  // every Statement inside fCases must be a SwitchCase
    std::shared_ptr<SymbolTable> fSymbols;

    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
