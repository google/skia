/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLVariable.h"

#include "src/base/SkEnumBitMask.h"
#include "src/base/SkStringView.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLIntrinsicList.h"
#include "src/sksl/SkSLMangler.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLLayout.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"

#include <utility>

namespace SkSL {

static constexpr Layout kDefaultLayout;

Variable::~Variable() {
    // Unhook this Variable from its associated VarDeclaration, since we're being deleted.
    if (VarDeclaration* declaration = this->varDeclaration()) {
        declaration->detachDeadVariable();
    }
}

ExtendedVariable::~ExtendedVariable() {
    // Unhook this Variable from its associated InterfaceBlock, since we're being deleted.
    if (fInterfaceBlockElement) {
        fInterfaceBlockElement->detachDeadVariable();
    }
}

const Expression* Variable::initialValue() const {
    VarDeclaration* declaration = this->varDeclaration();
    return declaration ? declaration->value().get() : nullptr;
}

VarDeclaration* Variable::varDeclaration() const {
    if (!fDeclaringElement) {
        return nullptr;
    }
    SkASSERT(fDeclaringElement->is<VarDeclaration>() ||
             fDeclaringElement->is<GlobalVarDeclaration>());
    return fDeclaringElement->is<GlobalVarDeclaration>()
               ? &fDeclaringElement->as<GlobalVarDeclaration>().varDeclaration()
               : &fDeclaringElement->as<VarDeclaration>();
}

GlobalVarDeclaration* Variable::globalVarDeclaration() const {
    if (!fDeclaringElement) {
        return nullptr;
    }
    SkASSERT(fDeclaringElement->is<VarDeclaration>() ||
             fDeclaringElement->is<GlobalVarDeclaration>());
    return fDeclaringElement->is<GlobalVarDeclaration>()
               ? &fDeclaringElement->as<GlobalVarDeclaration>()
               : nullptr;
}

void Variable::setVarDeclaration(VarDeclaration* declaration) {
    SkASSERT(!fDeclaringElement || this == declaration->var());
    if (!fDeclaringElement) {
        fDeclaringElement = declaration;
    }
}

void Variable::setGlobalVarDeclaration(GlobalVarDeclaration* global) {
    SkASSERT(!fDeclaringElement || this == global->varDeclaration().var());
    fDeclaringElement = global;
}

const Layout& Variable::layout() const {
    return kDefaultLayout;
}

std::string_view ExtendedVariable::mangledName() const {
    return fMangledName.empty() ? this->name() : fMangledName;
}

std::unique_ptr<Variable> Variable::Convert(const Context& context,
                                            Position pos,
                                            Position modifiersPos,
                                            const Layout& layout,
                                            ModifierFlags flags,
                                            const Type* type,
                                            Position namePos,
                                            std::string_view name,
                                            Storage storage) {
    if (layout.fLocation == 0 &&
        layout.fIndex == 0 &&
        (flags & ModifierFlag::kOut) &&
        ProgramConfig::IsFragment(context.fConfig->fKind) &&
        name != Compiler::FRAGCOLOR_NAME) {
        context.fErrors->error(modifiersPos,
                               "out location=0, index=0 is reserved for sk_FragColor");
    }
    if (type->isUnsizedArray() && storage != Variable::Storage::kInterfaceBlock) {
        context.fErrors->error(pos, "unsized arrays are not permitted here");
    }
    if (ProgramConfig::IsCompute(context.fConfig->fKind) && layout.fBuiltin == -1) {
        if (storage == Variable::Storage::kGlobal) {
            if (flags & ModifierFlag::kIn) {
                context.fErrors->error(pos, "pipeline inputs not permitted in compute shaders");
            } else if (flags & ModifierFlag::kOut) {
                context.fErrors->error(pos, "pipeline outputs not permitted in compute shaders");
            }
        }
    }
    if (storage == Variable::Storage::kParameter) {
        // The `in` modifier on function parameters is implicit, so we can replace `in float x` with
        // `float x`. This prevents any ambiguity when matching a function by its param types.
        if ((flags & (ModifierFlag::kOut | ModifierFlag::kIn)) == ModifierFlag::kIn) {
            flags &= ~(ModifierFlag::kOut | ModifierFlag::kIn);
        }
    }

    // Invent a mangled name for the variable, if it needs one.
    std::string mangledName;
    if (skstd::starts_with(name, '$')) {
        // The $ prefix will fail to compile in GLSL, so replace it with `sk_Priv`.
        mangledName = "sk_Priv" + std::string(name.substr(1));
    } else if (FindIntrinsicKind(name) != kNotIntrinsic) {
        // Having a variable name overlap an intrinsic name will prevent us from calling the
        // intrinsic, but it's not illegal for user names to shadow a global symbol.
        // Mangle the name to avoid a possible collision.
        mangledName = Mangler{}.uniqueName(name, context.fSymbolTable.get());
    }

    return Make(pos, modifiersPos, layout, flags, type, name, std::move(mangledName),
                context.fConfig->fIsBuiltinCode, storage);
}

std::unique_ptr<Variable> Variable::Make(Position pos,
                                         Position modifiersPosition,
                                         const Layout& layout,
                                         ModifierFlags flags,
                                         const Type* type,
                                         std::string_view name,
                                         std::string mangledName,
                                         bool builtin,
                                         Variable::Storage storage) {
    // the `in` modifier on function parameters is implicit and should have been removed
    SkASSERT(!(storage == Variable::Storage::kParameter &&
               (flags & (ModifierFlag::kOut | ModifierFlag::kIn)) == ModifierFlag::kIn));

    if (type->componentType().isInterfaceBlock() || !mangledName.empty() ||
        layout != kDefaultLayout) {
        return std::make_unique<ExtendedVariable>(pos,
                                                  modifiersPosition,
                                                  layout,
                                                  flags,
                                                  name,
                                                  type,
                                                  builtin,
                                                  storage,
                                                  std::move(mangledName));
    } else {
        return std::make_unique<Variable>(pos,
                                          modifiersPosition,
                                          flags,
                                          name,
                                          type,
                                          builtin,
                                          storage);
    }
}

Variable::ScratchVariable Variable::MakeScratchVariable(const Context& context,
                                                        Mangler& mangler,
                                                        std::string_view baseName,
                                                        const Type* type,
                                                        SymbolTable* symbolTable,
                                                        std::unique_ptr<Expression> initialValue) {
    // $floatLiteral or $intLiteral aren't real types that we can use for scratch variables, so
    // replace them if they ever appear here. If this happens, we likely forgot to coerce a type
    // somewhere during compilation.
    if (type->isLiteral()) {
        SkDEBUGFAIL("found a $literal type in MakeScratchVariable");
        type = &type->scalarTypeForLiteral();
    }

    // Provide our new variable with a unique name, and add it to our symbol table.
    const std::string* name =
            symbolTable->takeOwnershipOfString(mangler.uniqueName(baseName, symbolTable));

    // Create our new variable and add it to the symbol table.
    ScratchVariable result;
    auto var = std::make_unique<Variable>(initialValue ? initialValue->fPosition : Position(),
                                          /*modifiersPosition=*/Position(),
                                          ModifierFlag::kNone,
                                          name->c_str(),
                                          type,
                                          symbolTable->isBuiltin(),
                                          Variable::Storage::kLocal);

    // If we are creating an array type, reduce it to base type plus array-size.
    int arraySize = 0;
    if (type->isArray()) {
        arraySize = type->columns();
        type = &type->componentType();
    }
    // Create our variable declaration.
    result.fVarDecl = VarDeclaration::Make(context, var.get(), type, arraySize,
                                           std::move(initialValue));
    result.fVarSymbol = symbolTable->add(std::move(var));
    return result;
}

} // namespace SkSL
