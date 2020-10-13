/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SYMBOLTABLE
#define SKSL_SYMBOLTABLE

#include "include/private/SkTHash.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/ir/SkSLSymbol.h"

#include <memory>
#include <vector>

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

    const Symbol* operator[](StringFragment name);

    void addAlias(StringFragment name, const Symbol* symbol);
    void addWithoutOwnership(const Symbol* symbol);

    template <typename T>
    const T* add(std::unique_ptr<T> symbol) {
        const T* ptr = symbol.get();
        this->addWithoutOwnership(ptr);
        this->takeOwnershipOfSymbol(std::move(symbol));
        return ptr;
    }

    template <typename T>
    const T* takeOwnershipOfSymbol(std::unique_ptr<T> symbol) {
        const T* ptr = symbol.get();
        fOwnedSymbols.push_back(std::move(symbol));
        return ptr;
    }

    template <typename T>
    const T* takeOwnershipOfIRNode(std::unique_ptr<T> node) {
        const T* ptr = node.get();
        fOwnedNodes.push_back(std::move(node));
        return ptr;
    }

    // Call fn for every symbol in the table.  You may not mutate anything.
    template <typename Fn>
    void foreach(Fn&& fn) const {
        fSymbols.foreach(
                [&fn](const SymbolKey& key, const Symbol* symbol) { fn(key.fName, symbol); });
    }

    size_t count() {
        return fSymbols.count();
    }

    const String* takeOwnershipOfString(std::unique_ptr<String> n);

    std::shared_ptr<SymbolTable> fParent;

    std::vector<std::unique_ptr<const Symbol>> fOwnedSymbols;

private:
    struct SymbolKey {
        StringFragment fName;
        uint32_t       fHash;

        bool operator==(const SymbolKey& that) const { return fName == that.fName; }
        bool operator!=(const SymbolKey& that) const { return fName != that.fName; }
        struct Hash {
            uint32_t operator()(const SymbolKey& key) const { return key.fHash; }
        };
    };

    static SymbolKey MakeSymbolKey(StringFragment name) {
        return SymbolKey{name, SkOpts::hash_fn(name.data(), name.size(), 0)};
    }

    const Symbol* lookup(const SymbolKey& key);
    static std::vector<const FunctionDeclaration*> GetFunctions(const Symbol& s);

    std::vector<std::unique_ptr<IRNode>> fOwnedNodes;
    std::vector<std::unique_ptr<String>> fOwnedStrings;
    SkTHashMap<SymbolKey, const Symbol*, SymbolKey::Hash> fSymbols;
    ErrorReporter& fErrorReporter;

    friend class Dehydrator;
};

}  // namespace SkSL

#endif
