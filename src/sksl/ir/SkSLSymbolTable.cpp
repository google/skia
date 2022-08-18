/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLSymbolTable.h"

#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

const Symbol* SymbolTable::operator[](std::string_view name) const {
    return this->lookup(MakeSymbolKey(name));
}

bool SymbolTable::isType(std::string_view name) const {
    const Symbol* symbol = (*this)[name];
    return symbol && symbol->is<Type>();
}

bool SymbolTable::isBuiltinType(std::string_view name) const {
    if (!this->isBuiltin()) {
        return fParent && fParent->isBuiltinType(name);
    }
    return this->isType(name);
}

const Symbol* SymbolTable::lookup(const SymbolKey& key) const {
    const Symbol** symbolPPtr = fSymbols.find(key);
    if (symbolPPtr) {
        return *symbolPPtr;
    }

    // The symbol wasn't found; recurse into the parent symbol table.
    return fParent ? fParent->lookup(key) : nullptr;
}

const std::string* SymbolTable::takeOwnershipOfString(std::string str) {
    fOwnedStrings.push_front(std::move(str));
    // Because fOwnedStrings is a linked list, pointers to elements are stable.
    return &fOwnedStrings.front();
}

void SymbolTable::addWithoutOwnership(Symbol* symbol) {
    auto key = MakeSymbolKey(symbol->name());

    // If this is a function declaration, we need to keep the overload chain in sync.
    if (symbol->is<FunctionDeclaration>()) {
        // If we have a function with the same name...
        const Symbol* existingSymbol = this->lookup(key);
        if (existingSymbol && existingSymbol->is<FunctionDeclaration>()) {
            // ... add the existing function as the next overload in the chain.
            const FunctionDeclaration* existingDecl = &existingSymbol->as<FunctionDeclaration>();
            symbol->as<FunctionDeclaration>().setNextOverload(existingDecl);
            fSymbols[key] = symbol;
            return;
        }
    }

    return this->addWithoutOwnership(symbol, key);
}

void SymbolTable::addWithoutOwnership(const Symbol* symbol, const SymbolKey& key) {
    const Symbol*& refInSymbolTable = fSymbols[key];

    if (refInSymbolTable == nullptr) {
        refInSymbolTable = symbol;
        return;
    }

    ThreadContext::ReportError("symbol '" + std::string(symbol->name()) + "' was already defined",
                               symbol->fPosition);
}

const Type* SymbolTable::addArrayDimension(const Type* type, int arraySize) {
    if (arraySize == 0) {
        return type;
    }
    // If this is a builtin type, we add it as high as possible in the symbol table tree (at the
    // module boundary), to enable additional reuse of the array-type.
    if (type->isInBuiltinTypes() && fParent && !fAtModuleBoundary) {
        return fParent->addArrayDimension(type, arraySize);
    }
    // Reuse an existing array type with this name if one already exists in our symbol table.
    std::string arrayName = type->getArrayName(arraySize);
    if (const Symbol* existingType = (*this)[arrayName]) {
        return &existingType->as<Type>();
    }
    // Add a new array type to the symbol table.
    const std::string* arrayNamePtr = this->takeOwnershipOfString(std::move(arrayName));
    return this->add(Type::MakeArrayType(*arrayNamePtr, *type, arraySize));
}

}  // namespace SkSL
