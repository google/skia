/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLVariable.h"

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLMangler.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"

namespace SkSL {

Variable::~Variable() {
    // Unhook this Variable from its associated VarDeclaration, since we're being deleted.
    if (fDeclaration) {
        fDeclaration->setVar(nullptr);
    }
}

const Expression* Variable::initialValue() const {
    return fDeclaration ? fDeclaration->value().get() : nullptr;
}

std::unique_ptr<Variable> Variable::Convert(const Context& context, int line,
        const Modifiers& modifiers, const Type* baseType, skstd::string_view name, bool isArray,
        std::unique_ptr<Expression> arraySize, Variable::Storage storage) {
    if (modifiers.fLayout.fLocation == 0 && modifiers.fLayout.fIndex == 0 &&
        (modifiers.fFlags & Modifiers::kOut_Flag) &&
        context.fConfig->fKind == ProgramKind::kFragment && name != Compiler::FRAGCOLOR_NAME) {
        context.fErrors->error(line, "out location=0, index=0 is reserved for sk_FragColor");
    }
    if (!context.fConfig->fIsBuiltinCode && name.starts_with('$')) {
        context.fErrors->error(line, String("name '") + name + "' is reserved");
    }

    return Make(context, line, modifiers, baseType, name, isArray, std::move(arraySize), storage);
}

std::unique_ptr<Variable> Variable::Make(const Context& context, int line,
        const Modifiers& modifiers, const Type* baseType, skstd::string_view name, bool isArray,
        std::unique_ptr<Expression> arraySize, Variable::Storage storage) {
    const Type* type = baseType;
    int arraySizeValue = 0;
    if (isArray) {
        SkASSERT(arraySize);
        arraySizeValue = type->convertArraySize(context, std::move(arraySize));
        if (!arraySizeValue) {
            return nullptr;
        }
        type = ThreadContext::SymbolTable()->addArrayDimension(type, arraySizeValue);
    }
    return std::make_unique<Variable>(line, context.fModifiersPool->add(modifiers), name, type,
            context.fConfig->fIsBuiltinCode, storage);
}

Variable::ScratchVariable Variable::MakeScratchVariable(const Context& context,
                                                        skstd::string_view baseName,
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
    const String* name =
            symbolTable->takeOwnershipOfString(context.fMangler->uniqueName(baseName, symbolTable));

    // Create our new variable and add it to the symbol table.
    ScratchVariable result;
    auto var = std::make_unique<Variable>(initialValue ? initialValue->fLine : -1,
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
