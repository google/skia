/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSpan.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLDefines.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLDoStatement.h"
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

class Expression;

static void eliminate_unnecessary_braces(SkSpan<std::unique_ptr<ProgramElement>> elements) {
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

    for (std::unique_ptr<ProgramElement>& pe : elements) {
        if (pe->is<FunctionDefinition>()) {
            UnnecessaryBraceEliminator visitor;
            visitor.visitStatementPtr(pe->as<FunctionDefinition>().body());
        }
    }
}

void Transform::EliminateUnnecessaryBraces(Module& module) {
    return eliminate_unnecessary_braces(SkSpan(module.fElements));
}

}  // namespace SkSL
