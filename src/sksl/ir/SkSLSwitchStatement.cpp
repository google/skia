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
    std::vector<std::unique_ptr<SwitchCase>> cloned;
    for (const std::unique_ptr<SwitchCase>& sc : this->cases()) {
        cloned.emplace_back(&sc->clone().release()->as<SwitchCase>());
    }
    return std::make_unique<SwitchStatement>(fOffset,
                                             this->isStatic(),
                                             this->value()->clone(),
                                             std::move(cloned),
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
        const std::vector<std::unique_ptr<SwitchCase>>& cases) {
    std::forward_list<const SwitchCase*> duplicateCases;
    SkTHashSet<SKSL_INT> intValues;
    bool foundDefault = false;

    for (const std::unique_ptr<SwitchCase>& sc : cases) {
        const std::unique_ptr<Expression>& valueExpr = sc->value();

        // A null case-value indicates the `default` switch-case.
        if (!valueExpr) {
            if (foundDefault) {
                duplicateCases.push_front(sc.get());
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
            duplicateCases.push_front(sc.get());
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
            for (std::unique_ptr<Statement>& stmt : block.children()) {
                move_all_but_break(stmt, &blockStmts);
            }

            target->push_back(std::make_unique<Block>(block.fOffset, std::move(blockStmts),
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

std::unique_ptr<Statement> SwitchStatement::BlockForCase(
        std::vector<std::unique_ptr<SwitchCase>>* cases,
        SwitchCase* caseToCapture,
        std::shared_ptr<SymbolTable> symbolTable) {
    // We have to be careful to not move any of the pointers until after we're sure we're going to
    // succeed, so before we make any changes at all, we check the switch-cases to decide on a plan
    // of action. First, find the switch-case we are interested in.
    auto iter = cases->begin();
    for (; iter != cases->end(); ++iter) {
        if (iter->get() == caseToCapture) {
            break;
        }
    }

    // Next, walk forward through the rest of the switch. If we find a conditional break, we're
    // stuck and can't simplify at all. If we find an unconditional break, we have a range of
    // statements that we can use for simplification.
    auto startIter = iter;
    Statement* stripBreakStmt = nullptr;
    for (; iter != cases->end(); ++iter) {
        for (std::unique_ptr<Statement>& stmt : (*iter)->statements()) {
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

        if (stripBreakStmt) {
            break;
        }
    }

    // We fell off the bottom of the switch or encountered a break. We know the range of statements
    // that we need to move over, and we know it's safe to do so.
    StatementArray caseStmts;
    caseStmts.reserve_back(std::distance(startIter, iter));

    // We can move over most of the statements as-is.
    while (startIter != iter) {
        for (std::unique_ptr<Statement>& stmt : (*startIter)->statements()) {
            caseStmts.push_back(std::move(stmt));
        }
        ++startIter;
    }

    // If we found an unconditional break at the end, we need to move what we can while avoiding
    // that break.
    if (stripBreakStmt != nullptr) {
        for (std::unique_ptr<Statement>& stmt : (*startIter)->statements()) {
            if (stmt.get() == stripBreakStmt) {
                move_all_but_break(stmt, &caseStmts);
                stripBreakStmt = nullptr;
                break;
            }

            caseStmts.push_back(std::move(stmt));
        }
    }

    SkASSERT(stripBreakStmt == nullptr);  // Verify that we stripped any unconditional break.

    // Return our newly-synthesized block.
    return std::make_unique<Block>(caseToCapture->fOffset, std::move(caseStmts),
                                   std::move(symbolTable));
}

std::unique_ptr<Statement> SwitchStatement::Convert(const Context& context,
                                                    int offset,
                                                    bool isStatic,
                                                    std::unique_ptr<Expression> value,
                                                    ExpressionArray caseValues,
                                                    SkTArray<StatementArray> caseStatements,
                                                    std::shared_ptr<SymbolTable> symbolTable) {
    SkASSERT(caseValues.size() == caseStatements.size());
    if (context.fConfig->strictES2Mode()) {
        context.fErrors.error(offset, "switch statements are not supported");
        return nullptr;
    }

    if (!value->type().isEnum()) {
        value = context.fTypes.fInt->coerceExpression(std::move(value), context);
        if (!value) {
            return nullptr;
        }
    }

    std::vector<std::unique_ptr<SwitchCase>> cases;
    for (int i = 0; i < caseValues.count(); ++i) {
        int caseOffset;
        std::unique_ptr<Expression> caseValue;
        if (caseValues[i]) {
            caseOffset = caseValues[i]->fOffset;

            // Case values must be the same type as the switch value--`int` or a particular enum.
            caseValue = value->type().coerceExpression(std::move(caseValues[i]), context);
            if (!caseValue) {
                return nullptr;
            }
            // Case values must be a literal integer or a `const int` variable reference.
            SKSL_INT intValue;
            if (!ConstantFolder::GetConstantInt(*caseValue, &intValue)) {
                context.fErrors.error(caseValue->fOffset, "case value must be a constant integer");
                return nullptr;
            }
        } else {
            // The null case-expression corresponds to `default:`.
            caseOffset = offset;
        }
        cases.push_back(std::make_unique<SwitchCase>(caseOffset, std::move(caseValue),
                                                     std::move(caseStatements[i])));
    }

    // Detect duplicate `case` labels and report an error.
    // (Using forward_list here to optimize for the common case of no results.)
    std::forward_list<const SwitchCase*> duplicateCases = find_duplicate_case_values(cases);
    if (!duplicateCases.empty()) {
        duplicateCases.reverse();
        for (const SwitchCase* sc : duplicateCases) {
            if (sc->value() != nullptr) {
                context.fErrors.error(sc->fOffset,
                                      "duplicate case value '" + sc->value()->description() + "'");
            } else {
                context.fErrors.error(sc->fOffset, "duplicate default case");
            }
        }
        return nullptr;
    }

    return SwitchStatement::Make(context, offset, isStatic, std::move(value), std::move(cases),
                                 std::move(symbolTable));
}

std::unique_ptr<Statement> SwitchStatement::Make(const Context& context,
                                                 int offset,
                                                 bool isStatic,
                                                 std::unique_ptr<Expression> value,
                                                 std::vector<std::unique_ptr<SwitchCase>> cases,
                                                 std::shared_ptr<SymbolTable> symbolTable) {
    // Confirm that every switch-case has been coerced to the proper type.
    SkASSERT(std::all_of(cases.begin(), cases.end(), [&](const std::unique_ptr<SwitchCase>& sc) {
        return !sc->value() ||  // `default` case has a null value
               value->type() == sc->value()->type();
    }));

    // Confirm that every switch-case value is unique.
    SkASSERT(find_duplicate_case_values(cases).empty());

    // Flatten @switch statements.
    if (isStatic) {
        SKSL_INT switchValue;
        if (ConstantFolder::GetConstantInt(*value, &switchValue)) {
            SwitchCase* defaultCase = nullptr;
            SwitchCase* matchingCase = nullptr;
            for (const std::unique_ptr<SwitchCase>& sc : cases) {
                if (!sc->value()) {
                    defaultCase = sc.get();
                    continue;
                }

                SKSL_INT caseValue;
                SkAssertResult(ConstantFolder::GetConstantInt(*sc->value(), &caseValue));
                if (caseValue == switchValue) {
                    matchingCase = sc.get();
                    break;
                }
            }

            if (!matchingCase) {
                // No case value matches the switch value.
                if (!defaultCase) {
                    // No default switch-case exists; the switch had no effect.
                    // We can eliminate the entire switch!
                    return std::make_unique<Nop>();
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
            if (!context.fConfig->fSettings.fPermitInvalidStaticTests) {
                context.fErrors.error(value->fOffset,
                                      "static switch contains non-static conditional exit");
                return nullptr;
            }
        }
    }

    // The switch couldn't be optimized away; emit it normally.
    return std::make_unique<SwitchStatement>(offset, isStatic, std::move(value), std::move(cases),
                                             std::move(symbolTable));
}

}  // namespace SkSL
