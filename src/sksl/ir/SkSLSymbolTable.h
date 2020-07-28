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

struct FunctionDeclaration;

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

    const Symbol* operator[](StringFragment name);

    void add(StringFragment name, std::unique_ptr<const Symbol> symbol);

    void addWithoutOwnership(StringFragment name, const Symbol* symbol);

    const Symbol* takeOwnership(std::unique_ptr<const Symbol> s);

    IRNode* takeOwnership(std::unique_ptr<IRNode> n);

    String* takeOwnership(std::unique_ptr<String> n);

    std::unordered_map<StringFragment, const Symbol*>::iterator begin();

    std::unordered_map<StringFragment, const Symbol*>::iterator end();

    const std::shared_ptr<SymbolTable> fParent;

    std::vector<std::unique_ptr<const Symbol>> fOwnedSymbols;

private:
    static std::vector<const FunctionDeclaration*> GetFunctions(const Symbol& s);

    std::vector<std::unique_ptr<IRNode>> fOwnedNodes;

    std::vector<std::unique_ptr<String>> fOwnedStrings;

    std::unordered_map<StringFragment, const Symbol*> fSymbols;

    ErrorReporter& fErrorReporter;

    friend class Dehydrator;
};

} // namespace

#endif
