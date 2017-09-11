/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SYMBOLTABLE
#define SKSL_SYMBOLTABLE

#include <memory>
#include <unordered_map>
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

    const Symbol* operator[](const String& name);

    void add(const String& name, std::unique_ptr<Symbol> symbol);

    void addWithoutOwnership(const String& name, const Symbol* symbol);

    Symbol* takeOwnership(Symbol* s);

    void markAllFunctionsBuiltin();

    const std::shared_ptr<SymbolTable> fParent;

private:
    static std::vector<const FunctionDeclaration*> GetFunctions(const Symbol& s);

    std::vector<std::unique_ptr<Symbol>> fOwnedPointers;

    std::unordered_map<String, const Symbol*> fSymbols;

    ErrorReporter& fErrorReporter;
};

} // namespace

#endif
