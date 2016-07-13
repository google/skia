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
#include "SkSLErrorReporter.h"
#include "SkSLSymbol.h"
#include "SkSLUnresolvedFunction.h"

namespace SkSL {

/**
 * Maps identifiers to symbols. Functions, in particular, are mapped to either FunctionDeclaration
 * or UnresolvedFunction depending on whether they are overloaded or not.
 */
class SymbolTable {
public:
    SymbolTable(ErrorReporter& errorReporter)
    : fErrorReporter(errorReporter) {}

    SymbolTable(std::shared_ptr<SymbolTable> parent, ErrorReporter& errorReporter)
    : fParent(parent)
    , fErrorReporter(errorReporter) {}

    std::shared_ptr<Symbol> operator[](const std::string& name);

    void add(const std::string& name, std::shared_ptr<Symbol> symbol);

    const std::shared_ptr<SymbolTable> fParent;

private:
    static std::vector<std::shared_ptr<FunctionDeclaration>> GetFunctions(
                                                                  const std::shared_ptr<Symbol>& s);

    std::unordered_map<std::string, std::shared_ptr<Symbol>> fSymbols;

    ErrorReporter& fErrorReporter;
};

} // namespace

#endif
