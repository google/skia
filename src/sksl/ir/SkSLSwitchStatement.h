/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SWITCHSTATEMENT
#define SKSL_SWITCHSTATEMENT

#include "include/private/SkSLStatement.h"
#include "src/sksl/ir/SkSLSwitchCase.h"

#include <memory>
#include <vector>

namespace SkSL {

class Expression;
class SymbolTable;

/**
 * A 'switch' statement.
 */
class SwitchStatement final : public Statement {
public:
    inline static constexpr Kind kStatementKind = Kind::kSwitch;

    SwitchStatement(int line, bool isStatic, std::unique_ptr<Expression> value,
                    StatementArray cases, std::shared_ptr<SymbolTable> symbols)
        : INHERITED(line, kStatementKind)
        , fIsStatic(isStatic)
        , fValue(std::move(value))
        , fCases(std::move(cases))
        , fSymbols(std::move(symbols)) {}

    // Create a `switch` statement with an array of case-values and case-statements.
    // Coerces case values to the proper type and reports an error if cases are duplicated.
    // Reports errors via the ErrorReporter.
    static std::unique_ptr<Statement> Convert(const Context& context,
                                              int line,
                                              bool isStatic,
                                              std::unique_ptr<Expression> value,
                                              ExpressionArray caseValues,
                                              StatementArray caseStatements,
                                              std::shared_ptr<SymbolTable> symbolTable);

    // Create a `switch` statement with an array of SwitchCases. The array of SwitchCases must
    // already contain non-overlapping, correctly-typed case values. Reports errors via ASSERT.
    static std::unique_ptr<Statement> Make(const Context& context,
                                           int line,
                                           bool isStatic,
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

    bool isStatic() const {
        return fIsStatic;
    }

    const std::shared_ptr<SymbolTable>& symbols() const {
        return fSymbols;
    }

    std::unique_ptr<Statement> clone() const override;

    String description() const override;

private:
    bool fIsStatic;
    std::unique_ptr<Expression> fValue;
    StatementArray fCases;  // every Statement inside fCases must be a SwitchCase
    std::shared_ptr<SymbolTable> fSymbols;

    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
