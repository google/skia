/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/ir/SkSLUnresolvedFunction.h"

namespace SkSL {

std::vector<IRNode::ID> SymbolTable::getFunctions(IRNode::ID id) {
    const Symbol& s = (Symbol&) id.node();
    switch (s.fSymbolKind) {
        case Symbol::kFunctionDeclaration_Kind:
            return { id };
        case Symbol::kUnresolvedFunction_Kind:
            return ((UnresolvedFunction&) s).fFunctions;
        default:
            return std::vector<IRNode::ID>();
    }
}

IRNode::ID SymbolTable::operator[](StringFragment name) {
    const auto& entry = fSymbols.find(name);
    if (entry == fSymbols.end()) {
        if (fParent) {
            return (*fParent)[name];
        }
        return IRNode::ID();
    }
    if (fParent) {
        auto functions = this->getFunctions(entry->second);
        if (functions.size() > 0) {
            bool modified = false;
            IRNode::ID previous = (*fParent)[name];
            if (previous) {
                auto previousFunctions = this->getFunctions(previous);
                for (IRNode::ID prevID : previousFunctions) {
                    const FunctionDeclaration& prev = (FunctionDeclaration&) prevID.node();
                    bool found = false;
                    for (IRNode::ID currentID : functions) {
                        const FunctionDeclaration& current =
                                                            (FunctionDeclaration&) currentID.node();
                        if (current.matches(prev)) {
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        functions.push_back(prevID);
                        modified = true;
                    }
                }
                if (modified) {
                    SkASSERT(functions.size() > 1);
                    return fIRGenerator->createNode(new UnresolvedFunction(fIRGenerator,
                                                                           functions));
                }
            }
        }
    }
    return entry->second;
}

void SymbolTable::add(StringFragment name, IRNode::ID symbolID) {
    const auto& existing = fSymbols.find(name);
    Symbol& symbol = (Symbol&) symbolID.node();
    if (existing == fSymbols.end()) {
        fSymbols.insert({ name, symbolID });
    } else if (symbol.fSymbolKind == Symbol::kFunctionDeclaration_Kind) {
        const Symbol& oldSymbol = (Symbol&) existing->second.node();
        if (oldSymbol.fSymbolKind == Symbol::kFunctionDeclaration_Kind) {
            std::vector<IRNode::ID> functions;
            functions.push_back(existing->second);
            functions.push_back(symbolID);
            std::unique_ptr<Symbol> u = std::unique_ptr<Symbol>();
            fSymbols[name] = fIRGenerator->createNode(new UnresolvedFunction(fIRGenerator,
                                                                             std::move(functions)));
        } else if (oldSymbol.fSymbolKind == Symbol::kUnresolvedFunction_Kind) {
            std::vector<IRNode::ID> functions(((UnresolvedFunction&) oldSymbol).fFunctions);
            functions.push_back(symbolID);
            fSymbols[name] = fIRGenerator->createNode(new UnresolvedFunction(fIRGenerator,
                                                                             std::move(functions)));
        }
    } else {
        fIRGenerator->fErrors.error(symbol.fOffset, "symbol '" + name + "' was already defined");
    }
}


void SymbolTable::markAllFunctionsBuiltin() {
    for (const auto& pair : fSymbols) {
        Symbol& symbol = (Symbol&) pair.second.node();
        switch (symbol.fSymbolKind) {
            case Symbol::kFunctionDeclaration_Kind:
                ((FunctionDeclaration&) symbol).fBuiltin = true;
                break;
            case Symbol::kUnresolvedFunction_Kind:
                for (IRNode::ID f : ((UnresolvedFunction&) symbol).fFunctions) {
                    ((FunctionDeclaration&) f.node()).fBuiltin = true;
                }
                break;
            default:
                break;
        }
    }
}

std::unordered_map<StringFragment, IRNode::ID>::iterator SymbolTable::begin() {
    return fSymbols.begin();
}

std::unordered_map<StringFragment, IRNode::ID>::iterator SymbolTable::end() {
    return fSymbols.end();
}


} // namespace
