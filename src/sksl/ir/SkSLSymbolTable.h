/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SYMBOLTABLE
#define SKSL_SYMBOLTABLE

#include <map>
#include <memory>
#include <vector>
#include "SkSLErrorReporter.h"
#include "SkSLSymbol.h"

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

    SymbolTable(std::shared_ptr<SymbolTable> parent, ErrorReporter* errorReporter)
    : fParent(parent)
    , fErrorReporter(*errorReporter) {}

    const Symbol* operator[](StringFragment name);

    void add(StringFragment name, std::unique_ptr<Symbol> symbol);

    void addWithoutOwnership(StringFragment name, const Symbol* symbol);

    Symbol* takeOwnership(Symbol* s);

    IRNode* takeOwnership(IRNode* n);

    void markAllFunctionsBuiltin();

    std::map<StringFragment, const Symbol*>::iterator begin();

    std::map<StringFragment, const Symbol*>::iterator end();

    const std::shared_ptr<SymbolTable> fParent;

private:
    static std::vector<const FunctionDeclaration*> GetFunctions(const Symbol& s);

    std::vector<std::unique_ptr<Symbol>> fOwnedSymbols;

    std::vector<std::unique_ptr<IRNode>> fOwnedNodes;

    std::map<StringFragment, const Symbol*> fSymbols;

    ErrorReporter& fErrorReporter;
};

} // namespace

#endif
