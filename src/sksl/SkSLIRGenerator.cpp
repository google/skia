/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLIRGenerator.h"

#include "limits.h"
#include <iterator>
#include <memory>
#include <unordered_set>

#include "include/private/SkSLLayout.h"
#include "include/private/SkTArray.h"
#include "include/sksl/DSLCore.h"
#include "src/core/SkScopeExit.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/SkSLIntrinsicMap.h"
#include "src/sksl/SkSLOperators.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLBreakStatement.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLContinueStatement.h"
#include "src/sksl/ir/SkSLDiscardStatement.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLExternalFunctionCall.h"
#include "src/sksl/ir/SkSLExternalFunctionReference.h"
#include "src/sksl/ir/SkSLField.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLFunctionPrototype.h"
#include "src/sksl/ir/SkSLFunctionReference.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLLiteral.h"
#include "src/sksl/ir/SkSLMethodReference.h"
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLPoison.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLSetting.h"
#include "src/sksl/ir/SkSLStructDefinition.h"
#include "src/sksl/ir/SkSLSwitchCase.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLUnresolvedFunction.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"

namespace SkSL {

IRGenerator::IRGenerator(const Context* context)
        : fContext(*context) {}

void IRGenerator::appendRTAdjustFixupToVertexMain(const FunctionDeclaration& decl, Block* body) {
    using namespace SkSL::dsl;
    using SkSL::dsl::Swizzle;  // disambiguate from SkSL::Swizzle
    using OwnerKind = SkSL::FieldAccess::OwnerKind;

    // If this is a vertex program that uses RTAdjust, and this is main()...
    ThreadContext::RTAdjustData& rtAdjust = ThreadContext::RTAdjustState();
    if ((rtAdjust.fVar || rtAdjust.fInterfaceBlock) && decl.isMain() &&
        ProgramKind::kVertex == this->programKind()) {
        // ... append a line to the end of the function body which fixes up sk_Position.
        const Variable* skPerVertex = nullptr;
        if (const ProgramElement* perVertexDecl =
                fContext.fIntrinsics->find(Compiler::PERVERTEX_NAME)) {
            SkASSERT(perVertexDecl->is<SkSL::InterfaceBlock>());
            skPerVertex = &perVertexDecl->as<SkSL::InterfaceBlock>().variable();
        }

        SkASSERT(skPerVertex);
        auto Ref = [](const Variable* var) -> std::unique_ptr<Expression> {
            return VariableReference::Make(/*line=*/-1, var);
        };
        auto Field = [&](const Variable* var, int idx) -> std::unique_ptr<Expression> {
            return FieldAccess::Make(fContext, Ref(var), idx, OwnerKind::kAnonymousInterfaceBlock);
        };
        auto Pos = [&]() -> DSLExpression {
            return DSLExpression(FieldAccess::Make(fContext, Ref(skPerVertex), /*fieldIndex=*/0,
                                                   OwnerKind::kAnonymousInterfaceBlock));
        };
        auto Adjust = [&]() -> DSLExpression {
            return DSLExpression(rtAdjust.fInterfaceBlock
                                         ? Field(rtAdjust.fInterfaceBlock, rtAdjust.fFieldIndex)
                                         : Ref(rtAdjust.fVar));
        };

        auto fixupStmt = DSLStatement(
            Pos() = Float4(Swizzle(Pos(), X, Y) * Swizzle(Adjust(), X, Z) +
                           Swizzle(Pos(), W, W) * Swizzle(Adjust(), Y, W),
                           0,
                           Pos().w())
        );

        body->children().push_back(fixupStmt.release());
    }
}

void IRGenerator::CheckModifiers(const Context& context,
                                 int line,
                                 const Modifiers& modifiers,
                                 int permittedModifierFlags,
                                 int permittedLayoutFlags) {
    static constexpr struct { Modifiers::Flag flag; const char* name; } kModifierFlags[] = {
        { Modifiers::kConst_Flag,          "const" },
        { Modifiers::kIn_Flag,             "in" },
        { Modifiers::kOut_Flag,            "out" },
        { Modifiers::kUniform_Flag,        "uniform" },
        { Modifiers::kFlat_Flag,           "flat" },
        { Modifiers::kNoPerspective_Flag,  "noperspective" },
        { Modifiers::kHasSideEffects_Flag, "sk_has_side_effects" },
        { Modifiers::kInline_Flag,         "inline" },
        { Modifiers::kNoInline_Flag,       "noinline" },
        { Modifiers::kHighp_Flag,          "highp" },
        { Modifiers::kMediump_Flag,        "mediump" },
        { Modifiers::kLowp_Flag,           "lowp" },
        { Modifiers::kES3_Flag,            "$es3" },
    };

    int modifierFlags = modifiers.fFlags;
    for (const auto& f : kModifierFlags) {
        if (modifierFlags & f.flag) {
            if (!(permittedModifierFlags & f.flag)) {
                context.fErrors->error(line, "'" + String(f.name) + "' is not permitted here");
            }
            modifierFlags &= ~f.flag;
        }
    }
    SkASSERT(modifierFlags == 0);

    static constexpr struct { Layout::Flag flag; const char* name; } kLayoutFlags[] = {
        { Layout::kOriginUpperLeft_Flag,          "origin_upper_left"},
        { Layout::kPushConstant_Flag,             "push_constant"},
        { Layout::kBlendSupportAllEquations_Flag, "blend_support_all_equations"},
        { Layout::kSRGBUnpremul_Flag,             "srgb_unpremul"},
        { Layout::kLocation_Flag,                 "location"},
        { Layout::kOffset_Flag,                   "offset"},
        { Layout::kBinding_Flag,                  "binding"},
        { Layout::kIndex_Flag,                    "index"},
        { Layout::kSet_Flag,                      "set"},
        { Layout::kBuiltin_Flag,                  "builtin"},
        { Layout::kInputAttachmentIndex_Flag,     "input_attachment_index"},
    };

    int layoutFlags = modifiers.fLayout.fFlags;
    for (const auto& lf : kLayoutFlags) {
        if (layoutFlags & lf.flag) {
            if (!(permittedLayoutFlags & lf.flag)) {
                context.fErrors->error(
                        line, "layout qualifier '" + String(lf.name) + "' is not permitted here");
            }
            layoutFlags &= ~lf.flag;
        }
    }
    SkASSERT(layoutFlags == 0);
}

std::unique_ptr<Expression> IRGenerator::convertIdentifier(int line, skstd::string_view name) {
    const Symbol* result = (*fSymbolTable)[name];
    if (!result) {
        this->errorReporter().error(line, "unknown identifier '" + name + "'");
        return nullptr;
    }
    switch (result->kind()) {
        case Symbol::Kind::kFunctionDeclaration: {
            std::vector<const FunctionDeclaration*> f = {
                &result->as<FunctionDeclaration>()
            };
            return std::make_unique<FunctionReference>(fContext, line, f);
        }
        case Symbol::Kind::kUnresolvedFunction: {
            const UnresolvedFunction* f = &result->as<UnresolvedFunction>();
            return std::make_unique<FunctionReference>(fContext, line, f->functions());
        }
        case Symbol::Kind::kVariable: {
            const Variable* var = &result->as<Variable>();
            const Modifiers& modifiers = var->modifiers();
            switch (modifiers.fLayout.fBuiltin) {
                case SK_FRAGCOORD_BUILTIN:
                    if (caps().canUseFragCoord()) {
                        fInputs.fUseFlipRTUniform = true;
                    }
                    break;
                case SK_CLOCKWISE_BUILTIN:
                    fInputs.fUseFlipRTUniform = true;
                    break;
            }
            // default to kRead_RefKind; this will be corrected later if the variable is written to
            return VariableReference::Make(line, var, VariableReference::RefKind::kRead);
        }
        case Symbol::Kind::kField: {
            const Field* field = &result->as<Field>();
            auto base = VariableReference::Make(line, &field->owner(),
                                                VariableReference::RefKind::kRead);
            return FieldAccess::Make(fContext, std::move(base), field->fieldIndex(),
                                     FieldAccess::OwnerKind::kAnonymousInterfaceBlock);
        }
        case Symbol::Kind::kType: {
            return TypeReference::Convert(fContext, line, &result->as<Type>());
        }
        case Symbol::Kind::kExternal: {
            const ExternalFunction* r = &result->as<ExternalFunction>();
            return std::make_unique<ExternalFunctionReference>(line, r);
        }
        default:
            SK_ABORT("unsupported symbol type %d\n", (int) result->kind());
    }
}

void IRGenerator::start(const ParsedModule& base,
                        std::vector<std::unique_ptr<ProgramElement>>* elements,
                        std::vector<const ProgramElement*>* sharedElements) {
    fProgramElements = elements;
    fSharedElements = sharedElements;
    fSymbolTable = base.fSymbols;

    fInputs = {};
    fDefinedStructs.clear();
    SymbolTable::Push(&fSymbolTable, fContext.fConfig->fIsBuiltinCode);

    if (this->settings().fExternalFunctions) {
        // Add any external values to the new symbol table, so they're only visible to this Program.
        for (const std::unique_ptr<ExternalFunction>& ef : *this->settings().fExternalFunctions) {
            fSymbolTable->addWithoutOwnership(ef.get());
        }
    }

    if (this->isRuntimeEffect() && !fContext.fConfig->fSettings.fEnforceES2Restrictions) {
        // We're compiling a runtime effect, but we're not enforcing ES2 restrictions. Add various
        // non-ES2 types to our symbol table to allow them to be tested.
        fSymbolTable->addAlias("mat2x2", fContext.fTypes.fFloat2x2.get());
        fSymbolTable->addAlias("mat2x3", fContext.fTypes.fFloat2x3.get());
        fSymbolTable->addAlias("mat2x4", fContext.fTypes.fFloat2x4.get());
        fSymbolTable->addAlias("mat3x2", fContext.fTypes.fFloat3x2.get());
        fSymbolTable->addAlias("mat3x3", fContext.fTypes.fFloat3x3.get());
        fSymbolTable->addAlias("mat3x4", fContext.fTypes.fFloat3x4.get());
        fSymbolTable->addAlias("mat4x2", fContext.fTypes.fFloat4x2.get());
        fSymbolTable->addAlias("mat4x3", fContext.fTypes.fFloat4x3.get());
        fSymbolTable->addAlias("mat4x4", fContext.fTypes.fFloat4x4.get());

        fSymbolTable->addAlias("float2x3", fContext.fTypes.fFloat2x3.get());
        fSymbolTable->addAlias("float2x4", fContext.fTypes.fFloat2x4.get());
        fSymbolTable->addAlias("float3x2", fContext.fTypes.fFloat3x2.get());
        fSymbolTable->addAlias("float3x4", fContext.fTypes.fFloat3x4.get());
        fSymbolTable->addAlias("float4x2", fContext.fTypes.fFloat4x2.get());
        fSymbolTable->addAlias("float4x3", fContext.fTypes.fFloat4x3.get());

        fSymbolTable->addAlias("half2x3", fContext.fTypes.fHalf2x3.get());
        fSymbolTable->addAlias("half2x4", fContext.fTypes.fHalf2x4.get());
        fSymbolTable->addAlias("half3x2", fContext.fTypes.fHalf3x2.get());
        fSymbolTable->addAlias("half3x4", fContext.fTypes.fHalf3x4.get());
        fSymbolTable->addAlias("half4x2", fContext.fTypes.fHalf4x2.get());
        fSymbolTable->addAlias("half4x3", fContext.fTypes.fHalf4x3.get());

        fSymbolTable->addAlias("uint", fContext.fTypes.fUInt.get());
        fSymbolTable->addAlias("uint2", fContext.fTypes.fUInt2.get());
        fSymbolTable->addAlias("uint3", fContext.fTypes.fUInt3.get());
        fSymbolTable->addAlias("uint4", fContext.fTypes.fUInt4.get());

        fSymbolTable->addAlias("short", fContext.fTypes.fShort.get());
        fSymbolTable->addAlias("short2", fContext.fTypes.fShort2.get());
        fSymbolTable->addAlias("short3", fContext.fTypes.fShort3.get());
        fSymbolTable->addAlias("short4", fContext.fTypes.fShort4.get());

        fSymbolTable->addAlias("ushort", fContext.fTypes.fUShort.get());
        fSymbolTable->addAlias("ushort2", fContext.fTypes.fUShort2.get());
        fSymbolTable->addAlias("ushort3", fContext.fTypes.fUShort3.get());
        fSymbolTable->addAlias("ushort4", fContext.fTypes.fUShort4.get());
    }
}

IRGenerator::IRBundle IRGenerator::finish() {
    return IRBundle{std::move(*fProgramElements),
                    std::move(*fSharedElements),
                    std::move(fSymbolTable),
                    fInputs};
}

}  // namespace SkSL
