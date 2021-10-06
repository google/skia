/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/analysis/SkSLProgramVisitor.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariableReference.h"

namespace SkSL {
namespace {

class ProgramUsageVisitor : public ProgramVisitor {
public:
    ProgramUsageVisitor(ProgramUsage* usage, int delta) : fUsage(usage), fDelta(delta) {}

    bool visitProgramElement(const ProgramElement& pe) override {
        if (pe.is<FunctionDefinition>()) {
            for (const Variable* param : pe.as<FunctionDefinition>().declaration().parameters()) {
                // Ensure function-parameter variables exist in the variable usage map. They aren't
                // otherwise declared, but ProgramUsage::get() should be able to find them, even if
                // they are unread and unwritten.
                fUsage->fVariableCounts[param];
            }
        } else if (pe.is<InterfaceBlock>()) {
            // Ensure interface-block variables exist in the variable usage map.
            fUsage->fVariableCounts[&pe.as<InterfaceBlock>().variable()];
        }
        return INHERITED::visitProgramElement(pe);
    }

    bool visitStatement(const Statement& s) override {
        if (s.is<VarDeclaration>()) {
            // Add all declared variables to the usage map (even if never otherwise accessed).
            const VarDeclaration& vd = s.as<VarDeclaration>();
            ProgramUsage::VariableCounts& counts = fUsage->fVariableCounts[&vd.var()];
            counts.fDeclared += fDelta;
            SkASSERT(counts.fDeclared >= 0);
            if (vd.value()) {
                // The initial-value expression, when present, counts as a write.
                counts.fWrite += fDelta;
            }
        }
        return INHERITED::visitStatement(s);
    }

    bool visitExpression(const Expression& e) override {
        if (e.is<FunctionCall>()) {
            const FunctionDeclaration* f = &e.as<FunctionCall>().function();
            fUsage->fCallCounts[f] += fDelta;
            SkASSERT(fUsage->fCallCounts[f] >= 0);
        } else if (e.is<VariableReference>()) {
            const VariableReference& ref = e.as<VariableReference>();
            ProgramUsage::VariableCounts& counts = fUsage->fVariableCounts[ref.variable()];
            switch (ref.refKind()) {
                case VariableRefKind::kRead:
                    counts.fRead += fDelta;
                    break;
                case VariableRefKind::kWrite:
                    counts.fWrite += fDelta;
                    break;
                case VariableRefKind::kReadWrite:
                case VariableRefKind::kPointer:
                    counts.fRead += fDelta;
                    counts.fWrite += fDelta;
                    break;
            }
            SkASSERT(counts.fRead >= 0 && counts.fWrite >= 0);
        }
        return INHERITED::visitExpression(e);
    }

    using ProgramVisitor::visitProgramElement;
    using ProgramVisitor::visitStatement;

    ProgramUsage* fUsage;
    int fDelta;
    using INHERITED = ProgramVisitor;
};

}  // namespace

std::unique_ptr<ProgramUsage> Analysis::GetUsage(const Program& program) {
    auto usage = std::make_unique<ProgramUsage>();
    ProgramUsageVisitor addRefs(usage.get(), /*delta=*/+1);
    addRefs.visit(program);
    return usage;
}

std::unique_ptr<ProgramUsage> Analysis::GetUsage(const LoadedModule& module) {
    auto usage = std::make_unique<ProgramUsage>();
    ProgramUsageVisitor addRefs(usage.get(), /*delta=*/+1);
    for (const auto& element : module.fElements) {
        addRefs.visitProgramElement(*element);
    }
    return usage;
}

ProgramUsage::VariableCounts ProgramUsage::get(const Variable& v) const {
    const VariableCounts* counts = fVariableCounts.find(&v);
    SkASSERT(counts);
    return *counts;
}

bool ProgramUsage::isDead(const Variable& v) const {
    const Modifiers& modifiers = v.modifiers();
    VariableCounts counts = this->get(v);
    if ((v.storage() != Variable::Storage::kLocal && counts.fRead) ||
        (modifiers.fFlags &
         (Modifiers::kIn_Flag | Modifiers::kOut_Flag | Modifiers::kUniform_Flag))) {
        return false;
    }
    // Consider the variable dead if it's never read and never written (besides the initial-value).
    return !counts.fRead && (counts.fWrite <= (v.initialValue() ? 1 : 0));
}

int ProgramUsage::get(const FunctionDeclaration& f) const {
    const int* count = fCallCounts.find(&f);
    return count ? *count : 0;
}

void ProgramUsage::add(const Expression* expr) {
    ProgramUsageVisitor addRefs(this, /*delta=*/+1);
    addRefs.visitExpression(*expr);
}

void ProgramUsage::add(const Statement* stmt) {
    ProgramUsageVisitor addRefs(this, /*delta=*/+1);
    addRefs.visitStatement(*stmt);
}

void ProgramUsage::add(const ProgramElement& element) {
    ProgramUsageVisitor addRefs(this, /*delta=*/+1);
    addRefs.visitProgramElement(element);
}

void ProgramUsage::remove(const Expression* expr) {
    ProgramUsageVisitor subRefs(this, /*delta=*/-1);
    subRefs.visitExpression(*expr);
}

void ProgramUsage::remove(const Statement* stmt) {
    ProgramUsageVisitor subRefs(this, /*delta=*/-1);
    subRefs.visitStatement(*stmt);
}

void ProgramUsage::remove(const ProgramElement& element) {
    ProgramUsageVisitor subRefs(this, /*delta=*/-1);
    subRefs.visitProgramElement(element);
}

}  // namespace SkSL
