/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkSLModifiers.h"
#include "include/private/SkSLString.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLSymbolTable.h"

#include <vector>

namespace SkSL {

InterfaceBlock::~InterfaceBlock() {
    // Unhook this InterfaceBlock from its associated Variable, since we're being deleted.
    if (fVariable) {
        fVariable->detachDeadInterfaceBlock();
    }
}

std::unique_ptr<ProgramElement> InterfaceBlock::clone() const {
    return std::make_unique<InterfaceBlock>(fPosition,
                                            this->var(),
                                            this->typeName(),
                                            this->instanceName(),
                                            this->arraySize(),
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
