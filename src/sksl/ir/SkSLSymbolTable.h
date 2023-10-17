/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SYMBOLTABLE
#define SKSL_SYMBOLTABLE

#include "include/core/SkTypes.h"
#include "src/core/SkChecksum.h"
#include "src/core/SkTHash.h"
#include "src/sksl/ir/SkSLSymbol.h"

#include <cstddef>
#include <cstdint>
#include <forward_list>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace SkSL {

class Type;

/**
 * Maps identifiers to symbols.
 */
class SymbolTable {
public:
    explicit SymbolTable(bool builtin)
            : fBuiltin(builtin) {}

    explicit SymbolTable(std::shared_ptr<SymbolTable> parent, bool builtin)
            : fParent(std::move(parent))
            , fBuiltin(builtin) {}

    /** Replaces the passed-in SymbolTable with a newly-created child symbol table. */
    static void Push(std::shared_ptr<SymbolTable>* table) {
        Push(table, (*table)->isBuiltin());
    }
    static void Push(std::shared_ptr<SymbolTable>* table, bool isBuiltin) {
        *table = std::make_shared<SymbolTable>(*table, isBuiltin);
    }

    /**
     * Replaces the passed-in SymbolTable with its parent. If the child symbol table is otherwise
     * unreferenced, it will be deleted.
     */
    static void Pop(std::shared_ptr<SymbolTable>* table) {
        *table = (*table)->fParent;
    }

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
     * Looks up the requested symbol and returns a const pointer.
     */
    const Symbol* find(std::string_view name) const {
        return this->lookup(MakeSymbolKey(name));
    }

    /**
     * Looks up the requested symbol, only searching the built-in symbol tables. Always const.
     */
    const Symbol* findBuiltinSymbol(std::string_view name) const;

    /**
     * Looks up the requested symbol and returns a mutable pointer. Use caution--mutating a symbol
     * will have program-wide impact, and built-in symbol tables must never be mutated.
     */
    Symbol* findMutable(std::string_view name) const {
        return this->lookup(MakeSymbolKey(name));
    }

    /**
     * Assigns a new name to the passed-in symbol. The old name will continue to exist in the symbol
     * table and point to the symbol.
     */
    void renameSymbol(Symbol* symbol, std::string_view newName);

    /**
     * Returns true if the name refers to a type (user or built-in) in the current symbol table.
     */
    bool isType(std::string_view name) const;

    /**
     * Returns true if the name refers to a builtin type.
     */
    bool isBuiltinType(std::string_view name) const;

    /**
     * Adds a symbol to this symbol table, without conferring ownership. The caller is responsible
     * for keeping the Symbol alive throughout the lifetime of the program/module.
     */
    void addWithoutOwnership(Symbol* symbol);

    /**
     * Adds a symbol to this symbol table, conferring ownership.
     */
    template <typename T>
    T* add(std::unique_ptr<T> symbol) {
        T* ptr = symbol.get();
        this->addWithoutOwnership(this->takeOwnershipOfSymbol(std::move(symbol)));
        return ptr;
    }

    /**
     * Forces a symbol into this symbol table, without conferring ownership. Replaces any existing
     * symbol with the same name, if one exists.
     */
    void injectWithoutOwnership(Symbol* symbol);

    /**
     * Forces a symbol into this symbol table, conferring ownership. Replaces any existing symbol
     * with the same name, if one exists.
     */
    template <typename T>
    T* inject(std::unique_ptr<T> symbol) {
        T* ptr = symbol.get();
        this->injectWithoutOwnership(this->takeOwnershipOfSymbol(std::move(symbol)));
        return ptr;
    }

    /**
     * Confers ownership of a symbol without adding its name to the lookup table.
     */
    template <typename T>
    T* takeOwnershipOfSymbol(std::unique_ptr<T> symbol) {
        T* ptr = symbol.get();
        fOwnedSymbols.push_back(std::move(symbol));
        return ptr;
    }

    /**
     * Given type = `float` and arraySize = 5, creates the array type `float[5]` in the symbol
     * table. The created array type is returned. If zero is passed, the base type is returned
     * unchanged.
     */
    const Type* addArrayDimension(const Type* type, int arraySize);

    // Call fn for every symbol in the table. You may not mutate anything.
    template <typename Fn>
    void foreach(Fn&& fn) const {
        fSymbols.foreach(
                [&fn](const SymbolKey& key, const Symbol* symbol) { fn(key.fName, symbol); });
    }

    // Checks `this` directly against `other` to see if the two symbol tables have any names in
    // common. Parent tables are not considered.
    bool wouldShadowSymbolsFrom(const SymbolTable* other) const;

    size_t count() const {
        return fSymbols.count();
    }

    /** Returns true if this is a built-in SymbolTable. */
    bool isBuiltin() const {
        return fBuiltin;
    }

    const std::string* takeOwnershipOfString(std::string n);

    /**
     * Indicates that this symbol table's parent is in a different module than this one.
     */
    void markModuleBoundary() {
        fAtModuleBoundary = true;
    }

    std::shared_ptr<SymbolTable> fParent;

    std::vector<std::unique_ptr<const Symbol>> fOwnedSymbols;

private:
    struct SymbolKey {
        std::string_view fName;
        uint32_t         fHash;

        bool operator==(const SymbolKey& that) const { return fName == that.fName; }
        bool operator!=(const SymbolKey& that) const { return fName != that.fName; }
        struct Hash {
            uint32_t operator()(const SymbolKey& key) const { return key.fHash; }
        };
    };

    static SymbolKey MakeSymbolKey(std::string_view name) {
        return SymbolKey{name, SkChecksum::Hash32(name.data(), name.size())};
    }

    Symbol* lookup(const SymbolKey& key) const;

    bool fBuiltin = false;
    bool fAtModuleBoundary = false;
    std::forward_list<std::string> fOwnedStrings;
    skia_private::THashMap<SymbolKey, Symbol*, SymbolKey::Hash> fSymbols;
};

}  // namespace SkSL

#endif
