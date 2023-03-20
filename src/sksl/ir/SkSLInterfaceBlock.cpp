/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSpan.h"
#include "include/private/SkSLModifiers.h"
#include "include/private/SkSLString.h"
#include "include/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/ir/SkSLField.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLSymbolTable.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace SkSL {

enum class ProgramKind : int8_t;

InterfaceBlock::~InterfaceBlock() {
    // Unhook this InterfaceBlock from its associated Variable, since we're being deleted.
    if (fVariable) {
        fVariable->detachDeadInterfaceBlock();
    }
}

static std::optional<int> find_rt_adjust_index(SkSpan<const Type::Field> fields) {
    for (size_t index = 0; index < fields.size(); ++index) {
        const SkSL::Type::Field& f = fields[index];
        if (f.fName == SkSL::Compiler::RTADJUST_NAME) {
            return index;
        }
    }

    return std::nullopt;
}

std::unique_ptr<InterfaceBlock> InterfaceBlock::Convert(const Context& context,
                                                        Position pos,
                                                        Variable* variable,
                                                        std::shared_ptr<SymbolTable> symbols) {
    if (SkSL::ProgramKind kind = context.fConfig->fKind; !ProgramConfig::IsFragment(kind) &&
                                                         !ProgramConfig::IsVertex(kind) &&
                                                         !ProgramConfig::IsCompute(kind)) {
        context.fErrors->error(pos, "interface blocks are not allowed in this kind of program");
        return nullptr;
    }

    // Find sk_RTAdjust and error out if it's not of type `float4`.
    SkSpan<const Type::Field> fields = variable->type().componentType().fields();
    std::optional<int> rtAdjustIndex = find_rt_adjust_index(fields);
    if (rtAdjustIndex.has_value()) {
        const Type::Field& rtAdjustField = fields[*rtAdjustIndex];
        if (!rtAdjustField.fType->matches(*context.fTypes.fFloat4)) {
            context.fErrors->error(rtAdjustField.fPosition, "sk_RTAdjust must have type 'float4'");
            return nullptr;
        }
    }
    return InterfaceBlock::Make(context, pos, variable, rtAdjustIndex, symbols);
}

std::unique_ptr<InterfaceBlock> InterfaceBlock::Make(const Context& context,
                                                     Position pos,
                                                     Variable* variable,
                                                     std::optional<int> rtAdjustIndex,
                                                     std::shared_ptr<SymbolTable> symbols) {
    SkASSERT(ProgramConfig::IsFragment(context.fConfig->fKind) ||
             ProgramConfig::IsVertex(context.fConfig->fKind) ||
             ProgramConfig::IsCompute(context.fConfig->fKind));

    SkASSERT(variable->type().componentType().isInterfaceBlock());
    SkSpan<const Type::Field> fields = variable->type().componentType().fields();

    if (rtAdjustIndex.has_value()) {
        [[maybe_unused]] const Type::Field& rtAdjustField = fields[*rtAdjustIndex];
        SkASSERT(rtAdjustField.fName == SkSL::Compiler::RTADJUST_NAME);
        SkASSERT(rtAdjustField.fType->matches(*context.fTypes.fFloat4));

        ThreadContext::RTAdjustData& rtAdjustData = ThreadContext::RTAdjustState();
        rtAdjustData.fInterfaceBlock = variable;
        rtAdjustData.fFieldIndex = *rtAdjustIndex;
    }

    if (variable->name().empty()) {
        // This interface block is anonymous. Add each field to the top-level symbol table.
        for (size_t i = 0; i < fields.size(); ++i) {
            symbols->add(std::make_unique<SkSL::Field>(fields[i].fPosition, variable, i));
        }
    } else {
        // Add the global variable to the top-level symbol table.
        symbols->addWithoutOwnership(variable);
    }

    return std::make_unique<SkSL::InterfaceBlock>(pos, variable, symbols);
}

std::unique_ptr<ProgramElement> InterfaceBlock::clone() const {
    return std::make_unique<InterfaceBlock>(fPosition,
                                            this->var(),
                                            SymbolTable::WrapIfBuiltin(this->typeOwner()));
}

std::string InterfaceBlock::description() const {
    std::string result = this->var()->modifiers().description() +
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
