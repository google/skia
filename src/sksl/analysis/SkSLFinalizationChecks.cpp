/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "src/base/SkEnumBitMask.h"
#include "src/base/SkSafeMath.h"
#include "src/core/SkTHash.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLDefines.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/analysis/SkSLProgramUsage.h"
#include "src/sksl/analysis/SkSLProgramVisitor.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLLayout.h"
#include "src/sksl/ir/SkSLModifierFlags.h"
#include "src/sksl/ir/SkSLModifiersDeclaration.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

using namespace skia_private;

namespace SkSL {
namespace {

class FinalizationVisitor : public ProgramVisitor {
public:
    FinalizationVisitor(const Context& c, const ProgramUsage& u) : fContext(c), fUsage(u) {}

    bool visitProgramElement(const ProgramElement& pe) override {
        switch (pe.kind()) {
            case ProgramElement::Kind::kGlobalVar:
                this->checkGlobalVariableSizeLimit(pe.as<GlobalVarDeclaration>());
                break;
            case ProgramElement::Kind::kInterfaceBlock:
                // TODO(skia:13664): Enforce duplicate checks universally. This is currently not
                // possible without changes to the binding index assignment logic in graphite.
                this->checkBindUniqueness(pe.as<InterfaceBlock>());
                break;
            case ProgramElement::Kind::kFunction:
                this->checkOutParamsAreAssigned(pe.as<FunctionDefinition>());
                break;
            case ProgramElement::Kind::kModifiers:
                this->checkWorkgroupLocalSize(pe.as<ModifiersDeclaration>());
                break;
            default:
                break;
        }
        return INHERITED::visitProgramElement(pe);
    }

    void checkGlobalVariableSizeLimit(const GlobalVarDeclaration& globalDecl) {
        if (!ProgramConfig::IsRuntimeEffect(fContext.fConfig->fKind)) {
            return;
        }
        const VarDeclaration& decl = globalDecl.varDeclaration();

        size_t prevSlotsUsed = fGlobalSlotsUsed;
        fGlobalSlotsUsed = SkSafeMath::Add(fGlobalSlotsUsed, decl.var()->type().slotCount());
        // To avoid overzealous error reporting, only trigger the error at the first place where the
        // global limit is exceeded.
        if (prevSlotsUsed < kVariableSlotLimit && fGlobalSlotsUsed >= kVariableSlotLimit) {
            fContext.fErrors->error(decl.fPosition,
                                    "global variable '" + std::string(decl.var()->name()) +
                                    "' exceeds the size limit");
        }
    }

    void checkBindUniqueness(const InterfaceBlock& block) {
        const Variable* var = block.var();
        int32_t set = var->layout().fSet;
        int32_t binding = var->layout().fBinding;
        if (binding != -1) {
            // TODO(skia:13664): This should map a `set` value of -1 to the default settings value
            // used by codegen backends to prevent duplicates that may arise from the effective
            // default set value.
            uint64_t key = ((uint64_t)set << 32) + binding;
            if (!fBindings.contains(key)) {
                fBindings.add(key);
            } else {
                if (set != -1) {
                    fContext.fErrors->error(block.fPosition,
                                            "layout(set=" + std::to_string(set) +
                                                    ", binding=" + std::to_string(binding) +
                                                    ") has already been defined");
                } else {
                    fContext.fErrors->error(block.fPosition,
                                            "layout(binding=" + std::to_string(binding) +
                                                    ") has already been defined");
                }
            }
        }
    }

    void checkOutParamsAreAssigned(const FunctionDefinition& funcDef) {
        const FunctionDeclaration& funcDecl = funcDef.declaration();

        // Searches for `out` parameters that are not written to. According to the GLSL spec,
        // the value of an out-param that's never assigned to is unspecified, so report it.
        for (const Variable* param : funcDecl.parameters()) {
            const ModifierFlags paramInout = param->modifierFlags() & (ModifierFlag::kIn |
                                                                       ModifierFlag::kOut);
            if (paramInout == ModifierFlag::kOut) {
                ProgramUsage::VariableCounts counts = fUsage.get(*param);
                if (counts.fWrite <= 0) {
                    fContext.fErrors->error(param->fPosition,
                                            "function '" + std::string(funcDecl.name()) +
                                            "' never assigns a value to out parameter '" +
                                            std::string(param->name()) + "'");
                }
            }
        }
    }

    void checkWorkgroupLocalSize(const ModifiersDeclaration& d) {
        if (d.layout().fLocalSizeX >= 0) {
            if (fLocalSizeX >= 0) {
                fContext.fErrors->error(d.fPosition, "'local_size_x' was specified more than once");
            } else {
                fLocalSizeX = d.layout().fLocalSizeX;
            }
        }
        if (d.layout().fLocalSizeY >= 0) {
            if (fLocalSizeY >= 0) {
                fContext.fErrors->error(d.fPosition, "'local_size_y' was specified more than once");
            } else {
                fLocalSizeY = d.layout().fLocalSizeY;
            }
        }
        if (d.layout().fLocalSizeZ >= 0) {
            if (fLocalSizeZ >= 0) {
                fContext.fErrors->error(d.fPosition, "'local_size_z' was specified more than once");
            } else {
                fLocalSizeZ = d.layout().fLocalSizeZ;
            }
        }
    }

    bool visitExpression(const Expression& expr) override {
        switch (expr.kind()) {
            case Expression::Kind::kFunctionCall: {
                const FunctionDeclaration& decl = expr.as<FunctionCall>().function();
                if (!decl.isBuiltin() && !decl.definition()) {
                    fContext.fErrors->error(expr.fPosition, "function '" + decl.description() +
                                                            "' is not defined");
                }
                break;
            }
            case Expression::Kind::kFunctionReference:
            case Expression::Kind::kMethodReference:
            case Expression::Kind::kTypeReference:
                SkDEBUGFAIL("invalid reference-expr, should have been reported by coerce()");
                fContext.fErrors->error(expr.fPosition, "invalid expression");
                break;
            default:
                if (expr.type().matches(*fContext.fTypes.fInvalid)) {
                    fContext.fErrors->error(expr.fPosition, "invalid expression");
                }
                break;
        }
        return INHERITED::visitExpression(expr);
    }

    bool definesLocalSize() const {
        return fLocalSizeX >= 0 || fLocalSizeY >= 0 || fLocalSizeZ >= 0;
    }

private:
    using INHERITED = ProgramVisitor;
    size_t fGlobalSlotsUsed = 0;
    const Context& fContext;
    const ProgramUsage& fUsage;
    // we pack the set/binding pair into a single 64 bit int
    THashSet<uint64_t> fBindings;

    // Compute programs must at least specify the X dimension of the local size. The other
    // dimensions have a default value of "1".
    int fLocalSizeX = -1;
    int fLocalSizeY = -1;
    int fLocalSizeZ = -1;
};

}  // namespace

void Analysis::DoFinalizationChecks(const Program& program) {
    // Check all of the program's owned elements. (Built-in elements are assumed to be valid.)
    FinalizationVisitor visitor{*program.fContext, *program.usage()};
    for (const std::unique_ptr<ProgramElement>& element : program.fOwnedElements) {
        visitor.visitProgramElement(*element);
    }
    if (ProgramConfig::IsCompute(program.fConfig->fKind) && !visitor.definesLocalSize()) {
        program.fContext->fErrors->error(Position(),
                                         "compute programs must specify a workgroup size");
    }
}

}  // namespace SkSL
