/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLSwitchStatement.h"

#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTo.h"
#include "src/core/SkTHash.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLBreakStatement.h"
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLSwitchCase.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/transform/SkSLProgramWriter.h"
#include "src/sksl/transform/SkSLTransform.h"

#include <algorithm>
#include <iterator>

using namespace skia_private;

namespace SkSL {

std::string SwitchStatement::description() const {
    return "switch (" + this->value()->description() + ") " + this->caseBlock()->description();
}

static TArray<const SwitchCase*> find_duplicate_case_values(const StatementArray& cases) {
    TArray<const SwitchCase*> duplicateCases;
    THashSet<SKSL_INT> intValues;
    bool foundDefault = false;

    for (const std::unique_ptr<Statement>& stmt : cases) {
        const SwitchCase* sc = &stmt->as<SwitchCase>();
        if (sc->isDefault()) {
            if (foundDefault) {
                duplicateCases.push_back(sc);
                continue;
            }
            foundDefault = true;
        } else {
            SKSL_INT value = sc->value();
            if (intValues.contains(value)) {
                duplicateCases.push_back(sc);
                continue;
            }
            intValues.add(value);
        }
    }

    return duplicateCases;
}

static void remove_break_statements(std::unique_ptr<Statement>& stmt) {
    class RemoveBreaksWriter : public ProgramWriter {
    public:
        bool visitStatementPtr(std::unique_ptr<Statement>& stmt) override {
            if (stmt->is<BreakStatement>()) {
                stmt = Nop::Make();
                return false;
            }
            return ProgramWriter::visitStatementPtr(stmt);
        }

        bool visitExpressionPtr(std::unique_ptr<Expression>& expr) override {
            return false;
        }
    };
    RemoveBreaksWriter{}.visitStatementPtr(stmt);
}

static bool block_for_case(Statement* caseBlock, SwitchCase* caseToCapture) {
    // This function reduces a switch to the matching case (or cases, if fallthrough occurs) when
    // the switch-value is known and no conditional breaks exist. If conversion is not possible,
    // false is returned and no changes are made. Conversion can fail if the switch contains
    // conditional breaks.
    //
    // We have to be careful to not move any of the pointers until after we're sure we're going to
    // succeed, so before we make any changes at all, we check the switch-cases to decide on a plan
    // of action.
    //
    // First, we identify the code that would be run if the switch's value matches `caseToCapture`.
    StatementArray& cases = caseBlock->as<Block>().children();
    auto iter = cases.begin();
    for (; iter != cases.end(); ++iter) {
        const SwitchCase& sc = (*iter)->as<SwitchCase>();
        if (&sc == caseToCapture) {
            break;
        }
    }

    // Next, walk forward through the rest of the switch. If we find a conditional break, we're
    // stuck and can't simplify at all. If we find an unconditional break, we have a range of
    // statements that we can use for simplification.
    auto startIter = iter;
    bool removeBreakStatements = false;
    for (; iter != cases.end(); ++iter) {
        std::unique_ptr<Statement>& stmt = (*iter)->as<SwitchCase>().statement();
        if (Analysis::SwitchCaseContainsConditionalExit(*stmt)) {
            // We can't reduce switch-cases to a block when they have conditional exits.
            return false;
        }
        if (Analysis::SwitchCaseContainsUnconditionalExit(*stmt)) {
            // We found an unconditional exit. We can use this block, but we'll need to strip
            // out the break statement if there is one.
            removeBreakStatements = true;
            ++iter;
            break;
        }
    }

    // We fell off the bottom of the switch or encountered a break. Next, we must strip down
    // `caseBlock` to hold only the statements needed to execute `caseToCapture`. To do this, we
    // eliminate the SwitchCase elements. This converts each `case n: stmt;` element into just
    // `stmt;`. While doing this, we also move the elements to the front of the array if they
    // weren't already there.
    int numElements = SkToInt(std::distance(startIter, iter));
    for (int index = 0; index < numElements; ++index, ++startIter) {
        cases[index] = std::move((*startIter)->as<SwitchCase>().statement());
    }

    // Next, we shrink the statement array to destroy the excess statements.
    cases.pop_back_n(cases.size() - numElements);

    // If we found an unconditional break at the end, we need to eliminate that break.
    if (removeBreakStatements) {
        remove_break_statements(cases.back());
    }

    // We've stripped down `caseBlock` to contain only the captured case. Return true.
    return true;
}

std::unique_ptr<Statement> SwitchStatement::Convert(const Context& context,
                                                    Position pos,
                                                    std::unique_ptr<Expression> value,
                                                    ExpressionArray caseValues,
                                                    StatementArray caseStatements,
                                                    std::unique_ptr<SymbolTable> symbolTable) {
    SkASSERT(caseValues.size() == caseStatements.size());

    value = context.fTypes.fInt->coerceExpression(std::move(value), context);
    if (!value) {
        return nullptr;
    }

    StatementArray cases;
    for (int i = 0; i < caseValues.size(); ++i) {
        if (caseValues[i]) {
            Position casePos = caseValues[i]->fPosition;
            // Case values must be constant integers of the same type as the switch value
            std::unique_ptr<Expression> caseValue = value->type().coerceExpression(
                    std::move(caseValues[i]), context);
            if (!caseValue) {
                return nullptr;
            }
            SKSL_INT intValue;
            if (!ConstantFolder::GetConstantInt(*caseValue, &intValue)) {
                context.fErrors->error(casePos, "case value must be a constant integer");
                return nullptr;
            }
            cases.push_back(SwitchCase::Make(casePos, intValue, std::move(caseStatements[i])));
        } else {
            cases.push_back(SwitchCase::MakeDefault(pos, std::move(caseStatements[i])));
        }
    }

    // Detect duplicate `case` labels and report an error.
    TArray<const SwitchCase*> duplicateCases = find_duplicate_case_values(cases);
    if (!duplicateCases.empty()) {
        for (const SwitchCase* sc : duplicateCases) {
            if (sc->isDefault()) {
                context.fErrors->error(sc->fPosition, "duplicate default case");
            } else {
                context.fErrors->error(sc->fPosition, "duplicate case value '" +
                                                      std::to_string(sc->value()) + "'");
            }
        }
        return nullptr;
    }

    // If a switch-case has variable declarations at its top level, we want to create a scoped block
    // around the switch, then move the variable declarations out of the switch body and into the
    // outer scope. This prevents scoping issues in backends which don't offer a native switch.
    // (skia:14375) It also allows static-switch optimization to work properly when variables are
    // inherited from earlier fall-through cases. (oss-fuzz:70589)
    std::unique_ptr<Block> block =
            Transform::HoistSwitchVarDeclarationsAtTopLevel(context, cases, *symbolTable, pos);

    std::unique_ptr<Statement> switchStmt = SwitchStatement::Make(
            context, pos, std::move(value),
            Block::MakeBlock(pos, std::move(cases), Block::Kind::kBracedScope,
                             std::move(symbolTable)));
    if (block) {
        // Add the switch statement to the end of the var-decl block.
        block->children().push_back(std::move(switchStmt));
        return block;
    } else {
        // Return the switch statement directly.
        return switchStmt;
    }
}

std::unique_ptr<Statement> SwitchStatement::Make(const Context& context,
                                                 Position pos,
                                                 std::unique_ptr<Expression> value,
                                                 std::unique_ptr<Statement> caseBlock) {
    // Confirm that every statement in `cases` is a SwitchCase.
    const StatementArray& cases = caseBlock->as<Block>().children();
    SkASSERT(std::all_of(cases.begin(), cases.end(), [&](const std::unique_ptr<Statement>& stmt) {
        return stmt->is<SwitchCase>();
    }));

    // Confirm that every switch-case value is unique.
    SkASSERT(find_duplicate_case_values(cases).empty());

    // Flatten switch statements if we're optimizing, and the value is known
    if (context.fConfig->fSettings.fOptimize) {
        SKSL_INT switchValue;
        if (ConstantFolder::GetConstantInt(*value, &switchValue)) {
            SwitchCase* defaultCase = nullptr;
            SwitchCase* matchingCase = nullptr;
            for (const std::unique_ptr<Statement>& stmt : cases) {
                SwitchCase& sc = stmt->as<SwitchCase>();
                if (sc.isDefault()) {
                    defaultCase = &sc;
                    continue;
                }

                if (sc.value() == switchValue) {
                    matchingCase = &sc;
                    break;
                }
            }

            if (!matchingCase) {
                // No case value matches the switch value.
                if (!defaultCase) {
                    // No default switch-case exists; the switch had no effect. We can eliminate the
                    // body of the switch entirely.
                    // There's still value in preserving the symbol table here, particularly when
                    // the input program is malformed, so we keep the Block itself. (oss-fuzz:70613)
                    caseBlock->as<Block>().children().clear();
                    return caseBlock;
                }
                // We had a default case; that's what we matched with.
                matchingCase = defaultCase;
            }

            // Strip down our case block to contain only the matching case, if we can.
            if (block_for_case(caseBlock.get(), matchingCase)) {
                return caseBlock;
            }
        }
    }

    // The switch couldn't be optimized away; emit it normally.
    return std::make_unique<SwitchStatement>(pos, std::move(value), std::move(caseBlock));
}

}  // namespace SkSL
