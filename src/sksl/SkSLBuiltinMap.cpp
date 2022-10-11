/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "include/private/SkSLProgramElement.h"
#include "include/private/SkSLStatement.h"
#include "src/sksl/SkSLBuiltinMap.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"

#include <string>
#include <utility>

namespace SkSL {

class Symbol;

BuiltinMap::BuiltinMap(const BuiltinMap* parent, SkSpan<std::unique_ptr<ProgramElement>> elements)
        : fParent(parent) {
    // Transfer all of the program elements from the vector to this builtin element map. This maps
    // global symbols to the declaring ProgramElement.
    for (std::unique_ptr<ProgramElement>& element : elements) {
        switch (element->kind()) {
            case ProgramElement::Kind::kFunction: {
                const FunctionDeclaration& decl = element->as<FunctionDefinition>().declaration();
                SkASSERT(decl.isBuiltin());
                this->insertOrDie(&decl, std::move(element));
                break;
            }
            case ProgramElement::Kind::kFunctionPrototype: {
                // These are already in the symbol table.
                break;
            }
            case ProgramElement::Kind::kGlobalVar: {
                const GlobalVarDeclaration& global = element->as<GlobalVarDeclaration>();
                const Variable& var = global.declaration()->as<VarDeclaration>().var();
                SkASSERT(var.isBuiltin());
                this->insertOrDie(&var, std::move(element));
                break;
            }
            case ProgramElement::Kind::kInterfaceBlock: {
                const Variable& var = element->as<InterfaceBlock>().variable();
                SkASSERT(var.isBuiltin());
                this->insertOrDie(&var, std::move(element));
                break;
            }
            default:
                SkDEBUGFAILF("Unsupported element: %s\n", element->description().c_str());
                break;
        }
    }
}

void BuiltinMap::insertOrDie(const Symbol* symbol, std::unique_ptr<ProgramElement> element) {
    SkASSERT(!fElements.find(symbol));
    fElements.set(symbol, std::move(element));
}

const ProgramElement* BuiltinMap::find(const Symbol* symbol) const {
    if (std::unique_ptr<ProgramElement>* elem = fElements.find(symbol)) {
        return elem->get();
    }
    return fParent ? fParent->find(symbol) : nullptr;
}

void BuiltinMap::foreach(const std::function<void(const Symbol*,const ProgramElement&)>& fn) const {
    fElements.foreach([&](const Symbol* symbol, const std::unique_ptr<ProgramElement>& elem) {
        fn(symbol, *elem);
    });
    if (fParent) {
        fParent->foreach(fn);
    }
}

}  // namespace SkSL
