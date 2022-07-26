/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLSymbolTable.h"

#include "include/core/SkSpan.h"
#include "include/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLUnresolvedFunction.h"

namespace SkSL {

static SkSpan<const FunctionDeclaration* const> get_overload_set(
        const Symbol& s,
        const FunctionDeclaration*& scratchPtr) {
    switch (s.kind()) {
        case Symbol::Kind::kFunctionDeclaration:
            scratchPtr = &s.as<FunctionDeclaration>();
            return SkSpan(&scratchPtr, 1);

        case Symbol::Kind::kUnresolvedFunction:
            return SkSpan(s.as<UnresolvedFunction>().functions());

        default:
            return SkSpan<const FunctionDeclaration* const>{};
    }
}

const Symbol* SymbolTable::operator[](std::string_view name) {
    return this->lookup(this, /*encounteredModuleBoundary=*/false, MakeSymbolKey(name));
}

const Symbol* SymbolTable::lookup(SymbolTable* writableSymbolTable,
                                  bool encounteredModuleBoundary,
                                  const SymbolKey& key) {
    // Symbol-table lookup can cause new UnresolvedFunction nodes to be created; however, we don't
    // want these to end up in the symbol tables of loaded modules (where they will outlive the
    // Program associated with those UnresolvedFunction nodes). `writableSymbolTable` tracks the
    // closest symbol table to the root which is not a built-in. `encounteredModuleBoundary` is set
    // to true whenever we detect a module boundary; once this happens, we stop updating the
    // `writableSymbolTable` to prevent making changes outside of our current Program.
    if (!encounteredModuleBoundary) {
        writableSymbolTable = this;
        encounteredModuleBoundary |= fAtModuleBoundary;
    }
    const Symbol** symbolPPtr = fSymbols.find(key);
    if (!symbolPPtr) {
        // The symbol wasn't found; recurse into the parent symbol table.
        return fParent ? fParent->lookup(writableSymbolTable, encounteredModuleBoundary, key)
                       : nullptr;
    }

    const Symbol* symbol = *symbolPPtr;
    if (!fParent) {
        // We found a symbol at the top level.
        return symbol;
    }

    const FunctionDeclaration* scratchPtr;
    SkSpan<const FunctionDeclaration* const> overloadSet = get_overload_set(*symbol, scratchPtr);
    if (overloadSet.empty()) {
        // We found a non-function-related symbol. Return it.
        return symbol;
    }

    // We found a function-related symbol. We need to return the complete overload set.
    return this->buildOverloadSet(writableSymbolTable, encounteredModuleBoundary,
                                  key, symbol, overloadSet);
}

const Symbol* SymbolTable::buildOverloadSet(SymbolTable* writableSymbolTable,
                                            bool encounteredModuleBoundary,
                                            const SymbolKey& key,
                                            const Symbol* symbol,
                                            SkSpan<const FunctionDeclaration* const> overloadSet) {
    // Scan the parent symbol table for a matching symbol.
    const Symbol* overloadedSymbol = fParent->lookup(writableSymbolTable, encounteredModuleBoundary,
                                                     key);
    if (!overloadedSymbol) {
        return symbol;
    }

    const FunctionDeclaration* scratchPtr;
    SkSpan<const FunctionDeclaration* const> parentOverloadSet = get_overload_set(*overloadedSymbol,
                                                                                  scratchPtr);
    if (parentOverloadSet.empty()) {
        // The parent symbol wasn't function-related. Return the symbol we found.
        return symbol;
    }

    // We've found two distinct overload sets; we need to combine them. Start with the initial set.
    std::vector<const FunctionDeclaration*> combinedOverloadSet{overloadSet.begin(),
                                                                overloadSet.end()};
    // Now combine in any unique overloads from the parent overload set.
    for (const FunctionDeclaration* prev : parentOverloadSet) {
        bool matchFound = false;
        for (const FunctionDeclaration* current : combinedOverloadSet) {
            if (current->matches(*prev)) {
                matchFound = true;
                break;
            }
        }
        if (!matchFound) {
            combinedOverloadSet.push_back(prev);
        }
    }

    SkASSERT(combinedOverloadSet.size() >= overloadSet.size());
    if (combinedOverloadSet.size() == overloadSet.size()) {
        // We didn't add anything to the overload set. Our existing symbol is fine.
        return symbol;
    }

    // Add this combined overload set to the symbol table.
    SkASSERT(combinedOverloadSet.size() > 1);
    SkASSERT(writableSymbolTable);
    return writableSymbolTable->takeOwnershipOfSymbol(
            std::make_unique<UnresolvedFunction>(std::move(combinedOverloadSet)));
}

const std::string* SymbolTable::takeOwnershipOfString(std::string str) {
    fOwnedStrings.push_front(std::move(str));
    // Because fOwnedStrings is a linked list, pointers to elements are stable.
    return &fOwnedStrings.front();
}

void SymbolTable::addWithoutOwnership(const Symbol* symbol) {
    const std::string_view& name = symbol->name();

    const Symbol*& refInSymbolTable = fSymbols[MakeSymbolKey(name)];
    if (refInSymbolTable == nullptr) {
        refInSymbolTable = symbol;
        return;
    }

    if (!symbol->is<FunctionDeclaration>()) {
        fContext.fErrors->error(symbol->fPosition, "symbol '" + std::string(name) +
                "' was already defined");
        return;
    }

    std::vector<const FunctionDeclaration*> functions;
    if (refInSymbolTable->is<FunctionDeclaration>()) {
        functions = {&refInSymbolTable->as<FunctionDeclaration>(),
                     &symbol->as<FunctionDeclaration>()};

        refInSymbolTable = this->takeOwnershipOfSymbol(
                std::make_unique<UnresolvedFunction>(std::move(functions)));
    } else if (refInSymbolTable->is<UnresolvedFunction>()) {
        functions = refInSymbolTable->as<UnresolvedFunction>().functions();
        functions.push_back(&symbol->as<FunctionDeclaration>());

        refInSymbolTable = this->takeOwnershipOfSymbol(
                std::make_unique<UnresolvedFunction>(std::move(functions)));
    }
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
