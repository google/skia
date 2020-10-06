/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLSymbolTable.h"

#include "src/sksl/ir/SkSLSymbolAlias.h"
#include "src/sksl/ir/SkSLUnresolvedFunction.h"

namespace SkSL {

std::vector<const FunctionDeclaration*> SymbolTable::GetFunctions(const Symbol& s) {
    switch (s.kind()) {
        case Symbol::Kind::kFunctionDeclaration:
            return { &s.as<FunctionDeclaration>() };
        case Symbol::Kind::kUnresolvedFunction:
            return s.as<UnresolvedFunction>().fFunctions;
        default:
            return std::vector<const FunctionDeclaration*>();
    }
}

const Symbol* SymbolTable::operator[](StringFragment name) {
    const auto& entry = fSymbols.find(name);
    if (entry == fSymbols.end()) {
        if (fParent) {
            return (*fParent)[name];
        }
        return nullptr;
    }
    if (fParent) {
        auto functions = GetFunctions(*entry->second);
        if (functions.size() > 0) {
            bool modified = false;
            const Symbol* previous = (*fParent)[name];
            if (previous) {
                auto previousFunctions = GetFunctions(*previous);
                for (const FunctionDeclaration* prev : previousFunctions) {
                    bool found = false;
                    for (const FunctionDeclaration* current : functions) {
                        if (current->matches(*prev)) {
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        functions.push_back(prev);
                        modified = true;
                    }
                }
                if (modified) {
                    SkASSERT(functions.size() > 1);
                    return this->takeOwnershipOfSymbol(
                            std::make_unique<UnresolvedFunction>(functions));
                }
            }
        }
    }
    const Symbol* symbol = entry->second;
    while (symbol && symbol->is<SymbolAlias>()) {
        symbol = symbol->as<SymbolAlias>().origSymbol();
    }
    return symbol;
}

const String* SymbolTable::takeOwnershipOfString(std::unique_ptr<String> n) {
    String* result = n.get();
    fOwnedStrings.push_back(std::move(n));
    return result;
}

void SymbolTable::addAlias(StringFragment name, const Symbol* symbol) {
    this->add(name, std::make_unique<SymbolAlias>(symbol->fOffset, name, symbol));
}

void SymbolTable::addWithoutOwnership(StringFragment name, const Symbol* symbol) {
    SkASSERT(symbol->name() == name);

    const Symbol*& refInSymbolTable = fSymbols[name];
    if (refInSymbolTable == nullptr) {
        refInSymbolTable = symbol;
        return;
    }

    if (!symbol->is<FunctionDeclaration>()) {
        fErrorReporter.error(symbol->fOffset, "symbol '" + name + "' was already defined");
        return;
    }

    std::vector<const FunctionDeclaration*> functions;
    if (refInSymbolTable->is<FunctionDeclaration>()) {
        functions = {&refInSymbolTable->as<FunctionDeclaration>(),
                     &symbol->as<FunctionDeclaration>()};

        refInSymbolTable = this->takeOwnershipOfSymbol(
                std::make_unique<UnresolvedFunction>(std::move(functions)));
    } else if (refInSymbolTable->is<UnresolvedFunction>()) {
        functions = refInSymbolTable->as<UnresolvedFunction>().fFunctions;
        functions.push_back(&symbol->as<FunctionDeclaration>());

        refInSymbolTable = this->takeOwnershipOfSymbol(
                std::make_unique<UnresolvedFunction>(std::move(functions)));
    }
}

std::unordered_map<StringFragment, const Symbol*>::iterator SymbolTable::begin() {
    return fSymbols.begin();
}

std::unordered_map<StringFragment, const Symbol*>::iterator SymbolTable::end() {
    return fSymbols.end();
}

}  // namespace SkSL
