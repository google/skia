/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLVarDeclarations.h"

#include "include/core/SkSpan.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkEnumBitMask.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/SkSLProgramKind.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLString.h"
#include "src/sksl/ir/SkSLLayout.h"
#include "src/sksl/ir/SkSLModifierFlags.h"
#include "src/sksl/ir/SkSLModifiers.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLType.h"

#include <string_view>

namespace SkSL {
namespace {

static bool check_valid_uniform_type(Position pos,
                                     const Type* t,
                                     const Context& context,
                                     bool topLevel = true) {
    auto reportError = [&]() {
        context.fErrors->error(pos, "variables of type '" + t->displayName() +
                                    "' may not be uniform");
    };

    // In Runtime Effects we only allow a restricted set of types: shader, blender, colorFilter,
    // 32-bit signed integers, 16-bit and 32-bit floats, and their vector/square-matrix composites.
    if (ProgramConfig::IsRuntimeEffect(context.fConfig->fKind)) {
        // `shader`, `blender`, `colorFilter`
        if (t->isEffectChild()) {
            return true;
        }

        // `int`, `int2`, `int3`, `int4`
        const Type& ct = t->componentType();
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
        reportError();
        return false;
    }

    Position errorPosition = {};
    if (!t->isAllowedInUniform(&errorPosition)) {
        reportError();
        if (errorPosition.valid()) {
            context.fErrors->error(errorPosition, "caused by:");
        }
        return false;
    }

    return true;
}

}  // namespace

std::string VarDeclaration::description() const {
    std::string result = this->var()->layout().paddedDescription() +
                         this->var()->modifierFlags().paddedDescription() +
                         this->baseType().description() + ' ' + std::string(this->var()->name());
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
                                const Layout& layout,
                                ModifierFlags modifierFlags,
                                const Type* type,
                                const Type* baseType,
                                Variable::Storage storage) {
    SkASSERT(type->isArray() ? baseType->matches(type->componentType())
                             : baseType->matches(*type));

    if (baseType->componentType().isOpaque() && !baseType->componentType().isAtomic() &&
        storage != Variable::Storage::kGlobal) {
        context.fErrors->error(pos, "variables of type '" + baseType->displayName() +
                                    "' must be global");
    }
    if ((modifierFlags & ModifierFlag::kIn) && baseType->isMatrix()) {
        context.fErrors->error(pos, "'in' variables may not have matrix type");
    }
    if ((modifierFlags & ModifierFlag::kIn) && type->isUnsizedArray()) {
        context.fErrors->error(pos, "'in' variables may not have unsized array type");
    }
    if ((modifierFlags & ModifierFlag::kOut) && type->isUnsizedArray()) {
        context.fErrors->error(pos, "'out' variables may not have unsized array type");
    }
    if ((modifierFlags & ModifierFlag::kIn) && modifierFlags.isUniform()) {
        context.fErrors->error(pos, "'in uniform' variables not permitted");
    }
    if (modifierFlags.isReadOnly() && modifierFlags.isWriteOnly()) {
        context.fErrors->error(pos, "'readonly' and 'writeonly' qualifiers cannot be combined");
    }
    if (modifierFlags.isUniform() && modifierFlags.isBuffer()) {
        context.fErrors->error(pos, "'uniform buffer' variables not permitted");
    }
    if (modifierFlags.isWorkgroup() && (modifierFlags & (ModifierFlag::kIn |
                                                         ModifierFlag::kOut))) {
        context.fErrors->error(pos, "in / out variables may not be declared workgroup");
    }
    if (modifierFlags.isUniform()) {
        check_valid_uniform_type(pos, baseType, context);
    }
    if (baseType->isEffectChild() && !modifierFlags.isUniform()) {
        context.fErrors->error(pos, "variables of type '" + baseType->displayName() +
                                    "' must be uniform");
    }
    if (baseType->isEffectChild() && context.fConfig->fKind == ProgramKind::kMeshVertex) {
        context.fErrors->error(pos, "effects are not permitted in mesh vertex shaders");
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
        bool isBlockMember = (storage == Variable::Storage::kInterfaceBlock);
        bool isWritableStorageBuffer = modifierFlags.isBuffer() && !modifierFlags.isReadOnly();

        if (!modifierFlags.isWorkgroup() &&
            !(baseType->isInterfaceBlock() ? isWritableStorageBuffer : isBlockMember)) {
            context.fErrors->error(pos, "atomics are only permitted in workgroup variables and "
                                        "writable storage blocks");
        }
    }
    if (layout.fFlags & LayoutFlag::kColor) {
        if (!ProgramConfig::IsRuntimeEffect(context.fConfig->fKind)) {
            context.fErrors->error(pos, "'layout(color)' is only permitted in runtime effects");
        }
        if (!modifierFlags.isUniform()) {
            context.fErrors->error(pos, "'layout(color)' is only permitted on 'uniform' variables");
        }
        auto validColorXformType = [](const Type& t) {
            return t.isVector() && t.componentType().isFloat() &&
                   (t.columns() == 3 || t.columns() == 4);
        };
        if (!validColorXformType(*baseType)) {
            context.fErrors->error(pos, "'layout(color)' is not permitted on variables of type '" +
                                        baseType->displayName() + "'");
        }
    }

    ModifierFlags permitted = ModifierFlag::kConst | ModifierFlag::kHighp | ModifierFlag::kMediump |
                              ModifierFlag::kLowp;
    if (storage == Variable::Storage::kGlobal) {
        // Uniforms are allowed in all programs
        permitted |= ModifierFlag::kUniform;

        // No other modifiers are allowed in runtime effects.
        if (!ProgramConfig::IsRuntimeEffect(context.fConfig->fKind)) {
            if (baseType->isInterfaceBlock()) {
                // Interface blocks allow `buffer`.
                permitted |= ModifierFlag::kBuffer;

                if (modifierFlags.isBuffer()) {
                    // Only storage blocks allow `readonly` and `writeonly`.
                    // (`readonly` and `writeonly` textures are converted to separate types via
                    // applyAccessQualifiers.)
                    permitted |= ModifierFlag::kReadOnly | ModifierFlag::kWriteOnly;
                }

                // It is an error for an unsized array to appear anywhere but the last member of a
                // "buffer" block.
                const auto& fields = baseType->fields();
                const int illegalRangeEnd = SkToInt(fields.size()) -
                                            (modifierFlags.isBuffer() ? 1 : 0);
                for (int i = 0; i < illegalRangeEnd; ++i) {
                    if (fields[i].fType->isUnsizedArray()) {
                        context.fErrors->error(
                                fields[i].fPosition,
                                "unsized array must be the last member of a storage block");
                    }
                }
            }

            if (!baseType->isOpaque()) {
                // Only non-opaque types allow `in` and `out`.
                permitted |= ModifierFlag::kIn | ModifierFlag::kOut;
            }
            if (ProgramConfig::IsFragment(context.fConfig->fKind) && baseType->isStruct() &&
                !baseType->isInterfaceBlock()) {
                // Only structs in fragment shaders allow `pixel_local`.
                permitted |= ModifierFlag::kPixelLocal;
            }
            if (ProgramConfig::IsCompute(context.fConfig->fKind)) {
                // Only compute shaders allow `workgroup`.
                if (!baseType->isOpaque() || baseType->isAtomic()) {
                    permitted |= ModifierFlag::kWorkgroup;
                }
            } else {
                // Only vertex/fragment shaders allow `flat` and `noperspective`.
                permitted |= ModifierFlag::kFlat | ModifierFlag::kNoPerspective;
            }
        }
    }

    LayoutFlags permittedLayoutFlags = LayoutFlag::kAll;

    // Pixel format modifiers are required on storage textures, and forbidden on other types.
    if (baseType->isStorageTexture()) {
        if (!(layout.fFlags & LayoutFlag::kAllPixelFormats)) {
            context.fErrors->error(pos, "storage textures must declare a pixel format");
        }
    } else {
        permittedLayoutFlags &= ~LayoutFlag::kAllPixelFormats;
    }

    // The `texture` and `sampler` modifiers can be present respectively on a texture and sampler or
    // simultaneously on a combined image-sampler but they are not permitted on any other type.
    switch (baseType->typeKind()) {
        case Type::TypeKind::kSampler:
            // Both texture and sampler flags are permitted
            break;
        case Type::TypeKind::kTexture:
            permittedLayoutFlags &= ~LayoutFlag::kSampler;
            break;
        case Type::TypeKind::kSeparateSampler:
            permittedLayoutFlags &= ~LayoutFlag::kTexture;
            break;
        default:
            permittedLayoutFlags &= ~(LayoutFlag::kTexture | LayoutFlag::kSampler);
            break;
    }

    // We don't allow 'binding' or 'set' on normal uniform variables, only on textures, samplers,
    // and interface blocks (holding uniform variables). They're also only allowed at global scope,
    // not on interface block fields (or locals/parameters).
    bool permitBindingAndSet = baseType->typeKind() == Type::TypeKind::kSampler ||
                               baseType->typeKind() == Type::TypeKind::kSeparateSampler ||
                               baseType->typeKind() == Type::TypeKind::kTexture ||
                               baseType->isInterfaceBlock();
    if (storage != Variable::Storage::kGlobal || (modifierFlags.isUniform() &&
                                                  !permitBindingAndSet)) {
        permittedLayoutFlags &= ~LayoutFlag::kBinding;
        permittedLayoutFlags &= ~LayoutFlag::kSet;
        permittedLayoutFlags &= ~LayoutFlag::kAllBackends;
    }
    if (ProgramConfig::IsRuntimeEffect(context.fConfig->fKind)) {
        // Disallow all layout flags except 'color' in runtime effects
        permittedLayoutFlags &= LayoutFlag::kColor;
    }

    // The `push_constant` flag isn't allowed on in-variables, out-variables, bindings or sets.
    if ((layout.fFlags & (LayoutFlag::kSet | LayoutFlag::kBinding)) ||
        (modifierFlags & (ModifierFlag::kIn | ModifierFlag::kOut))) {
        permittedLayoutFlags &= ~LayoutFlag::kPushConstant;
    }
    // The `builtin` layout flag is only allowed in modules.
    if (!context.fConfig->isBuiltinCode()) {
        permittedLayoutFlags &= ~LayoutFlag::kBuiltin;
    }

    modifierFlags.checkPermittedFlags(context, modifiersPosition, permitted);
    layout.checkPermittedLayout(context, modifiersPosition, permittedLayoutFlags);
}

bool VarDeclaration::ErrorCheckAndCoerce(const Context& context,
                                         const Variable& var,
                                         const Type* baseType,
                                         std::unique_ptr<Expression>& value) {
    if (baseType->matches(*context.fTypes.fInvalid)) {
        context.fErrors->error(var.fPosition, "invalid type");
        return false;
    }
    if (baseType->isVoid()) {
        context.fErrors->error(var.fPosition, "variables of type 'void' are not allowed");
        return false;
    }

    ErrorCheck(context, var.fPosition, var.modifiersPosition(), var.layout(), var.modifierFlags(),
               &var.type(), baseType, var.storage());
    if (value) {
        if (var.type().isOpaque() || var.type().isOrContainsAtomic()) {
            context.fErrors->error(value->fPosition, "opaque type '" + var.type().displayName() +
                                                     "' cannot use initializer expressions");
            return false;
        }
        if (var.modifierFlags() & ModifierFlag::kIn) {
            context.fErrors->error(value->fPosition,
                                   "'in' variables cannot use initializer expressions");
            return false;
        }
        if (var.modifierFlags().isUniform()) {
            context.fErrors->error(value->fPosition,
                                   "'uniform' variables cannot use initializer expressions");
            return false;
        }
        if (var.storage() == Variable::Storage::kInterfaceBlock) {
            context.fErrors->error(value->fPosition,
                                   "initializers are not permitted on interface block fields");
            return false;
        }
        if (context.fConfig->strictES2Mode() && var.type().isOrContainsArray()) {
            context.fErrors->error(value->fPosition, "initializers are not permitted on arrays "
                                                     "(or structs containing arrays)");
            return false;
        }
        value = var.type().coerceExpression(std::move(value), context);
        if (!value) {
            return false;
        }
    }
    if (var.modifierFlags().isConst()) {
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

std::unique_ptr<VarDeclaration> VarDeclaration::Convert(const Context& context,
                                                        Position overallPos,
                                                        const Modifiers& modifiers,
                                                        const Type& type,
                                                        Position namePos,
                                                        std::string_view name,
                                                        VariableStorage storage,
                                                        std::unique_ptr<Expression> value) {
    // Parameter declaration-statements do not exist in the grammar (unlike, say, K&R C).
    SkASSERT(storage != VariableStorage::kParameter);

    std::unique_ptr<Variable> var = Variable::Convert(context,
                                                      overallPos,
                                                      modifiers.fPosition,
                                                      modifiers.fLayout,
                                                      modifiers.fFlags,
                                                      &type,
                                                      namePos,
                                                      name,
                                                      storage);
    if (!var) {
        return nullptr;
    }
    return VarDeclaration::Convert(context, std::move(var), std::move(value));
}

std::unique_ptr<VarDeclaration> VarDeclaration::Convert(const Context& context,
                                                        std::unique_ptr<Variable> var,
                                                        std::unique_ptr<Expression> value) {
    const Type* baseType = &var->type();
    int arraySize = 0;
    if (baseType->isArray()) {
        arraySize = baseType->columns();
        baseType = &baseType->componentType();
    }
    if (!ErrorCheckAndCoerce(context, *var, baseType, value)) {
        return nullptr;
    }
    std::unique_ptr<VarDeclaration> varDecl = VarDeclaration::Make(context, var.get(), baseType,
                                                                   arraySize, std::move(value));
    if (!varDecl) {
        return nullptr;
    }

    if (var->storage() == Variable::Storage::kGlobal ||
        var->storage() == Variable::Storage::kInterfaceBlock) {
        // Check if this globally-scoped variable name overlaps an existing symbol name.
        if (context.fSymbolTable->find(var->name())) {
            context.fErrors->error(var->fPosition,
                                   "symbol '" + std::string(var->name()) + "' was already defined");
            return nullptr;
        }

        // `sk_RTAdjust` is special, and makes the IR generator emit position-fixup expressions.
        if (var->name() == Compiler::RTADJUST_NAME) {
            if (!var->type().matches(*context.fTypes.fFloat4)) {
                context.fErrors->error(var->fPosition, "sk_RTAdjust must have type 'float4'");
                return nullptr;
            }
        }
    }

    context.fSymbolTable->add(context, std::move(var));
    return varDecl;
}

std::unique_ptr<VarDeclaration> VarDeclaration::Make(const Context& context,
                                                     Variable* var,
                                                     const Type* baseType,
                                                     int arraySize,
                                                     std::unique_ptr<Expression> value) {
    SkASSERT(!baseType->isArray());
    // function parameters cannot have variable declarations
    SkASSERT(var->storage() != Variable::Storage::kParameter);
    // 'const' variables must be initialized
    SkASSERT(!var->modifierFlags().isConst() || value);
    // 'const' variable initializer must be a constant expression
    SkASSERT(!var->modifierFlags().isConst() || Analysis::IsConstantExpression(*value));
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
    SkASSERT(!(value && (var->modifierFlags() & ModifierFlag::kIn)));
    // 'uniform' variables cannot use initializer expressions
    SkASSERT(!(value && var->modifierFlags().isUniform()));
    // in strict-ES2 mode, is-or-contains-array types cannot use initializer expressions
    SkASSERT(!(value && var->type().isOrContainsArray() && context.fConfig->strictES2Mode()));

    auto result = std::make_unique<VarDeclaration>(var, baseType, arraySize, std::move(value));
    var->setVarDeclaration(result.get());
    return result;
}

}  // namespace SkSL
