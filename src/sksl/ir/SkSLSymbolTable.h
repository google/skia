/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SYMBOLTABLE
#define SKSL_SYMBOLTABLE

#include <unordered_map>
#include <memory>
#include <vector>
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/ir/SkSLSymbol.h"

namespace SkSL {

class FunctionDeclaration;

/**
 * Maps identifiers to symbols. Functions, in particular, are mapped to either FunctionDeclaration
 * or UnresolvedFunction depending on whether they are overloaded or not.
 */
class SymbolTable {
public:
    SymbolTable(ErrorReporter* errorReporter)
    : fErrorReporter(*errorReporter) {}

    SymbolTable(std::shared_ptr<SymbolTable> parent)
    : fParent(parent)
    , fErrorReporter(parent->fErrorReporter) {}

    Symbol* operator[](StringFragment name);

    void addAlias(StringFragment name, Symbol* symbol);
    void addWithoutOwnership(Symbol* symbol);

    template <typename T>
    T* add(std::unique_ptr<T> symbol) {
        T* ptr = symbol.get();
        this->addWithoutOwnership(ptr);
        this->takeOwnershipOfSymbol(std::move(symbol));
        return ptr;
    }

    template <typename T>
    T* takeOwnershipOfSymbol(std::unique_ptr<T> symbol) {
        T* ptr = symbol.get();
        fOwnedSymbols.push_back(std::move(symbol));
        return ptr;
    }

    template <typename T>
    T* takeOwnershipOfIRNode(std::unique_ptr<T> node) {
        T* ptr = node.get();
        fOwnedNodes.push_back(std::move(node));
        return ptr;
    }

    const String* takeOwnershipOfString(std::unique_ptr<String> n);

    std::unordered_map<StringFragment, Symbol*>::iterator begin();

    std::unordered_map<StringFragment, Symbol*>::iterator end();

    std::shared_ptr<SymbolTable> fParent;

    std::vector<std::unique_ptr<const Symbol>> fOwnedSymbols;

private:
    static std::vector<const FunctionDeclaration*> GetFunctions(const Symbol& s);

    std::vector<std::unique_ptr<IRNode>> fOwnedNodes;

    std::vector<std::unique_ptr<String>> fOwnedStrings;

    std::unordered_map<StringFragment, Symbol*> fSymbols;

    ErrorReporter& fErrorReporter;

    friend class Dehydrator;
};

}  // namespace SkSL

#endif
