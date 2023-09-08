/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTArray.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLDefines.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLIRHelpers.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLModifierFlags.h"
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/transform/SkSLProgramWriter.h"
#include "src/sksl/transform/SkSLTransform.h"

#include <memory>
#include <utility>

using namespace skia_private;

namespace SkSL {

class Context;

std::unique_ptr<Statement> Transform::HoistSwitchVarDeclarationsAtTopLevel(
        const Context& context,
        std::unique_ptr<SwitchStatement> stmt) {
    struct HoistSwitchVarDeclsVisitor : public ProgramWriter {
        HoistSwitchVarDeclsVisitor(const Context& c) : fContext(c) {}

        bool visitExpressionPtr(std::unique_ptr<Expression>& expr) override {
            // We don't need to recurse into expressions.
            return false;
        }

        bool visitStatementPtr(std::unique_ptr<Statement>& stmt) override {
            switch (stmt->kind()) {
                case StatementKind::kSwitchCase:
                    // Recurse inward from the switch and its inner switch-cases.
                    return INHERITED::visitStatementPtr(stmt);

                case StatementKind::kBlock:
                    if (!stmt->as<Block>().isScope()) {
                        // Recurse inward from unscoped blocks.
                        return INHERITED::visitStatementPtr(stmt);
                    }
                    break;

                case StatementKind::kVarDeclaration:
                    // Keep track of variable declarations.
                    fVarDeclarations.push_back(&stmt);
                    break;

                default:
                    break;
            }

            // We don't need to recurse into other statement types; we're only interested in the top
            // level of the switch statement.
            return false;
        }

        const Context& fContext;
        TArray<std::unique_ptr<Statement>*> fVarDeclarations;

        using INHERITED = ProgramWriter;
    };

    // Visit every switch-case in the switch, looking for hoistable var-declarations.
    HoistSwitchVarDeclsVisitor visitor(context);
    for (std::unique_ptr<Statement>& sc : stmt->as<SwitchStatement>().cases()) {
        visitor.visitStatementPtr(sc);
    }

    // If no declarations were found, return the switch as-is.
    if (visitor.fVarDeclarations.empty()) {
        return stmt;
    }

    // Move all of the var-declaration statements into a separate block.
    StatementArray blockStmts;
    blockStmts.reserve_exact(visitor.fVarDeclarations.size() + 1);
    for (std::unique_ptr<Statement>* innerDeclaration : visitor.fVarDeclarations) {
        VarDeclaration& decl = (*innerDeclaration)->as<VarDeclaration>();
        std::unique_ptr<Statement> replacementStmt;
        bool isConst = decl.var()->modifierFlags().isConst();
        if (decl.value() && !isConst) {
            // The inner variable-declaration has an initial-value; we must replace the declaration
            // with an assignment to the variable. This also has the helpful effect of stripping off
            // the initial-value from the declaration.
            struct AssignmentHelper : public IRHelpers {
                using IRHelpers::IRHelpers;

                std::unique_ptr<Statement> makeAssignmentStmt(VarDeclaration& decl) const {
                    return Assign(Ref(decl.var()), std::move(decl.value()));
                }
            };

            AssignmentHelper helper(context);
            replacementStmt = helper.makeAssignmentStmt(decl);
        } else {
            // The inner variable-declaration has no initial-value, or it's const and has a constant
            // value; we can move it upwards as-is and replace its statement with a no-op.
            SkASSERT(!isConst || Analysis::IsConstantExpression(*decl.value()));

            replacementStmt = Nop::Make();
        }

        // Move the var-declaration above the switch, and replace the existing statement with either
        // an assignment (if there was an initial-value) or a no-op (if there wasn't one).
        blockStmts.push_back(std::move(*innerDeclaration));
        *innerDeclaration = std::move(replacementStmt);
    }

    // Return a scoped Block holding the switch.
    Position pos = stmt->fPosition;
    blockStmts.push_back(std::move(stmt));
    return Block::MakeBlock(pos, std::move(blockStmts));
}

}  // namespace SkSL
