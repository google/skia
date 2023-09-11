/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkDebug.h"
#include "src/base/SkEnumBitMask.h"
#include "src/core/SkTHash.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/analysis/SkSLProgramUsage.h"
#include "src/sksl/analysis/SkSLProgramVisitor.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLModifierFlags.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"

#include <cstring>
#include <memory>
#include <string_view>
#include <vector>

namespace SkSL {

struct Program;

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
            fUsage->fVariableCounts[pe.as<InterfaceBlock>().var()];
        }
        return INHERITED::visitProgramElement(pe);
    }

    bool visitStatement(const Statement& s) override {
        if (s.is<VarDeclaration>()) {
            // Add all declared variables to the usage map (even if never otherwise accessed).
            const VarDeclaration& vd = s.as<VarDeclaration>();
            ProgramUsage::VariableCounts& counts = fUsage->fVariableCounts[vd.var()];
            counts.fVarExists += fDelta;
            SkASSERT(counts.fVarExists >= 0 && counts.fVarExists <= 1);
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

std::unique_ptr<ProgramUsage> Analysis::GetUsage(const Module& module) {
    auto usage = std::make_unique<ProgramUsage>();
    ProgramUsageVisitor addRefs(usage.get(), /*delta=*/+1);

    for (const Module* m = &module; m != nullptr; m = m->fParent) {
        for (const std::unique_ptr<ProgramElement>& element : m->fElements) {
            addRefs.visitProgramElement(*element);
        }
    }
    return usage;
}

ProgramUsage::VariableCounts ProgramUsage::get(const Variable& v) const {
    const VariableCounts* counts = fVariableCounts.find(&v);
    SkASSERT(counts);
    return *counts;
}

bool ProgramUsage::isDead(const Variable& v) const {
    ModifierFlags flags = v.modifierFlags();
    VariableCounts counts = this->get(v);
    if (flags & (ModifierFlag::kIn | ModifierFlag::kOut | ModifierFlag::kUniform)) {
        // Never eliminate ins, outs, or uniforms.
        return false;
    }
    if (v.type().componentType().isOpaque()) {
        // Never eliminate samplers, runtime-effect children, or atomics.
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

static bool contains_matching_data(const ProgramUsage& a, const ProgramUsage& b) {
    constexpr bool kReportMismatch = false;

    for (const auto& [varA, varCountA] : a.fVariableCounts) {
        // Skip variable entries with zero reported usage.
        if (!varCountA.fVarExists && !varCountA.fRead && !varCountA.fWrite) {
            continue;
        }
        // Find the matching variable in the other map and ensure that its counts match.
        const ProgramUsage::VariableCounts* varCountB = b.fVariableCounts.find(varA);
        if (!varCountB || 0 != memcmp(&varCountA, varCountB, sizeof(varCountA))) {
            if constexpr (kReportMismatch) {
                SkDebugf("VariableCounts mismatch: '%.*s' (E%d R%d W%d != E%d R%d W%d)\n",
                         (int)varA->name().size(), varA->name().data(),
                         varCountA.fVarExists,
                         varCountA.fRead,
                         varCountA.fWrite,
                         varCountB ? varCountB->fVarExists : 0,
                         varCountB ? varCountB->fRead : 0,
                         varCountB ? varCountB->fWrite : 0);
            }
            return false;
        }
    }

    for (const auto& [callA, callCountA] : a.fCallCounts) {
        // Skip function-call entries with zero reported usage.
        if (!callCountA) {
            continue;
        }
        // Find the matching function in the other map and ensure that its call-count matches.
        const int* callCountB = b.fCallCounts.find(callA);
        if (!callCountB || callCountA != *callCountB) {
            if constexpr (kReportMismatch) {
                SkDebugf("CallCounts mismatch: '%.*s' (%d != %d)\n",
                         (int)callA->name().size(), callA->name().data(),
                         callCountA,
                         callCountB ? *callCountB : 0);
            }
            return false;
        }
    }

    // Every non-zero entry in A has a matching non-zero entry in B.
    return true;
}

bool ProgramUsage::operator==(const ProgramUsage& that) const {
    // ProgramUsage can be "equal" while the underlying hash maps look slightly different, because a
    // dead-stripped variable or function will have a usage count of zero, but will still exist in
    // the maps. If the program usage is re-analyzed from scratch, the maps will not contain an
    // entry for these variables or functions at all. This means our maps can be "equal" while
    // having different element counts.
    //
    // In order to check these maps, we compare map entries bi-directionally, skipping zero-usage
    // entries. If all the non-zero elements in `this` match the elements in `that`, and all the
    // non-zero elements in `that` match the elements in `this`, all the non-zero elements must be
    // identical, and all the zero elements must be either zero or non-existent on both sides.
    return contains_matching_data(*this, that) &&
           contains_matching_data(that, *this);
}

}  // namespace SkSL
