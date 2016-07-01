/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

 #include "SkSLSymbolTable.h"

namespace SkSL {

std::vector<std::shared_ptr<FunctionDeclaration>> SymbolTable::GetFunctions(
                                                                 const std::shared_ptr<Symbol>& s) {
    switch (s->fKind) {
        case Symbol::kFunctionDeclaration_Kind:
            return { std::static_pointer_cast<FunctionDeclaration>(s) };
        case Symbol::kUnresolvedFunction_Kind:
            return ((UnresolvedFunction&) *s).fFunctions;
        default:
            return { };
    }
}

std::shared_ptr<Symbol> SymbolTable::operator[](const std::string& name) {
    const auto& entry = fSymbols.find(name);
    if (entry == fSymbols.end()) {
        if (fParent) {
            return (*fParent)[name];
        }
        return nullptr;
    }
    if (fParent) {
        auto functions = GetFunctions(entry->second);
        if (functions.size() > 0) {
            bool modified = false;
            std::shared_ptr<Symbol> previous = (*fParent)[name];
            if (previous) {
                auto previousFunctions = GetFunctions(previous);
                for (const std::shared_ptr<FunctionDeclaration>& prev : previousFunctions) {
                    bool found = false;
                    for (const std::shared_ptr<FunctionDeclaration>& current : functions) {
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
                    ASSERT(functions.size() > 1);
                    return std::shared_ptr<Symbol>(new UnresolvedFunction(functions));
                }
            }
        }
    }
    return entry->second;
}

void SymbolTable::add(const std::string& name, std::shared_ptr<Symbol> symbol) {
        const auto& existing = fSymbols.find(name);
        if (existing == fSymbols.end()) {
            fSymbols[name] = symbol;
        } else if (symbol->fKind == Symbol::kFunctionDeclaration_Kind) {
            const std::shared_ptr<Symbol>& oldSymbol = existing->second;
            if (oldSymbol->fKind == Symbol::kFunctionDeclaration_Kind) {
                std::vector<std::shared_ptr<FunctionDeclaration>> functions;
                functions.push_back(std::static_pointer_cast<FunctionDeclaration>(oldSymbol));
                functions.push_back(std::static_pointer_cast<FunctionDeclaration>(symbol));
                fSymbols[name].reset(new UnresolvedFunction(std::move(functions)));
            } else if (oldSymbol->fKind == Symbol::kUnresolvedFunction_Kind) {
                std::vector<std::shared_ptr<FunctionDeclaration>> functions;
                for (const auto& f : ((UnresolvedFunction&) *oldSymbol).fFunctions) {
                    functions.push_back(f);
                }
                functions.push_back(std::static_pointer_cast<FunctionDeclaration>(symbol));
                fSymbols[name].reset(new UnresolvedFunction(std::move(functions)));
            }
        } else {
            fErrorReporter.error(symbol->fPosition, "symbol '" + name + "' was already defined");
        }
    }
} // namespace
