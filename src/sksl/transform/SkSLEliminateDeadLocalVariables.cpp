/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "src/core/SkTHash.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLModule.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/analysis/SkSLProgramUsage.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"
#include "src/sksl/transform/SkSLProgramWriter.h"
#include "src/sksl/transform/SkSLTransform.h"

#include <memory>
#include <utility>
#include <vector>

using namespace skia_private;

namespace SkSL {

class Context;

static bool eliminate_dead_local_variables(const Context& context,
                                           SkSpan<std::unique_ptr<ProgramElement>> elements,
                                           ProgramUsage* usage) {
    class DeadLocalVariableEliminator : public ProgramWriter {
    public:
        DeadLocalVariableEliminator(const Context& context, ProgramUsage* usage)
                : fContext(context)
                , fUsage(usage) {}

        using ProgramWriter::visitProgramElement;

        bool visitExpressionPtr(std::unique_ptr<Expression>& expr) override {
            if (expr->is<BinaryExpression>()) {
                // Search for expressions of the form `deadVar = anyExpression`.
                BinaryExpression& binary = expr->as<BinaryExpression>();
                if (VariableReference* assignedVar = binary.isAssignmentIntoVariable()) {
                    if (fDeadVariables.contains(assignedVar->variable())) {
                        // Replace `deadVar = anyExpression` with `anyExpression`.
                        fUsage->remove(expr.get());
                        expr = std::move(binary.right());
                        fUsage->add(expr.get());

                        // If `anyExpression` is now a lone ExpressionStatement, it's highly likely
                        // that we can eliminate it entirely. This flag will let us know to check.
                        fAssignmentWasEliminated = true;

                        // Re-process the newly cleaned-up expression. This lets us fully clean up
                        // gnarly assignments like `a = b = 123;` where both `a` and `b` are dead,
                        // or silly double-assignments like `a = a = 123;`.
                        return this->visitExpressionPtr(expr);
                    }
                }
            }
            if (expr->is<VariableReference>()) {
                SkASSERT(!fDeadVariables.contains(expr->as<VariableReference>().variable()));
            }
            return INHERITED::visitExpressionPtr(expr);
        }

        bool visitStatementPtr(std::unique_ptr<Statement>& stmt) override {
            if (stmt->is<VarDeclaration>()) {
                VarDeclaration& varDecl = stmt->as<VarDeclaration>();
                const Variable* var = varDecl.var();
                ProgramUsage::VariableCounts* counts = fUsage->fVariableCounts.find(var);
                SkASSERT(counts);
                SkASSERT(counts->fVarExists);
                if (CanEliminate(var, *counts)) {
                    fDeadVariables.add(var);
                    if (var->initialValue()) {
                        // The variable has an initial-value expression, which might have side
                        // effects. ExpressionStatement::Make will preserve side effects, but
                        // replaces pure expressions with Nop.
                        fUsage->remove(stmt.get());
                        stmt = ExpressionStatement::Make(fContext, std::move(varDecl.value()));
                        fUsage->add(stmt.get());
                    } else {
                        // The variable has no initial-value and can be cleanly eliminated.
                        fUsage->remove(stmt.get());
                        stmt = Nop::Make();
                    }
                    fMadeChanges = true;

                    // Re-process the newly cleaned-up statement. This lets us fully clean up
                    // gnarly assignments like `a = b = 123;` where both `a` and `b` are dead,
                    // or silly double-assignments like `a = a = 123;`.
                    return this->visitStatementPtr(stmt);
                }
            }

            bool result = INHERITED::visitStatementPtr(stmt);

            // If we eliminated an assignment above, we may have left behind an inert
            // ExpressionStatement.
            if (fAssignmentWasEliminated) {
                fAssignmentWasEliminated = false;
                if (stmt->is<ExpressionStatement>()) {
                    ExpressionStatement& exprStmt = stmt->as<ExpressionStatement>();
                    if (!Analysis::HasSideEffects(*exprStmt.expression())) {
                        // The expression-statement was inert; eliminate it entirely.
                        fUsage->remove(&exprStmt);
                        stmt = Nop::Make();
                    }
                }
            }

            return result;
        }

        static bool CanEliminate(const Variable* var, const ProgramUsage::VariableCounts& counts) {
            return counts.fVarExists && !counts.fRead && var->storage() == VariableStorage::kLocal;
        }

        bool fMadeChanges = false;
        const Context& fContext;
        ProgramUsage* fUsage;
        THashSet<const Variable*> fDeadVariables;
        bool fAssignmentWasEliminated = false;

        using INHERITED = ProgramWriter;
    };

    DeadLocalVariableEliminator visitor{context, usage};

    for (auto& [var, counts] : usage->fVariableCounts) {
        if (DeadLocalVariableEliminator::CanEliminate(var, counts)) {
            // This program contains at least one dead local variable.
            // Scan the program for any dead local variables and eliminate them all.
            for (std::unique_ptr<ProgramElement>& pe : elements) {
                if (pe->is<FunctionDefinition>()) {
                    visitor.visitProgramElement(*pe);
                }
            }
            break;
        }
    }

    return visitor.fMadeChanges;
}

bool Transform::EliminateDeadLocalVariables(const Context& context,
                                            Module& module,
                                            ProgramUsage* usage) {
    return eliminate_dead_local_variables(context, SkSpan(module.fElements), usage);
}

bool Transform::EliminateDeadLocalVariables(Program& program) {
    return program.fConfig->fSettings.fRemoveDeadVariables
                   ? eliminate_dead_local_variables(*program.fContext,
                                                    SkSpan(program.fOwnedElements),
                                                    program.fUsage.get())
                   : false;
}

}  // namespace SkSL
