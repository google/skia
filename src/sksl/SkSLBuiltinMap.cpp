/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "include/private/SkSLProgramElement.h"
#include "src/sksl/SkSLBuiltinMap.h"

#include <string>
#include <utility>

namespace SkSL {

class SymbolTable;

BuiltinMap::BuiltinMap(const BuiltinMap* parent,
                       std::shared_ptr<SymbolTable> symbolTable,
                       SkSpan<std::unique_ptr<ProgramElement>> elements)
        : fParent(parent)
        , fSymbolTable(symbolTable) {
    for (std::unique_ptr<ProgramElement>& element : elements) {
        switch (element->kind()) {
            case ProgramElement::Kind::kFunction:
            case ProgramElement::Kind::kGlobalVar:
            case ProgramElement::Kind::kInterfaceBlock:
                // We don't look these up from the BuiltinMap anymore, but we can't delete them.
                fUnmappedElements.push_back(std::move(element));
                break;

            case ProgramElement::Kind::kFunctionPrototype:
                // These are already in the symbol table.
                break;

            default:
                SkDEBUGFAILF("Unsupported element: %s\n", element->description().c_str());
                break;
        }
    }
}

void BuiltinMap::foreach(const std::function<void(const ProgramElement&)>& fn) const {
    for (const std::unique_ptr<ProgramElement>& elem : fUnmappedElements) {
        fn(*elem);
    }
    if (fParent) {
        fParent->foreach(fn);
    }
}

}  // namespace SkSL
