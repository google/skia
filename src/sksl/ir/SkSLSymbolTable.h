/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SYMBOLTABLE
#define SKSL_SYMBOLTABLE

#include "include/private/SkSLString.h"
#include "include/private/SkSLSymbol.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTHash.h"
#include "src/sksl/SkSLErrorReporter.h"

#include <forward_list>
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
    SymbolTable(ErrorReporter* errorReporter, bool builtin)
    : fBuiltin(builtin)
    , fErrorReporter(*errorReporter) {}

    SymbolTable(std::shared_ptr<SymbolTable> parent, bool builtin)
    : fParent(parent)
    , fBuiltin(builtin)
    , fErrorReporter(parent->fErrorReporter) {}

    /**
     * If the input is a built-in symbol table, returns a new empty symbol table as a child of the
     * input table. If the input is not a built-in symbol table, returns it as-is. Built-in symbol
     * tables must not be mutated after creation, so they must be wrapped if mutation is necessary.
     */
    static std::shared_ptr<SymbolTable> WrapIfBuiltin(std::shared_ptr<SymbolTable> symbolTable) {
        if (!symbolTable) {
            return nullptr;
        }
        if (!symbolTable->isBuiltin()) {
            return symbolTable;
        }
        return std::make_shared<SymbolTable>(std::move(symbolTable), /*builtin=*/false);
    }

    /**
     * Looks up the requested symbol and returns it. If a function has overloads, an
     * UnresolvedFunction symbol (pointing to all of the candidates) will be added to the symbol
     * table and returned.
     */
    const Symbol* operator[](skstd::string_view name);

    /**
     * Creates a new name for a symbol which already exists; does not take ownership of Symbol*.
     */
    void addAlias(skstd::string_view name, const Symbol* symbol);

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

    /**
     * Given type = `float` and arraySize = 5, creates the array type `float[5]` in the symbol
     * table. The created array type is returned. `kUnsizedArray` can be passed as a `[]` dimension.
     * If zero is passed, the base type is returned unchanged.
     */
    const Type* addArrayDimension(const Type* type, int arraySize);

    // Call fn for every symbol in the table.  You may not mutate anything.
    template <typename Fn>
    void foreach(Fn&& fn) const {
        fSymbols.foreach(
                [&fn](const SymbolKey& key, const Symbol* symbol) { fn(key.fName, symbol); });
    }

    size_t count() {
        return fSymbols.count();
    }

    /** Returns true if this is a built-in SymbolTable. */
    bool isBuiltin() const {
        return fBuiltin;
    }

    const String* takeOwnershipOfString(String n);

    std::shared_ptr<SymbolTable> fParent;

    std::vector<std::unique_ptr<const Symbol>> fOwnedSymbols;

private:
    struct SymbolKey {
        skstd::string_view fName;
        uint32_t       fHash;

        bool operator==(const SymbolKey& that) const { return fName == that.fName; }
        bool operator!=(const SymbolKey& that) const { return fName != that.fName; }
        struct Hash {
            uint32_t operator()(const SymbolKey& key) const { return key.fHash; }
        };
    };

    static SymbolKey MakeSymbolKey(skstd::string_view name) {
        return SymbolKey{name, SkOpts::hash_fn(name.data(), name.size(), 0)};
    }

    const Symbol* lookup(SymbolTable* writableSymbolTable, const SymbolKey& key);

    static std::vector<const FunctionDeclaration*> GetFunctions(const Symbol& s);

    bool fBuiltin = false;
    std::vector<std::unique_ptr<IRNode>> fOwnedNodes;
    std::forward_list<String> fOwnedStrings;
    SkTHashMap<SymbolKey, const Symbol*, SymbolKey::Hash> fSymbols;
    ErrorReporter& fErrorReporter;

    friend class Dehydrator;
};

}  // namespace SkSL

#endif
