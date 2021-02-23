/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SWITCHSTATEMENT
#define SKSL_SWITCHSTATEMENT

#include "src/sksl/ir/SkSLStatement.h"
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
    static constexpr Kind kStatementKind = Kind::kSwitch;

    // Use SwitchStatement::Make to check invariants when creating switch statements.
    SwitchStatement(int offset, bool isStatic, std::unique_ptr<Expression> value,
                    std::vector<std::unique_ptr<SwitchCase>> cases,
                    const std::shared_ptr<SymbolTable> symbols)
        : INHERITED(offset, kStatementKind)
        , fIsStatic(isStatic)
        , fValue(std::move(value))
        , fCases(std::move(cases))
        , fSymbols(std::move(symbols)) {}

    /** Create a `switch` statement with an array of case-values and case-statements.
     * Coerces case values to the proper type and reports an error if cases are duplicated.
     */
    static std::unique_ptr<Statement> Make(const Context& context,
                                           int offset,
                                           bool isStatic,
                                           std::unique_ptr<Expression> value,
                                           ExpressionArray caseValues,
                                           SkTArray<StatementArray> caseStatements,
                                           std::shared_ptr<SymbolTable> symbolTable);

    /**
     * Create a `switch` statement with an array of SwitchCases.
     * The array of SwitchCases must already contain non-overlapping, correctly-typed case values.
     */
    static std::unique_ptr<Statement> Make(const Context& context,
                                           int offset,
                                           bool isStatic,
                                           std::unique_ptr<Expression> value,
                                           std::vector<std::unique_ptr<SwitchCase>> cases,
                                           std::shared_ptr<SymbolTable> symbolTable);

    /**
     * Returns a block containing all of the statements that will be run if the given case matches
     * (which, owing to the statements being owned by unique_ptrs, means the switch itself will be
     * disassembled by this call and must then be discarded).
     * Returns null (and leaves the switch unmodified) if no such simple reduction is possible, such
     * as when break statements appear inside conditionals.
     */
    static std::unique_ptr<Statement> BlockForCase(std::vector<std::unique_ptr<SwitchCase>>* cases,
                                                   SwitchCase* caseToCapture,
                                                   std::shared_ptr<SymbolTable> symbolTable);

    std::unique_ptr<Expression>& value() {
        return fValue;
    }

    const std::unique_ptr<Expression>& value() const {
        return fValue;
    }

    std::vector<std::unique_ptr<SwitchCase>>& cases() {
        return fCases;
    }

    const std::vector<std::unique_ptr<SwitchCase>>& cases() const {
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
    std::vector<std::unique_ptr<SwitchCase>> fCases;
    std::shared_ptr<SymbolTable> fSymbols;

    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
