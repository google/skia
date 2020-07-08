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

    SymbolTable(std::shared_ptr<SymbolTable> parent, ErrorReporter* errorReporter)
    : fParent(parent)
    , fErrorReporter(*errorReporter) {}

    SymbolTable(std::shared_ptr<SymbolTable> parent, ErrorReporter* errorReporter,
                std::vector<std::unique_ptr<Symbol>> ownedSymbols,
                std::vector<std::pair<const char*, int>> map)
    : fParent(parent)
    , fErrorReporter(*errorReporter) {
        for (std::pair<const char*, int> entry : map) {
            fSymbols.insert({ entry.first, ownedSymbols[entry.second].get() });
        }
        for (std::unique_ptr<Symbol>& s : ownedSymbols) {
            this->takeOwnership(std::move(s));
        }
    }

    ~SymbolTable() {
        valid = false;
    }

    const Symbol* operator[](StringFragment name);

// TEMPORARY remove this
    const Symbol* get(StringFragment name) { const Symbol* result = (*this)[name]; if (!result) { printf("FAILED TO FIND: %s\n", String(name).c_str()); SkASSERT(false); } return result; }

    void add(StringFragment name, std::unique_ptr<Symbol> symbol);

    void addWithoutOwnership(StringFragment name, const Symbol* symbol);

    Symbol* takeOwnership(std::unique_ptr<Symbol> s);

    IRNode* takeOwnership(std::unique_ptr<IRNode> n);

    String* takeOwnership(std::unique_ptr<String> n);

#ifdef SKSL_STANDALONE
    String constructionCode() const;
#endif

    std::unordered_map<StringFragment, const Symbol*>::iterator begin();

    std::unordered_map<StringFragment, const Symbol*>::iterator end();

    const std::shared_ptr<SymbolTable> fParent;

//private:
    void appendMapCode(String& mapCode, const Symbol* s) const;

    static std::vector<const FunctionDeclaration*> GetFunctions(const Symbol& s);

    std::vector<std::unique_ptr<Symbol>> fOwnedSymbols;

    std::vector<std::unique_ptr<IRNode>> fOwnedNodes;

    std::vector<std::unique_ptr<String>> fOwnedStrings;

    std::unordered_map<StringFragment, const Symbol*> fSymbols;

    ErrorReporter& fErrorReporter;

// FIXME DON'T LAND WITH THIS
    bool valid = true;
};

} // namespace

#endif
