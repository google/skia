/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSpan.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLString.h"
#include "src/sksl/ir/SkSLFieldSymbol.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLLayout.h"
#include "src/sksl/ir/SkSLModifierFlags.h"
#include "src/sksl/ir/SkSLModifiers.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>

using namespace skia_private;

namespace SkSL {

enum class ProgramKind : int8_t;

InterfaceBlock::~InterfaceBlock() {
    // Unhook this InterfaceBlock from its associated Variable, since we're being deleted.
    if (fVariable) {
        fVariable->detachDeadInterfaceBlock();
    }
}

static std::optional<int> find_rt_adjust_index(SkSpan<const Field> fields) {
    for (size_t index = 0; index < fields.size(); ++index) {
        const SkSL::Field& f = fields[index];
        if (f.fName == SkSL::Compiler::RTADJUST_NAME) {
            return index;
        }
    }

    return std::nullopt;
}

std::unique_ptr<InterfaceBlock> InterfaceBlock::Convert(const Context& context,
                                                        Position pos,
                                                        const Modifiers& modifiers,
                                                        std::string_view typeName,
                                                        TArray<Field> fields,
                                                        std::string_view varName,
                                                        int arraySize) {
    if (SkSL::ProgramKind kind = context.fConfig->fKind; !ProgramConfig::IsFragment(kind) &&
                                                         !ProgramConfig::IsVertex(kind) &&
                                                         !ProgramConfig::IsCompute(kind)) {
        context.fErrors->error(pos, "interface blocks are not allowed in this kind of program");
        return nullptr;
    }
    // Find sk_RTAdjust and error out if it's not of type `float4`.
    std::optional<int> rtAdjustIndex = find_rt_adjust_index(fields);
    if (rtAdjustIndex.has_value()) {
        const Field& rtAdjustField = fields[*rtAdjustIndex];
        if (!rtAdjustField.fType->matches(*context.fTypes.fFloat4)) {
            context.fErrors->error(rtAdjustField.fPosition, "sk_RTAdjust must have type 'float4'");
            return nullptr;
        }
    }
    // Build a struct type corresponding to the passed-in fields.
    const Type* baseType = context.fSymbolTable->add(context,
                                                     Type::MakeStructType(context,
                                                                          pos,
                                                                          typeName,
                                                                          std::move(fields),
                                                                          /*interfaceBlock=*/true));
    // Array-ify the type if necessary.
    const Type* type = baseType;
    if (arraySize > 0) {
        arraySize = type->convertArraySize(context, pos, pos, arraySize);
        if (!arraySize) {
            return nullptr;
        }
        type = context.fSymbolTable->addArrayDimension(context, type, arraySize);
    }

    // Error-check the interface block as if it were being declared as a global variable.
    VarDeclaration::ErrorCheck(context,
                               pos,
                               modifiers.fPosition,
                               modifiers.fLayout,
                               modifiers.fFlags,
                               type,
                               baseType,
                               VariableStorage::kGlobal);

    // Create a global variable for the Interface Block.
    std::unique_ptr<SkSL::Variable> var = SkSL::Variable::Convert(context,
                                                                  pos,
                                                                  modifiers.fPosition,
                                                                  modifiers.fLayout,
                                                                  modifiers.fFlags,
                                                                  type,
                                                                  pos,
                                                                  varName,
                                                                  VariableStorage::kGlobal);
    return InterfaceBlock::Make(context,
                                pos,
                                context.fSymbolTable->takeOwnershipOfSymbol(std::move(var)));
}

std::unique_ptr<InterfaceBlock> InterfaceBlock::Make(const Context& context,
                                                     Position pos,
                                                     Variable* variable) {
    SkASSERT(ProgramConfig::IsFragment(context.fConfig->fKind) ||
             ProgramConfig::IsVertex(context.fConfig->fKind) ||
             ProgramConfig::IsCompute(context.fConfig->fKind));

    SkASSERT(variable->type().componentType().isInterfaceBlock());
    SkSpan<const Field> fields = variable->type().componentType().fields();

    if (variable->name().empty()) {
        // This interface block is anonymous. Add each field to the top-level symbol table.
        for (size_t i = 0; i < fields.size(); ++i) {
            context.fSymbolTable->add(
                    context, std::make_unique<SkSL::FieldSymbol>(fields[i].fPosition, variable, i));
        }
    } else {
        // Add the global variable to the top-level symbol table.
        context.fSymbolTable->addWithoutOwnership(context, variable);
    }

    return std::make_unique<SkSL::InterfaceBlock>(pos, variable);
}

std::string InterfaceBlock::description() const {
    std::string result = this->var()->layout().description() +
                         this->var()->modifierFlags().description() + ' ' +
                         std::string(this->typeName()) + " {\n";
    const Type* structType = &this->var()->type();
    if (structType->isArray()) {
        structType = &structType->componentType();
    }
    for (const auto& f : structType->fields()) {
        result += f.description() + "\n";
    }
    result += "}";
    if (!this->instanceName().empty()) {
        result += " " + std::string(this->instanceName());
        if (this->arraySize() > 0) {
            String::appendf(&result, "[%d]", this->arraySize());
        }
    }
    return result + ";";
}

}  // namespace SkSL
