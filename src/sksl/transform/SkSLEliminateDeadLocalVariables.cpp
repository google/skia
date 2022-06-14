/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/SkSLProgramElement.h"
#include "include/private/SkSLStatement.h"
#include "include/private/SkTHash.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/transform/SkSLProgramWriter.h"
#include "src/sksl/transform/SkSLTransform.h"

#include <memory>
#include <utility>

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
            // We don't need to look inside expressions at all.
            return false;
        }

        bool visitStatementPtr(std::unique_ptr<Statement>& stmt) override {
            if (stmt->is<VarDeclaration>()) {
                VarDeclaration& varDecl = stmt->as<VarDeclaration>();
                const Variable* var = &varDecl.var();
                ProgramUsage::VariableCounts* counts = fUsage->fVariableCounts.find(var);
                SkASSERT(counts);
                SkASSERT(counts->fDeclared);
                if (CanEliminate(var, *counts)) {
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
                }
                return false;
            }
            return INHERITED::visitStatementPtr(stmt);
        }

        static bool CanEliminate(const Variable* var, const ProgramUsage::VariableCounts& counts) {
            if (!counts.fDeclared || counts.fRead || var->storage() != VariableStorage::kLocal) {
                return false;
            }
            if (var->initialValue()) {
                SkASSERT(counts.fWrite >= 1);
                return counts.fWrite == 1;
            } else {
                return counts.fWrite == 0;
            }
        }

        bool fMadeChanges = false;
        const Context& fContext;
        ProgramUsage* fUsage;

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
                                            LoadedModule& module,
                                            ProgramUsage* usage) {
    return eliminate_dead_local_variables(context, SkSpan(module.fElements), usage);
}

bool Transform::EliminateDeadLocalVariables(Program& program, ProgramUsage* usage) {
    return program.fConfig->fSettings.fRemoveDeadVariables
                   ? eliminate_dead_local_variables(*program.fContext,
                                                    SkSpan(program.fOwnedElements),
                                                    usage)
                   : false;
}

}  // namespace SkSL
