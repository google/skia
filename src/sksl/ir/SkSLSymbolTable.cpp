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
    switch (s.fKind) {
        case Symbol::kFunctionDeclaration_Kind:
            return { &((FunctionDeclaration&) s) };
        case Symbol::kUnresolvedFunction_Kind:
            return ((UnresolvedFunction&) s).fFunctions;
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
                    return this->takeOwnership(std::unique_ptr<Symbol>(
                                                                new UnresolvedFunction(functions)));
                }
            }
        }
    }
    return entry->second;
}

Symbol* SymbolTable::takeOwnership(std::unique_ptr<Symbol> s) {
    Symbol* result = s.get();
    fOwnedSymbols.push_back(std::move(s));
    return result;
}

IRNode* SymbolTable::takeOwnership(std::unique_ptr<IRNode> n) {
    IRNode* result = n.get();
    fOwnedNodes.push_back(std::move(n));
    return result;
}

void SymbolTable::add(StringFragment name, std::unique_ptr<Symbol> symbol) {
    this->addWithoutOwnership(name, symbol.get());
    this->takeOwnership(std::move(symbol));
}

void SymbolTable::addWithoutOwnership(StringFragment name, const Symbol* symbol) {
    const auto& existing = fSymbols.find(name);
    if (existing == fSymbols.end()) {
        fSymbols[name] = symbol;
    } else if (symbol->fKind == Symbol::kFunctionDeclaration_Kind) {
        const Symbol* oldSymbol = existing->second;
        if (oldSymbol->fKind == Symbol::kFunctionDeclaration_Kind) {
            std::vector<const FunctionDeclaration*> functions;
            functions.push_back((const FunctionDeclaration*) oldSymbol);
            functions.push_back((const FunctionDeclaration*) symbol);
            std::unique_ptr<Symbol> u = std::unique_ptr<Symbol>(new UnresolvedFunction(std::move(
                                                                                       functions)));
            fSymbols[name] = this->takeOwnership(std::move(u));
        } else if (oldSymbol->fKind == Symbol::kUnresolvedFunction_Kind) {
            std::vector<const FunctionDeclaration*> functions;
            for (const auto* f : ((UnresolvedFunction&) *oldSymbol).fFunctions) {
                functions.push_back(f);
            }
            functions.push_back((const FunctionDeclaration*) symbol);
            std::unique_ptr<Symbol> u = std::unique_ptr<Symbol>(new UnresolvedFunction(std::move(
                                                                                       functions)));
            fSymbols[name] = this->takeOwnership(std::move(u));
        }
    } else {
        fErrorReporter.error(symbol->fOffset, "symbol '" + name + "' was already defined");
    }
}


void SymbolTable::markAllFunctionsBuiltin() {
    for (const auto& pair : fSymbols) {
        switch (pair.second->fKind) {
            case Symbol::kFunctionDeclaration_Kind:
                if (!((FunctionDeclaration&)*pair.second).fDefined) {
                    ((FunctionDeclaration&)*pair.second).fBuiltin = true;
                }
                break;
            case Symbol::kUnresolvedFunction_Kind:
                for (auto& f : ((UnresolvedFunction&) *pair.second).fFunctions) {
                    if (!((FunctionDeclaration*)f)->fDefined) {
                        ((FunctionDeclaration*)f)->fBuiltin = true;
                    }
                }
                break;
            default:
                break;
        }
    }
}

std::unordered_map<StringFragment, const Symbol*>::iterator SymbolTable::begin() {
    return fSymbols.begin();
}

std::unordered_map<StringFragment, const Symbol*>::iterator SymbolTable::end() {
    return fSymbols.end();
}


} // namespace
