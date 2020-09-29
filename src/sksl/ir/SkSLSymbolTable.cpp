/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLSymbolTable.h"
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

// Returns true if the function array was modified.
bool add_function_decl_if_missing(const FunctionDeclaration* candidate,
                                  std::vector<const FunctionDeclaration*>* functions) {
    for (const FunctionDeclaration* current : *functions) {
        if (current->matches(*candidate)) {
            return false;
        }
    }

    functions->push_back(candidate);
    return true;
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
        std::vector<const FunctionDeclaration*> functions = GetFunctions(*entry->second);
        if (!functions.empty()) {
            const Symbol* previous = (*fParent)[name];
            if (previous) {
                bool modified = false;

                if (previous->is<FunctionDeclaration>()) {
                    modified = add_function_decl_if_missing(&previous->as<FunctionDeclaration>(),
                                                            &functions);
                } else if (previous->is<UnresolvedFunction>()) {
                    for (const FunctionDeclaration* prev :
                         previous->as<UnresolvedFunction>().fFunctions) {
                        modified |= add_function_decl_if_missing(prev, &functions);
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
    return entry->second;
}

const String* SymbolTable::takeOwnershipOfString(std::unique_ptr<String> n) {
    String* result = n.get();
    fOwnedStrings.push_back(std::move(n));
    return result;
}

void SymbolTable::addWithoutOwnership(StringFragment name, const Symbol* symbol) {
    const auto& existing = fSymbols.find(name);
    if (existing == fSymbols.end()) {
        fSymbols[name] = symbol;
    } else if (symbol->kind() == Symbol::Kind::kFunctionDeclaration) {
        const Symbol* oldSymbol = existing->second;
        if (oldSymbol->kind() == Symbol::Kind::kFunctionDeclaration) {
            std::vector<const FunctionDeclaration*> functions;
            functions.push_back(&oldSymbol->as<FunctionDeclaration>());
            functions.push_back(&symbol->as<FunctionDeclaration>());
            std::unique_ptr<const Symbol> u = std::unique_ptr<const Symbol>(
                                                      new UnresolvedFunction(std::move(functions)));
            fSymbols[name] = this->takeOwnershipOfSymbol(std::move(u));
        } else if (oldSymbol->kind() == Symbol::Kind::kUnresolvedFunction) {
            std::vector<const FunctionDeclaration*> functions;
            for (const auto* f : oldSymbol->as<UnresolvedFunction>().fFunctions) {
                functions.push_back(f);
            }
            functions.push_back(&symbol->as<FunctionDeclaration>());
            std::unique_ptr<const Symbol> u = std::unique_ptr<const Symbol>(
                                                      new UnresolvedFunction(std::move(functions)));
            fSymbols[name] = this->takeOwnershipOfSymbol(std::move(u));
        }
    } else {
        fErrorReporter.error(symbol->fOffset, "symbol '" + name + "' was already defined");
    }
}

std::unordered_map<StringFragment, const Symbol*>::iterator SymbolTable::begin() {
    return fSymbols.begin();
}

std::unordered_map<StringFragment, const Symbol*>::iterator SymbolTable::end() {
    return fSymbols.end();
}

}  // namespace SkSL
