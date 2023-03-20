/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLVariable.h"

#include "include/private/SkSLIRNode.h"
#include "include/private/SkSLLayout.h"
#include "include/sksl/SkSLErrorReporter.h"
#include "src/base/SkStringView.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLMangler.h"
#include "src/sksl/SkSLModifiersPool.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"

#include <type_traits>
#include <utility>

namespace SkSL {

Variable::~Variable() {
    // Unhook this Variable from its associated VarDeclaration, since we're being deleted.
    if (VarDeclaration* declaration = this->varDeclaration()) {
        declaration->detachDeadVariable();
    }
}

InterfaceBlockVariable::~InterfaceBlockVariable() {
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

std::string Variable::mangledName() const {
    // Only private variables need to use name mangling.
    std::string_view name = this->name();
    if (!skstd::starts_with(name, '$')) {
        return std::string(name);
    }

    // The $ prefix will fail to compile in GLSL, so replace it with `sk_Priv`.
    name.remove_prefix(1);
    return "sk_Priv" + std::string(name);
}

std::unique_ptr<Variable> Variable::Convert(const Context& context,
                                            Position pos,
                                            Position modifiersPos,
                                            const Modifiers& modifiers,
                                            const Type* baseType,
                                            Position namePos,
                                            std::string_view name,
                                            bool isArray,
                                            std::unique_ptr<Expression> arraySize,
                                            Variable::Storage storage) {
    if (modifiers.fLayout.fLocation == 0 && modifiers.fLayout.fIndex == 0 &&
        (modifiers.fFlags & Modifiers::kOut_Flag) &&
        ProgramConfig::IsFragment(context.fConfig->fKind) && name != Compiler::FRAGCOLOR_NAME) {
        context.fErrors->error(modifiersPos,
                               "out location=0, index=0 is reserved for sk_FragColor");
    }
    if (baseType->isUnsizedArray() && storage != Variable::Storage::kInterfaceBlock) {
        context.fErrors->error(pos, "unsized arrays are not permitted here");
    }
    if (ProgramConfig::IsCompute(ThreadContext::Context().fConfig->fKind) &&
            modifiers.fLayout.fBuiltin == -1) {
        if (storage == Variable::Storage::kGlobal) {
            if (modifiers.fFlags & Modifiers::kIn_Flag) {
                context.fErrors->error(pos, "pipeline inputs not permitted in compute shaders");
            } else if (modifiers.fFlags & Modifiers::kOut_Flag) {
                context.fErrors->error(pos, "pipeline outputs not permitted in compute shaders");
            }
        }
    }

    return Make(context, pos, modifiersPos, modifiers, baseType, name, isArray,
                std::move(arraySize), storage);
}

std::unique_ptr<Variable> Variable::Make(const Context& context,
                                         Position pos,
                                         Position modifiersPos,
                                         const Modifiers& modifiers,
                                         const Type* baseType,
                                         std::string_view name,
                                         bool isArray,
                                         std::unique_ptr<Expression> arraySize,
                                         Variable::Storage storage) {
    const Type* type = baseType;
    int arraySizeValue = 0;
    if (isArray) {
        SkASSERT(arraySize);
        arraySizeValue = type->convertArraySize(context, pos, std::move(arraySize));
        if (!arraySizeValue) {
            return nullptr;
        }
        type = ThreadContext::SymbolTable()->addArrayDimension(type, arraySizeValue);
    }
    if (type->componentType().isInterfaceBlock()) {
        return std::make_unique<InterfaceBlockVariable>(pos,
                                                        modifiersPos,
                                                        context.fModifiersPool->add(modifiers),
                                                        name,
                                                        type,
                                                        context.fConfig->fIsBuiltinCode,
                                                        storage);
    } else {
        return std::make_unique<Variable>(pos,
                                          modifiersPos,
                                          context.fModifiersPool->add(modifiers),
                                          name,
                                          type,
                                          context.fConfig->fIsBuiltinCode,
                                          storage);
    }
}

Variable::ScratchVariable Variable::MakeScratchVariable(const Context& context,
                                                        Mangler& mangler,
                                                        std::string_view baseName,
                                                        const Type* type,
                                                        const Modifiers& modifiers,
                                                        SymbolTable* symbolTable,
                                                        std::unique_ptr<Expression> initialValue) {
    // $floatLiteral or $intLiteral aren't real types that we can use for scratch variables, so
    // replace them if they ever appear here. If this happens, we likely forgot to coerce a type
    // somewhere during compilation.
    if (type->isLiteral()) {
        SkDEBUGFAIL("found a $literal type in MakeScratchVariable");
        type = &type->scalarTypeForLiteral();
    }

    // Out-parameters aren't supported.
    SkASSERT(!(modifiers.fFlags & Modifiers::kOut_Flag));

    // Provide our new variable with a unique name, and add it to our symbol table.
    const std::string* name =
            symbolTable->takeOwnershipOfString(mangler.uniqueName(baseName, symbolTable));

    // Create our new variable and add it to the symbol table.
    ScratchVariable result;
    auto var = std::make_unique<Variable>(initialValue ? initialValue->fPosition : Position(),
                                          /*modifiersPosition=*/Position(),
                                          context.fModifiersPool->add(Modifiers{}),
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
