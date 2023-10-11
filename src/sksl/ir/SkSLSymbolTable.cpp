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

bool SymbolTable::isType(std::string_view name) const {
    const Symbol* symbol = this->find(name);
    return symbol && symbol->is<Type>();
}

bool SymbolTable::isBuiltinType(std::string_view name) const {
    if (!this->isBuiltin()) {
        return fParent && fParent->isBuiltinType(name);
    }
    return this->isType(name);
}

const Symbol* SymbolTable::findBuiltinSymbol(std::string_view name) const {
    if (!this->isBuiltin()) {
        return fParent ? fParent->findBuiltinSymbol(name) : nullptr;
    }
    return this->find(name);
}

bool SymbolTable::wouldShadowSymbolsFrom(const SymbolTable* other) const {
    // We are checking two hash maps for overlap; we always iterate over the smaller one to minimize
    // the total number of checks.
    const SymbolTable* self = this;
    if (self->count() > other->count()) {
        std::swap(self, other);
    }

    bool foundShadow = false;

    self->fSymbols.foreach([&](const SymbolKey& key, const Symbol* symbol) {
        if (foundShadow) {
            // We've already found a shadowed symbol; stop searching.
            return;
        }
        if (other->fSymbols.find(key) != nullptr) {
            foundShadow = true;
        }
    });

    return foundShadow;
}

Symbol* SymbolTable::lookup(const SymbolKey& key) const {
    Symbol** symbolPPtr = fSymbols.find(key);
    if (symbolPPtr) {
        return *symbolPPtr;
    }

    // The symbol wasn't found; recurse into the parent symbol table.
    return fParent ? fParent->lookup(key) : nullptr;
}

void SymbolTable::renameSymbol(Symbol* symbol, std::string_view newName) {
    if (symbol->is<FunctionDeclaration>()) {
        // This is a function declaration, so we need to rename the entire overload set.
        for (FunctionDeclaration* fn = &symbol->as<FunctionDeclaration>(); fn != nullptr;
             fn = fn->mutableNextOverload()) {
            fn->setName(newName);
        }
    } else {
        // Other types of symbols don't allow multiple symbols with the same name.
        symbol->setName(newName);
    }

    this->addWithoutOwnership(symbol);
}

const std::string* SymbolTable::takeOwnershipOfString(std::string str) {
    fOwnedStrings.push_front(std::move(str));
    // Because fOwnedStrings is a linked list, pointers to elements are stable.
    return &fOwnedStrings.front();
}

void SymbolTable::addWithoutOwnership(Symbol* symbol) {
    if (symbol->name().empty()) {
        // We have legitimate use cases of nameless symbols, such as anonymous function parameters.
        // If we find one here, we don't need to add its name to the symbol table.
        return;
    }
    auto key = MakeSymbolKey(symbol->name());

    // If this is a function declaration, we need to keep the overload chain in sync.
    if (symbol->is<FunctionDeclaration>()) {
        // If we have a function with the same name...
        Symbol* existingSymbol = this->lookup(key);
        if (existingSymbol && existingSymbol->is<FunctionDeclaration>()) {
            // ... add the existing function as the next overload in the chain.
            FunctionDeclaration* existingDecl = &existingSymbol->as<FunctionDeclaration>();
            symbol->as<FunctionDeclaration>().setNextOverload(existingDecl);
            fSymbols[key] = symbol;
            return;
        }
    }

    if (fAtModuleBoundary && fParent && fParent->lookup(key)) {
        // We are attempting to declare a symbol at global scope that already exists in a parent
        // module. This is a duplicate symbol and should be rejected.
    } else {
        Symbol*& refInSymbolTable = fSymbols[key];

        if (refInSymbolTable == nullptr) {
            refInSymbolTable = symbol;
            return;
        }
    }

    ThreadContext::ReportError("symbol '" + std::string(symbol->name()) + "' was already defined",
                               symbol->fPosition);
}

void SymbolTable::injectWithoutOwnership(Symbol* symbol) {
    auto key = MakeSymbolKey(symbol->name());
    fSymbols[key] = symbol;
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
    if (const Symbol* existingType = this->find(arrayName)) {
        return &existingType->as<Type>();
    }
    // Add a new array type to the symbol table.
    const std::string* arrayNamePtr = this->takeOwnershipOfString(std::move(arrayName));
    return this->add(Type::MakeArrayType(*arrayNamePtr, *type, arraySize));
}

}  // namespace SkSL
