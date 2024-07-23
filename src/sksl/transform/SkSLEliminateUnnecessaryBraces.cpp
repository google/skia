/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLDefines.h"
#include "src/sksl/SkSLModule.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/transform/SkSLProgramWriter.h"
#include "src/sksl/transform/SkSLTransform.h"

#include <memory>
#include <utility>
#include <vector>

namespace SkSL {

class Context;

void Transform::EliminateUnnecessaryBraces(const Context& context, Module& module) {
    class UnnecessaryBraceEliminator : public ProgramWriter {
    public:
        bool visitExpressionPtr(std::unique_ptr<Expression>& expr) override {
            // We don't need to look inside expressions at all.
            return false;
        }

        bool visitStatementPtr(std::unique_ptr<Statement>& stmt) override {
            // Work from the innermost blocks to the outermost.
            INHERITED::visitStatementPtr(stmt);

            switch (stmt->kind()) {
                case StatementKind::kIf: {
                    IfStatement& ifStmt = stmt->as<IfStatement>();
                    EliminateBracesFrom(ifStmt.ifTrue());
                    EliminateBracesFrom(ifStmt.ifFalse());
                    break;
                }
                case StatementKind::kFor: {
                    ForStatement& forStmt = stmt->as<ForStatement>();
                    EliminateBracesFrom(forStmt.statement());
                    break;
                }
                case StatementKind::kDo: {
                    DoStatement& doStmt = stmt->as<DoStatement>();
                    EliminateBracesFrom(doStmt.statement());
                    break;
                }
                default:
                    break;
            }

            // We always check the entire program.
            return false;
        }

        static void EliminateBracesFrom(std::unique_ptr<Statement>& stmt) {
            if (!stmt || !stmt->is<Block>()) {
                return;
            }
            Block& block = stmt->as<Block>();
            std::unique_ptr<Statement>* usefulStmt = nullptr;
            for (std::unique_ptr<Statement>& childStmt : block.children()) {
                if (childStmt->isEmpty()) {
                    continue;
                }
                if (usefulStmt) {
                    // We found two non-empty statements. We can't eliminate braces from
                    // this block.
                    return;
                }
                // We found one non-empty statement.
                usefulStmt = &childStmt;
            }

            if (!usefulStmt) {
                // This block held zero useful statements. Replace the block with a nop.
                stmt = Nop::Make();
            } else {
                // This block held one useful statement. Replace the block with that statement.
                stmt = std::move(*usefulStmt);
            }
        }

        using INHERITED = ProgramWriter;
    };

    class RequiredBraceWriter : public ProgramWriter {
    public:
        RequiredBraceWriter(const Context& ctx) : fContext(ctx) {}

        bool visitExpressionPtr(std::unique_ptr<Expression>& expr) override {
            // We don't need to look inside expressions at all.
            return false;
        }

        bool visitStatementPtr(std::unique_ptr<Statement>& stmt) override {
            // Look for the following structure:
            //
            //    if (...)
            //      if (...)
            //        any statement;
            //    else
            //      any statement;
            //
            // This structure isn't correct if we emit it textually, because the else-clause would
            // be interpreted as if it were bound to the inner if-statement, like this:
            //
            //    if (...) {
            //      if (...)
            //        any statement;
            //      else
            //        any statement;
            //    }
            //
            // If we find such a structure, we must disambiguate the else-clause by adding braces:
            //    if (...) {
            //      if (...)
            //        any statement;
            //    } else
            //      any statement;

            // Work from the innermost blocks to the outermost.
            INHERITED::visitStatementPtr(stmt);

            // We are looking for an if-statement.
            if (stmt->is<IfStatement>()) {
                IfStatement& outer = stmt->as<IfStatement>();

                // It should have an else clause, and directly wrap another if-statement (no Block).
                if (outer.ifFalse() && outer.ifTrue()->is<IfStatement>()) {
                    const IfStatement& inner = outer.ifTrue()->as<IfStatement>();

                    // The inner if statement shouldn't have an else clause.
                    if (!inner.ifFalse()) {
                        // This structure is ambiguous; the else clause on the outer if-statement
                        // will bind to the inner if-statement if we don't add braces. We must wrap
                        // the outer if-statement's true-clause in braces.
                        StatementArray blockStmts;
                        blockStmts.push_back(std::move(outer.ifTrue()));
                        Position stmtPosition = blockStmts.front()->position();
                        std::unique_ptr<Statement> bracedIfTrue =
                                Block::MakeBlock(stmtPosition, std::move(blockStmts));
                        stmt = IfStatement::Make(fContext,
                                                 outer.position(),
                                                 std::move(outer.test()),
                                                 std::move(bracedIfTrue),
                                                 std::move(outer.ifFalse()));
                    }
                }
            }

            // We always check the entire program.
            return false;
        }

        const Context& fContext;
        using INHERITED = ProgramWriter;
    };

    for (std::unique_ptr<ProgramElement>& pe : module.fElements) {
        if (pe->is<FunctionDefinition>()) {
            // First, we eliminate braces around single-statement child blocks wherever possible.
            UnnecessaryBraceEliminator eliminator;
            eliminator.visitStatementPtr(pe->as<FunctionDefinition>().body());

            // The first pass can be overzealous, since it can remove so many braces that else-
            // clauses are bound to the wrong if-statement. Search for this case and fix it up
            // if we find it.
            RequiredBraceWriter writer(context);
            writer.visitStatementPtr(pe->as<FunctionDefinition>().body());
        }
    }
}

}  // namespace SkSL
