/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLVarDeclarations.h"

#include "include/private/SkSLLayout.h"
#include "include/private/SkSLModifiers.h"
#include "include/private/SkSLProgramKind.h"
#include "include/private/SkSLString.h"
#include "include/sksl/SkSLErrorReporter.h"
#include "include/sksl/SkSLPosition.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLType.h"

#include <cstddef>
#include <string_view>
#include <vector>

namespace SkSL {
namespace {

static bool check_valid_uniform_type(Position pos,
                                     const Type* t,
                                     const Context& context,
                                     bool topLevel = true) {
    const Type& ct = t->componentType();

    // In RuntimeEffects we only allow a restricted set of types, namely shader/blender/colorFilter,
    // 32-bit signed integers, 16-bit and 32-bit floats, and their composites.
    {
        bool error = false;
        if (ProgramConfig::IsRuntimeEffect(context.fConfig->fKind)) {
            // `shader`, `blender`, `colorFilter`
            if (t->isEffectChild()) {
                return true;
            }

            // `int`, `int2`, `int3`, `int4`
            if (ct.isSigned() && ct.bitWidth() == 32 && (t->isScalar() || t->isVector())) {
                return true;
            }

            // `float`, `float2`, `float3`, `float4`, `float2x2`, `float3x3`, `float4x4`
            // `half`, `half2`, `half3`, `half4`, `half2x2`, `half3x3`, `half4x4`
            if (ct.isFloat() &&
                (t->isScalar() || t->isVector() || (t->isMatrix() && t->rows() == t->columns()))) {
                return true;
            }

            // Everything else is an error.
            error = true;
        }

        // We disallow boolean uniforms in SkSL since they are not well supported by backend
        // platforms and drivers. We disallow atomic variables in uniforms as that doesn't map
        // cleanly to all backends.
        if (error || (ct.isBoolean() && (t->isScalar() || t->isVector())) || ct.isAtomic()) {
            context.fErrors->error(
                    pos, "variables of type '" + t->displayName() + "' may not be uniform");
            return false;
        }
    }

    // In non-RTE SkSL we allow structs and interface blocks to be uniforms but we must make sure
    // their fields are allowed.
    if (t->isStruct()) {
        for (const Type::Field& field : t->fields()) {
            if (!check_valid_uniform_type(
                        field.fPosition, field.fType, context, /*topLevel=*/false)) {
                // Emit a "caused by" line only for the top-level uniform type and not for any
                // nested structs.
                if (topLevel) {
                    context.fErrors->error(pos, "caused by:");
                }
                return false;
            }
        }
    }
    return true;
}

}  // namespace

std::unique_ptr<Statement> VarDeclaration::clone() const {
    // Cloning a VarDeclaration is inherently problematic, as we normally expect a one-to-one
    // mapping between Variables and VarDeclarations and a straightforward clone would violate this
    // assumption. We could of course theoretically clone the Variable as well, but that would
    // require additional context and tracking, since for the whole process to work we would also
    // have to fixup any subsequent VariableReference clones to point to the newly cloned Variables
    // instead of the originals.
    //
    // Since the only reason we ever clone VarDeclarations is to support tests of clone() and we do
    // not expect to ever need to do so otherwise, a full solution to this issue is unnecessary at
    // the moment. We instead just keep track of whether a VarDeclaration is a clone so we can
    // handle its cleanup properly. This allows clone() to work in the simple case that a
    // VarDeclaration's clone does not outlive the original, which is adequate for testing. Since
    // this leaves a sharp  edge in place - destroying the original could cause a use-after-free in
    // some circumstances - we also disable cloning altogether unless the
    // fAllowVarDeclarationCloneForTesting ProgramSetting is enabled.
    if (ThreadContext::Settings().fAllowVarDeclarationCloneForTesting) {
        return std::make_unique<VarDeclaration>(this->var(),
                                                &this->baseType(),
                                                fArraySize,
                                                this->value() ? this->value()->clone() : nullptr,
                                                /*isClone=*/true);
    } else {
        SkDEBUGFAIL("VarDeclaration::clone() is unsupported");
        return nullptr;
    }
}

std::string VarDeclaration::description() const {
    std::string result = this->var()->modifiers().description() + this->baseType().description() +
                         " " + std::string(this->var()->name());
    if (this->arraySize() > 0) {
        String::appendf(&result, "[%d]", this->arraySize());
    }
    if (this->value()) {
        result += " = " + this->value()->description();
    }
    result += ";";
    return result;
}

void VarDeclaration::ErrorCheck(const Context& context,
                                Position pos,
                                Position modifiersPosition,
                                const Modifiers& modifiers,
                                const Type* type,
                                Variable::Storage storage) {
    const Type* baseType = type;
    if (baseType->isArray()) {
        baseType = &baseType->componentType();
    }
    SkASSERT(!baseType->isArray());

    if (baseType->matches(*context.fTypes.fInvalid)) {
        context.fErrors->error(pos, "invalid type");
        return;
    }
    if (baseType->isVoid()) {
        context.fErrors->error(pos, "variables of type 'void' are not allowed");
        return;
    }

    if (baseType->componentType().isOpaque() && !baseType->componentType().isAtomic() &&
        storage != Variable::Storage::kGlobal) {
        context.fErrors->error(pos,
                "variables of type '" + baseType->displayName() + "' must be global");
    }
    if ((modifiers.fFlags & Modifiers::kIn_Flag) && baseType->isMatrix()) {
        context.fErrors->error(pos, "'in' variables may not have matrix type");
    }
    if ((modifiers.fFlags & Modifiers::kIn_Flag) && type->isUnsizedArray()) {
        context.fErrors->error(pos, "'in' variables may not have unsized array type");
    }
    if ((modifiers.fFlags & Modifiers::kOut_Flag) && type->isUnsizedArray()) {
        context.fErrors->error(pos, "'out' variables may not have unsized array type");
    }
    if ((modifiers.fFlags & Modifiers::kIn_Flag) && (modifiers.fFlags & Modifiers::kUniform_Flag)) {
        context.fErrors->error(pos, "'in uniform' variables not permitted");
    }
    if ((modifiers.fFlags & Modifiers::kReadOnly_Flag) &&
        (modifiers.fFlags & Modifiers::kWriteOnly_Flag)) {
        context.fErrors->error(pos, "'readonly' and 'writeonly' qualifiers cannot be combined");
    }
    if ((modifiers.fFlags & Modifiers::kUniform_Flag) &&
        (modifiers.fFlags & Modifiers::kBuffer_Flag)) {
        context.fErrors->error(pos, "'uniform buffer' variables not permitted");
    }
    if ((modifiers.fFlags & Modifiers::kWorkgroup_Flag) &&
        (modifiers.fFlags & (Modifiers::kIn_Flag | Modifiers::kOut_Flag))) {
        context.fErrors->error(pos, "in / out variables may not be declared workgroup");
    }
    if ((modifiers.fFlags & Modifiers::kUniform_Flag)) {
        check_valid_uniform_type(pos, baseType, context);
    }
    if (baseType->isEffectChild() && !(modifiers.fFlags & Modifiers::kUniform_Flag)) {
        context.fErrors->error(pos,
                "variables of type '" + baseType->displayName() + "' must be uniform");
    }
    if (baseType->isEffectChild() && (context.fConfig->fKind == ProgramKind::kMeshVertex ||
                                      context.fConfig->fKind == ProgramKind::kMeshFragment)) {
        context.fErrors->error(pos, "effects are not permitted in custom mesh shaders");
    }
    if (baseType->isOrContainsAtomic()) {
        // An atomic variable (or a struct or an array that contains an atomic member) must be
        // either:
        //   a. Declared as a workgroup-shared variable, OR
        //   b. Declared as the member of writable storage buffer block (i.e. has no readonly
        //   restriction).
        //
        // The checks below will enforce these two rules on all declarations. If the variable is not
        // declared with the workgroup modifier, then it must be declared in the interface block
        // storage. If this is the declaration for an interface block that contains an atomic
        // member, then it must have the `buffer` modifier and no `readonly` modifier.
        bool isWorkgroup = modifiers.fFlags & Modifiers::kWorkgroup_Flag;
        bool isBlockMember = (storage == Variable::Storage::kInterfaceBlock);
        bool isWritableStorageBuffer = modifiers.fFlags & Modifiers::kBuffer_Flag &&
                                       !(modifiers.fFlags & Modifiers::kReadOnly_Flag);

        if (!isWorkgroup &&
            !(baseType->isInterfaceBlock() ? isWritableStorageBuffer : isBlockMember)) {
            context.fErrors->error(pos,
                                   "atomics are only permitted in workgroup variables and writable "
                                   "storage blocks");
        }
    }
    if (modifiers.fLayout.fFlags & Layout::kColor_Flag) {
        if (!ProgramConfig::IsRuntimeEffect(context.fConfig->fKind)) {
            context.fErrors->error(pos, "'layout(color)' is only permitted in runtime effects");
        }
        if (!(modifiers.fFlags & Modifiers::kUniform_Flag)) {
            context.fErrors->error(pos,
                                   "'layout(color)' is only permitted on 'uniform' variables");
        }
        auto validColorXformType = [](const Type& t) {
            return t.isVector() && t.componentType().isFloat() &&
                   (t.columns() == 3 || t.columns() == 4);
        };
        if (!validColorXformType(*baseType)) {
            context.fErrors->error(pos,
                                   "'layout(color)' is not permitted on variables of type '" +
                                           baseType->displayName() + "'");
        }
    }

    int permitted = Modifiers::kConst_Flag | Modifiers::kHighp_Flag | Modifiers::kMediump_Flag |
                    Modifiers::kLowp_Flag;
    if (storage == Variable::Storage::kGlobal) {
        // Uniforms are allowed in all programs
        permitted |= Modifiers::kUniform_Flag;

        // No other modifiers are allowed in runtime effects.
        if (!ProgramConfig::IsRuntimeEffect(context.fConfig->fKind)) {
            if (baseType->isInterfaceBlock()) {
                // Interface blocks allow `buffer`.
                permitted |= Modifiers::kBuffer_Flag;

                if (modifiers.fFlags & Modifiers::kBuffer_Flag) {
                    // Only storage blocks allow `readonly` and `writeonly`.
                    // (`readonly` and `writeonly` textures are converted to separate types via
                    // applyAccessQualifiers.)
                    permitted |= Modifiers::kReadOnly_Flag | Modifiers::kWriteOnly_Flag;
                }

                // It is an error for an unsized array to appear anywhere but the last member of a
                // "buffer" block.
                const auto& fields = baseType->fields();
                const size_t illegalRangeEnd =
                        fields.size() - ((modifiers.fFlags & Modifiers::kBuffer_Flag) ? 1 : 0);
                for (size_t i = 0; i < illegalRangeEnd; ++i) {
                    if (fields[i].fType->isUnsizedArray()) {
                        context.fErrors->error(
                                fields[i].fPosition,
                                "unsized array must be the last member of a storage block");
                    }
                }
            }

            if (!baseType->isOpaque()) {
                // Only non-opaque types allow `in` and `out`.
                permitted |= Modifiers::kIn_Flag | Modifiers::kOut_Flag;
            }
            if (ProgramConfig::IsCompute(context.fConfig->fKind)) {
                // Only compute shaders allow `workgroup`.
                if (!baseType->isOpaque() || baseType->isAtomic()) {
                    permitted |= Modifiers::kWorkgroup_Flag;
                }
            } else {
                // Only vertex/fragment shaders allow `flat` and `noperspective`.
                permitted |= Modifiers::kFlat_Flag | Modifiers::kNoPerspective_Flag;
            }
        }
    }

    int permittedLayoutFlags = ~0;

    // The `texture` and `sampler` modifiers can be present respectively on a texture and sampler or
    // simultaneously on a combined image-sampler but they are not permitted on any other type.
    switch (baseType->typeKind()) {
        case Type::TypeKind::kSampler:
            // Both texture and sampler flags are permitted
            break;
        case Type::TypeKind::kTexture:
            permittedLayoutFlags &= ~Layout::kSampler_Flag;
            break;
        case Type::TypeKind::kSeparateSampler:
            permittedLayoutFlags &= ~Layout::kTexture_Flag;
            break;
        default:
            permittedLayoutFlags &= ~(Layout::kTexture_Flag | Layout::kSampler_Flag);
            break;
    }

    // We don't allow 'binding' or 'set' on normal uniform variables, only on textures, samplers,
    // and interface blocks (holding uniform variables). They're also only allowed at global scope,
    // not on interface block fields (or locals/parameters).
    bool permitBindingAndSet = baseType->typeKind() == Type::TypeKind::kSampler ||
                               baseType->typeKind() == Type::TypeKind::kSeparateSampler ||
                               baseType->typeKind() == Type::TypeKind::kTexture ||
                               baseType->isInterfaceBlock();
    if (storage != Variable::Storage::kGlobal ||
        ((modifiers.fFlags & Modifiers::kUniform_Flag) && !permitBindingAndSet)) {
        permittedLayoutFlags &= ~Layout::kBinding_Flag;
        permittedLayoutFlags &= ~Layout::kSet_Flag;
        permittedLayoutFlags &= ~Layout::kSPIRV_Flag;
        permittedLayoutFlags &= ~Layout::kMetal_Flag;
        permittedLayoutFlags &= ~Layout::kWGSL_Flag;
        permittedLayoutFlags &= ~Layout::kGL_Flag;
    }
    if (ProgramConfig::IsRuntimeEffect(context.fConfig->fKind)) {
        // Disallow all layout flags except 'color' in runtime effects
        permittedLayoutFlags &= Layout::kColor_Flag;
    }
    modifiers.checkPermitted(context, modifiersPosition, permitted, permittedLayoutFlags);
}

bool VarDeclaration::ErrorCheckAndCoerce(const Context& context, const Variable& var,
        std::unique_ptr<Expression>& value) {
    ErrorCheck(context, var.fPosition, var.modifiersPosition(), var.modifiers(), &var.type(),
            var.storage());
    if (value) {
        if (var.type().isOpaque()) {
            context.fErrors->error(value->fPosition, "opaque type '" + var.type().displayName() +
                                                     "' cannot use initializer expressions");
            return false;
        }
        if (var.modifiers().fFlags & Modifiers::kIn_Flag) {
            context.fErrors->error(value->fPosition,
                                   "'in' variables cannot use initializer expressions");
            return false;
        }
        if (var.modifiers().fFlags & Modifiers::kUniform_Flag) {
            context.fErrors->error(value->fPosition,
                                   "'uniform' variables cannot use initializer expressions");
            return false;
        }
        if (var.storage() == Variable::Storage::kInterfaceBlock) {
            context.fErrors->error(value->fPosition,
                                   "initializers are not permitted on interface block fields");
            return false;
        }
        value = var.type().coerceExpression(std::move(value), context);
        if (!value) {
            return false;
        }
    }
    if (var.modifiers().fFlags & Modifiers::kConst_Flag) {
        if (!value) {
            context.fErrors->error(var.fPosition, "'const' variables must be initialized");
            return false;
        }
        if (!Analysis::IsConstantExpression(*value)) {
            context.fErrors->error(value->fPosition,
                                   "'const' variable initializer must be a constant expression");
            return false;
        }
    }
    if (var.storage() == Variable::Storage::kInterfaceBlock) {
        if (var.type().isOpaque()) {
            context.fErrors->error(var.fPosition, "opaque type '" + var.type().displayName() +
                    "' is not permitted in an interface block");
            return false;
        }
    }
    if (var.storage() == Variable::Storage::kGlobal) {
        if (value && !Analysis::IsConstantExpression(*value)) {
            context.fErrors->error(value->fPosition,
                                   "global variable initializer must be a constant expression");
            return false;
        }
    }
    return true;
}

std::unique_ptr<Statement> VarDeclaration::Convert(const Context& context,
                                                   std::unique_ptr<Variable> var,
                                                   std::unique_ptr<Expression> value,
                                                   bool addToSymbolTable) {
    if (!ErrorCheckAndCoerce(context, *var, value)) {
        return nullptr;
    }
    const Type* baseType = &var->type();
    int arraySize = 0;
    if (baseType->isArray()) {
        arraySize = baseType->columns();
        baseType = &baseType->componentType();
    }
    std::unique_ptr<Statement> varDecl = VarDeclaration::Make(context, var.get(), baseType,
                                                              arraySize, std::move(value));
    if (!varDecl) {
        return nullptr;
    }

    SymbolTable* symbols = ThreadContext::SymbolTable().get();
    if (var->storage() == Variable::Storage::kGlobal ||
        var->storage() == Variable::Storage::kInterfaceBlock) {
        // Check if this globally-scoped variable name overlaps an existing symbol name.
        if (symbols->find(var->name())) {
            context.fErrors->error(var->fPosition,
                                   "symbol '" + std::string(var->name()) + "' was already defined");
            return nullptr;
        }

        // `sk_RTAdjust` is special, and makes the IR generator emit position-fixup expressions.
        if (var->name() == Compiler::RTADJUST_NAME) {
            if (ThreadContext::RTAdjustState().fVar ||
                ThreadContext::RTAdjustState().fInterfaceBlock) {
                context.fErrors->error(var->fPosition, "duplicate definition of 'sk_RTAdjust'");
                return nullptr;
            }
            if (!var->type().matches(*context.fTypes.fFloat4)) {
                context.fErrors->error(var->fPosition, "sk_RTAdjust must have type 'float4'");
                return nullptr;
            }
            ThreadContext::RTAdjustState().fVar = var.get();
        }
    }

    if (addToSymbolTable) {
        symbols->add(std::move(var));
    } else {
        symbols->takeOwnershipOfSymbol(std::move(var));
    }
    return varDecl;
}

std::unique_ptr<Statement> VarDeclaration::Make(const Context& context, Variable* var,
        const Type* baseType, int arraySize, std::unique_ptr<Expression> value) {
    SkASSERT(!baseType->isArray());
    // function parameters cannot have variable declarations
    SkASSERT(var->storage() != Variable::Storage::kParameter);
    // 'const' variables must be initialized
    SkASSERT(!(var->modifiers().fFlags & Modifiers::kConst_Flag) || value);
    // 'const' variable initializer must be a constant expression
    SkASSERT(!(var->modifiers().fFlags & Modifiers::kConst_Flag) ||
             Analysis::IsConstantExpression(*value));
    // global variable initializer must be a constant expression
    SkASSERT(!(value && var->storage() == Variable::Storage::kGlobal &&
               !Analysis::IsConstantExpression(*value)));
    // opaque type not permitted on an interface block
    SkASSERT(!(var->storage() == Variable::Storage::kInterfaceBlock && var->type().isOpaque()));
    // initializers are not permitted on interface block fields
    SkASSERT(!(var->storage() == Variable::Storage::kInterfaceBlock && value));
    // opaque type cannot use initializer expressions
    SkASSERT(!(value && var->type().isOpaque()));
    // 'in' variables cannot use initializer expressions
    SkASSERT(!(value && (var->modifiers().fFlags & Modifiers::kIn_Flag)));
    // 'uniform' variables cannot use initializer expressions
    SkASSERT(!(value && (var->modifiers().fFlags & Modifiers::kUniform_Flag)));

    auto result = std::make_unique<VarDeclaration>(var, baseType, arraySize, std::move(value));
    var->setVarDeclaration(result.get());
    return std::move(result);
}

}  // namespace SkSL
