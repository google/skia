/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkSLModifiers.h"
#include "include/private/SkSLProgramElement.h"
#include "include/private/SkSLString.h"
#include "include/private/SkSLSymbol.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLModifiersPool.h"
#include "src/sksl/SkSLPool.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/analysis/SkSLProgramUsage.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLSymbolTable.h" // IWYU pragma: keep
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"

#include <type_traits>
#include <utility>

namespace SkSL {

Program::Program(std::unique_ptr<std::string> source,
                 std::unique_ptr<ProgramConfig> config,
                 std::shared_ptr<Context> context,
                 std::vector<std::unique_ptr<ProgramElement>> elements,
                 std::vector<const ProgramElement*> sharedElements,
                 std::unique_ptr<ModifiersPool> modifiers,
                 std::shared_ptr<SymbolTable> symbols,
                 std::unique_ptr<Pool> pool,
                 Inputs inputs)
        : fSource(std::move(source))
        , fConfig(std::move(config))
        , fContext(context)
        , fModifiers(std::move(modifiers))
        , fSymbols(symbols)
        , fPool(std::move(pool))
        , fOwnedElements(std::move(elements))
        , fSharedElements(std::move(sharedElements))
        , fInputs(inputs) {
    fUsage = Analysis::GetUsage(*this);
}

Program::~Program() {
    // Some or all of the program elements are in the pool. To free them safely, we must attach
    // the pool before destroying any program elements. (Otherwise, we may accidentally call
    // delete on a pooled node.)
    AutoAttachPoolToThread attach(fPool.get());

    fOwnedElements.clear();
    fContext.reset();
    fSymbols.reset();
    fModifiers.reset();
}

std::string Program::description() const {
    std::string result = fConfig->versionDescription();
    for (const ProgramElement* e : this->elements()) {
        result += e->description();
    }
    return result;
}

const FunctionDeclaration* Program::getFunction(const char* functionName) const {
    const Symbol* symbol = fSymbols->find(functionName);
    bool valid = symbol && symbol->is<FunctionDeclaration>() &&
                 symbol->as<FunctionDeclaration>().definition();
    return valid ? &symbol->as<FunctionDeclaration>() : nullptr;
}

static void gather_uniforms(UniformInfo* info, const Type& type, const std::string& name) {
    switch (type.typeKind()) {
        case Type::TypeKind::kStruct:
            for (const auto& f : type.fields()) {
                gather_uniforms(info, *f.fType, name + "." + std::string(f.fName));
            }
            break;
        case Type::TypeKind::kArray:
            for (int i = 0; i < type.columns(); ++i) {
                gather_uniforms(info, type.componentType(),
                                String::printf("%s[%d]", name.c_str(), i));
            }
            break;
        case Type::TypeKind::kScalar:
        case Type::TypeKind::kVector:
        case Type::TypeKind::kMatrix:
            info->fUniforms.push_back({name, type.componentType().numberKind(),
                                       type.rows(), type.columns(), info->fUniformSlotCount});
            info->fUniformSlotCount += type.slotCount();
            break;
        default:
            break;
    }
}

std::unique_ptr<UniformInfo> Program::getUniformInfo() {
    auto info = std::make_unique<UniformInfo>();
    for (const ProgramElement* e : this->elements()) {
        if (!e->is<GlobalVarDeclaration>()) {
            continue;
        }
        const GlobalVarDeclaration& decl = e->as<GlobalVarDeclaration>();
        const Variable& var = *decl.varDeclaration().var();
        if (var.modifiers().fFlags & Modifiers::kUniform_Flag) {
            gather_uniforms(info.get(), var.type(), std::string(var.name()));
        }
    }
    return info;
}

}  // namespace SkSL
