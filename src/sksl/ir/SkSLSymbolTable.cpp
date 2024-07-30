/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLSymbolTable.h"

#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

std::unique_ptr<SymbolTable> SymbolTable::insertNewParent() {
    auto newTable = std::make_unique<SymbolTable>(fParent, fBuiltin);
    fParent = newTable.get();
    return newTable;
}

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

void SymbolTable::renameSymbol(const Context& context, Symbol* symbol, std::string_view newName) {
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

    this->addWithoutOwnership(context, symbol);
}

std::unique_ptr<Symbol> SymbolTable::removeSymbol(const Symbol* symbol) {
    // Remove the symbol from our symbol lookup table.
    if (fSymbols.removeIfExists(MakeSymbolKey(symbol->name()))) {
        // Transfer ownership of the symbol if we own it. (This will leave a nullptr behind in the
        // `fOwnedSymbols` list, which should be harmless.)
        for (std::unique_ptr<Symbol>& owned : fOwnedSymbols) {
            if (symbol == owned.get()) {
                return std::move(owned);
            }
        }
    }

    // We don't own the symbol after all.
    return nullptr;
}

void SymbolTable::moveSymbolTo(SymbolTable* otherTable, Symbol* sym, const Context& context) {
    if (std::unique_ptr<Symbol> ownedSymbol = this->removeSymbol(sym)) {
        otherTable->add(context, std::move(ownedSymbol));
    } else {
        otherTable->addWithoutOwnership(context, sym);
    }
}

const std::string* SymbolTable::takeOwnershipOfString(std::string str) {
    fOwnedStrings.push_front(std::move(str));
    // Because fOwnedStrings is a linked list, pointers to elements are stable.
    return &fOwnedStrings.front();
}

void SymbolTable::addWithoutOwnership(const Context& context, Symbol* symbol) {
    if (!this->addWithoutOwnership(symbol)) {
        context.fErrors->error(symbol->position(),
                               "symbol '" + std::string(symbol->name()) + "' was already defined");
    }
}

void SymbolTable::addWithoutOwnershipOrDie(Symbol* symbol) {
    if (!this->addWithoutOwnership(symbol)) {
        SK_ABORT("symbol '%.*s' was already defined",
                 (int)symbol->name().size(), symbol->name().data());
    }
}

bool SymbolTable::addWithoutOwnership(Symbol* symbol) {
    if (symbol->name().empty()) {
        // We have legitimate use cases of nameless symbols, such as anonymous function parameters.
        // If we find one here, we don't need to add its name to the symbol table.
        return true;
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
            return true;
        }
    }

    if (fAtModuleBoundary && fParent && fParent->lookup(key)) {
        // We are attempting to declare a symbol at global scope that already exists in a parent
        // module. This is a duplicate symbol and should be rejected.
        return false;
    }

    std::swap(symbol, fSymbols[key]);
    return symbol == nullptr;
}

void SymbolTable::injectWithoutOwnership(Symbol* symbol) {
    auto key = MakeSymbolKey(symbol->name());
    fSymbols[key] = symbol;
}

const Type* SymbolTable::addArrayDimension(const Context& context,
                                           const Type* type,
                                           int arraySize) {
    if (arraySize == 0) {
        return type;
    }
    // If we are making an array of a builtin type, we add it as high as possible in the symbol
    // table tree (at the module boundary), to enable additional reuse of the array-type.
    if (fParent && !fAtModuleBoundary && !context.fConfig->isBuiltinCode() && type->isBuiltin()) {
        return fParent->addArrayDimension(context, type, arraySize);
    }
    // Reuse an existing array type with this name if one already exists in our symbol table.
    std::string arrayName = type->getArrayName(arraySize);
    if (const Symbol* existingSymbol = this->find(arrayName)) {
        // We would expect an existing symbol named `Type[123]` to match our `Type[123]`. However,
        // we might be compiling invalid code that contains duplicate symbols, and so we need to
        // verify that these two types actually match before reusing the existing type.
        const Type* existingType = &existingSymbol->as<Type>();
        if (existingType->isArray() && type->matches(existingType->componentType())) {
            return existingType;
        }
    }
    // Add a new array type to the symbol table.
    const std::string* arrayNamePtr = this->takeOwnershipOfString(std::move(arrayName));
    return this->add(context, Type::MakeArrayType(context, *arrayNamePtr, *type, arraySize));
}

std::unique_ptr<Expression> SymbolTable::instantiateSymbolRef(const Context& context,
                                                              std::string_view name,
                                                              Position pos) {
    if (const Symbol* symbol = this->find(name)) {
        return symbol->instantiate(context, pos);
    }
    context.fErrors->error(pos, "unknown identifier '" + std::string(name) + "'");
    return nullptr;
}

}  // namespace SkSL
