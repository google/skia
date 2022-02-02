/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkSLStatement.h"
#include "src/core/SkSafeMath.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/analysis/SkSLProgramVisitor.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"

namespace SkSL {
namespace {

class FinalizationVisitor : public ProgramVisitor {
public:
    FinalizationVisitor(const Context& c, const ProgramUsage& u) : fContext(c), fUsage(u) {}

    bool visitProgramElement(const ProgramElement& pe) override {
        switch (pe.kind()) {
            case ProgramElement::Kind::kGlobalVar: {
                this->checkGlobalVariableSizeLimit(pe.as<GlobalVarDeclaration>());
                break;
            }
            case ProgramElement::Kind::kFunction: {
                this->checkOutParamsAreAssigned(pe.as<FunctionDefinition>());
                break;
            }
            default:
                break;
        }
        return INHERITED::visitProgramElement(pe);
    }

    void checkGlobalVariableSizeLimit(const GlobalVarDeclaration& globalDecl) {
        const VarDeclaration& decl = globalDecl.declaration()->as<VarDeclaration>();

        size_t prevSlotsUsed = fGlobalSlotsUsed;
        fGlobalSlotsUsed = SkSafeMath::Add(fGlobalSlotsUsed, decl.var().type().slotCount());
        // To avoid overzealous error reporting, only trigger the error at the first place where the
        // global limit is exceeded.
        if (prevSlotsUsed < kVariableSlotLimit && fGlobalSlotsUsed >= kVariableSlotLimit) {
            fContext.fErrors->error(decl.fLine,
                                    "global variable '" + std::string(decl.var().name()) +
                                    "' exceeds the size limit");
        }
    }

    void checkOutParamsAreAssigned(const FunctionDefinition& funcDef) {
        const FunctionDeclaration& funcDecl = funcDef.declaration();

        // Searches for `out` parameters that are not written to. According to the GLSL spec,
        // the value of an out-param that's never assigned to is unspecified, so report it.
        // Structs are currently exempt from the rule because custom mesh specifications require an
        // `out` parameter for a Varyings struct, even if your mesh program doesn't need Varyings.
        for (const Variable* param : funcDecl.parameters()) {
            const int paramInout = param->modifiers().fFlags & (Modifiers::Flag::kIn_Flag |
                                                                Modifiers::Flag::kOut_Flag);
            if (!param->type().isStruct() && paramInout == Modifiers::Flag::kOut_Flag) {
                ProgramUsage::VariableCounts counts = fUsage.get(*param);
                if (counts.fWrite <= 0) {
                    fContext.fErrors->error(funcDecl.fLine,
                                            "function '" + std::string(funcDecl.name()) +
                                            "' never assigns a value to out parameter '" +
                                            std::string(param->name()) + "'");
                }
            }
        }
    }

    bool visitStatement(const Statement& stmt) override {
        if (!fContext.fConfig->fSettings.fPermitInvalidStaticTests) {
            switch (stmt.kind()) {
                case Statement::Kind::kIf:
                    if (stmt.as<IfStatement>().isStatic()) {
                        fContext.fErrors->error(stmt.fLine, "static if has non-static test");
                    }
                    break;

                case Statement::Kind::kSwitch:
                    if (stmt.as<SwitchStatement>().isStatic()) {
                        fContext.fErrors->error(stmt.fLine, "static switch has non-static test");
                    }
                    break;

                default:
                    break;
            }
        }
        return INHERITED::visitStatement(stmt);
    }

    bool visitExpression(const Expression& expr) override {
        switch (expr.kind()) {
            case Expression::Kind::kFunctionCall: {
                const FunctionDeclaration& decl = expr.as<FunctionCall>().function();
                if (!decl.isBuiltin() && !decl.definition()) {
                    fContext.fErrors->error(expr.fLine, "function '" + decl.description() +
                                                        "' is not defined");
                }
                break;
            }
            case Expression::Kind::kExternalFunctionReference:
            case Expression::Kind::kFunctionReference:
            case Expression::Kind::kMethodReference:
            case Expression::Kind::kTypeReference:
                SkDEBUGFAIL("invalid reference-expr, should have been reported by coerce()");
                fContext.fErrors->error(expr.fLine, "invalid expression");
                break;
            default:
                if (expr.type().matches(*fContext.fTypes.fInvalid)) {
                    fContext.fErrors->error(expr.fLine, "invalid expression");
                }
                break;
        }
        return INHERITED::visitExpression(expr);
    }

private:
    using INHERITED = ProgramVisitor;
    size_t fGlobalSlotsUsed = 0;
    const Context& fContext;
    const ProgramUsage& fUsage;
};

}  // namespace

void Analysis::DoFinalizationChecks(const Program& program) {
    // Check all of the program's owned elements. (Built-in elements are assumed to be valid.)
    FinalizationVisitor visitor{*program.fContext, *program.usage()};
    for (const std::unique_ptr<ProgramElement>& element : program.fOwnedElements) {
        visitor.visitProgramElement(*element);
    }
}

}  // namespace SkSL
