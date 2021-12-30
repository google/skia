/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLSwitchStatement.h"

#include <forward_list>

#include "include/private/SkTHash.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

std::unique_ptr<Statement> SwitchStatement::clone() const {
    StatementArray cases;
    cases.reserve_back(this->cases().size());
    for (const std::unique_ptr<Statement>& stmt : this->cases()) {
        cases.push_back(stmt->clone());
    }
    return std::make_unique<SwitchStatement>(fLine,
                                             this->isStatic(),
                                             this->value()->clone(),
                                             std::move(cases),
                                             SymbolTable::WrapIfBuiltin(this->symbols()));
}

String SwitchStatement::description() const {
    String result;
    if (this->isStatic()) {
        result += "@";
    }
    result += String::printf("switch (%s) {\n", this->value()->description().c_str());
    for (const auto& c : this->cases()) {
        result += c->description();
    }
    result += "}";
    return result;
}

static std::forward_list<const SwitchCase*> find_duplicate_case_values(
        const StatementArray& cases) {
    std::forward_list<const SwitchCase*> duplicateCases;
    SkTHashSet<SKSL_INT> intValues;
    bool foundDefault = false;

    for (const std::unique_ptr<Statement>& stmt : cases) {
        const SwitchCase* sc = &stmt->as<SwitchCase>();
        const std::unique_ptr<Expression>& valueExpr = sc->value();

        // A null case-value indicates the `default` switch-case.
        if (!valueExpr) {
            if (foundDefault) {
                duplicateCases.push_front(sc);
                continue;
            }
            foundDefault = true;
            continue;
        }

        // GetConstantInt already succeeded when the SwitchCase was first assembled, so it should
        // succeed this time too.
        SKSL_INT intValue = 0;
        SkAssertResult(ConstantFolder::GetConstantInt(*valueExpr, &intValue));
        if (intValues.contains(intValue)) {
            duplicateCases.push_front(sc);
            continue;
        }
        intValues.add(intValue);
    }

    return duplicateCases;
}

static void move_all_but_break(std::unique_ptr<Statement>& stmt, StatementArray* target) {
    switch (stmt->kind()) {
        case Statement::Kind::kBlock: {
            // Recurse into the block.
            Block& block = stmt->as<Block>();

            StatementArray blockStmts;
            blockStmts.reserve_back(block.children().size());
            for (std::unique_ptr<Statement>& blockStmt : block.children()) {
                move_all_but_break(blockStmt, &blockStmts);
            }

            target->push_back(Block::Make(block.fLine, std::move(blockStmts),
                                          block.symbolTable(), block.isScope()));
            break;
        }

        case Statement::Kind::kBreak:
            // Do not append a break to the target.
            break;

        default:
            // Append normal statements to the target.
            target->push_back(std::move(stmt));
            break;
    }
}

std::unique_ptr<Statement> SwitchStatement::BlockForCase(StatementArray* cases,
                                                         SwitchCase* caseToCapture,
                                                         std::shared_ptr<SymbolTable> symbolTable) {
    // We have to be careful to not move any of the pointers until after we're sure we're going to
    // succeed, so before we make any changes at all, we check the switch-cases to decide on a plan
    // of action. First, find the switch-case we are interested in.
    auto iter = cases->begin();
    for (; iter != cases->end(); ++iter) {
        const SwitchCase& sc = (*iter)->as<SwitchCase>();
        if (&sc == caseToCapture) {
            break;
        }
    }

    // Next, walk forward through the rest of the switch. If we find a conditional break, we're
    // stuck and can't simplify at all. If we find an unconditional break, we have a range of
    // statements that we can use for simplification.
    auto startIter = iter;
    Statement* stripBreakStmt = nullptr;
    for (; iter != cases->end(); ++iter) {
        std::unique_ptr<Statement>& stmt = (*iter)->as<SwitchCase>().statement();
        if (Analysis::SwitchCaseContainsConditionalExit(*stmt)) {
            // We can't reduce switch-cases to a block when they have conditional exits.
            return nullptr;
        }
        if (Analysis::SwitchCaseContainsUnconditionalExit(*stmt)) {
            // We found an unconditional exit. We can use this block, but we'll need to strip
            // out the break statement if there is one.
            stripBreakStmt = stmt.get();
            break;
        }
    }

    // We fell off the bottom of the switch or encountered a break. We know the range of statements
    // that we need to move over, and we know it's safe to do so.
    StatementArray caseStmts;
    caseStmts.reserve_back(std::distance(startIter, iter) + 1);

    // We can move over most of the statements as-is.
    while (startIter != iter) {
        caseStmts.push_back(std::move((*startIter)->as<SwitchCase>().statement()));
        ++startIter;
    }

    // If we found an unconditional break at the end, we need to move what we can while avoiding
    // that break.
    if (stripBreakStmt != nullptr) {
        SkASSERT((*startIter)->as<SwitchCase>().statement().get() == stripBreakStmt);
        move_all_but_break((*startIter)->as<SwitchCase>().statement(), &caseStmts);
    }

    // Return our newly-synthesized block.
    return Block::Make(caseToCapture->fLine, std::move(caseStmts), std::move(symbolTable));
}

std::unique_ptr<Statement> SwitchStatement::Convert(const Context& context,
                                                    int line,
                                                    bool isStatic,
                                                    std::unique_ptr<Expression> value,
                                                    ExpressionArray caseValues,
                                                    StatementArray caseStatements,
                                                    std::shared_ptr<SymbolTable> symbolTable) {
    SkASSERT(caseValues.size() == caseStatements.size());

    value = context.fTypes.fInt->coerceExpression(std::move(value), context);
    if (!value) {
        return nullptr;
    }

    StatementArray cases;
    for (int i = 0; i < caseValues.count(); ++i) {
        int caseLine;
        std::unique_ptr<Expression> caseValue;
        if (caseValues[i]) {
            caseLine = caseValues[i]->fLine;

            // Case values must be the same type as the switch value--`int` or a particular enum.
            caseValue = value->type().coerceExpression(std::move(caseValues[i]), context);
            if (!caseValue) {
                return nullptr;
            }
            // Case values must be a literal integer or a `const int` variable reference.
            SKSL_INT intValue;
            if (!ConstantFolder::GetConstantInt(*caseValue, &intValue)) {
                context.fErrors->error(caseValue->fLine, "case value must be a constant integer");
                return nullptr;
            }
        } else {
            // The null case-expression corresponds to `default:`.
            caseLine = line;
        }
        cases.push_back(std::make_unique<SwitchCase>(caseLine, std::move(caseValue),
                                                     std::move(caseStatements[i])));
    }

    // Detect duplicate `case` labels and report an error.
    // (Using forward_list here to optimize for the common case of no results.)
    std::forward_list<const SwitchCase*> duplicateCases = find_duplicate_case_values(cases);
    if (!duplicateCases.empty()) {
        duplicateCases.reverse();
        for (const SwitchCase* sc : duplicateCases) {
            if (sc->value() != nullptr) {
                context.fErrors->error(sc->fLine,
                                       "duplicate case value '" + sc->value()->description() + "'");
            } else {
                context.fErrors->error(sc->fLine, "duplicate default case");
            }
        }
        return nullptr;
    }

    return SwitchStatement::Make(context, line, isStatic, std::move(value), std::move(cases),
                                 std::move(symbolTable));
}

std::unique_ptr<Statement> SwitchStatement::Make(const Context& context,
                                                 int line,
                                                 bool isStatic,
                                                 std::unique_ptr<Expression> value,
                                                 StatementArray cases,
                                                 std::shared_ptr<SymbolTable> symbolTable) {
    // Confirm that every statement in `cases` is a SwitchCase.
    SkASSERT(std::all_of(cases.begin(), cases.end(), [&](const std::unique_ptr<Statement>& stmt) {
        return stmt->is<SwitchCase>();
    }));

    // Confirm that every switch-case has been coerced to the proper type.
    SkASSERT(std::all_of(cases.begin(), cases.end(), [&](const std::unique_ptr<Statement>& stmt) {
        return !stmt->as<SwitchCase>().value() ||  // `default` case has a null value
               value->type().matches(stmt->as<SwitchCase>().value()->type());
    }));

    // Confirm that every switch-case value is unique.
    SkASSERT(find_duplicate_case_values(cases).empty());

    // Flatten @switch statements.
    if (isStatic || context.fConfig->fSettings.fOptimize) {
        SKSL_INT switchValue;
        if (ConstantFolder::GetConstantInt(*value, &switchValue)) {
            SwitchCase* defaultCase = nullptr;
            SwitchCase* matchingCase = nullptr;
            for (const std::unique_ptr<Statement>& stmt : cases) {
                SwitchCase& sc = stmt->as<SwitchCase>();
                if (!sc.value()) {
                    defaultCase = &sc;
                    continue;
                }

                SKSL_INT caseValue;
                SkAssertResult(ConstantFolder::GetConstantInt(*sc.value(), &caseValue));
                if (caseValue == switchValue) {
                    matchingCase = &sc;
                    break;
                }
            }

            if (!matchingCase) {
                // No case value matches the switch value.
                if (!defaultCase) {
                    // No default switch-case exists; the switch had no effect.
                    // We can eliminate the entire switch!
                    return Nop::Make();
                }
                // We had a default case; that's what we matched with.
                matchingCase = defaultCase;
            }

            // Convert the switch-case that we matched with into a block.
            std::unique_ptr<Statement> newBlock = BlockForCase(&cases, matchingCase, symbolTable);
            if (newBlock) {
                return newBlock;
            }

            // Report an error if this was a static switch and BlockForCase failed us.
            if (isStatic && !context.fConfig->fSettings.fPermitInvalidStaticTests) {
                context.fErrors->error(value->fLine,
                                       "static switch contains non-static conditional exit");
                return nullptr;
            }
        }
    }

    // The switch couldn't be optimized away; emit it normally.
    return std::make_unique<SwitchStatement>(line, isStatic, std::move(value), std::move(cases),
                                             std::move(symbolTable));
}

}  // namespace SkSL
