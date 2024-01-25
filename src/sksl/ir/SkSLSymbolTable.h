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
#include <utility>
#include <vector>

namespace SkSL {

class Context;
class Expression;
class Position;
class Type;

/**
 * Maps identifiers to symbols.
 */
class SymbolTable {
public:
    explicit SymbolTable(bool builtin)
            : fBuiltin(builtin) {}

    explicit SymbolTable(SymbolTable* parent, bool builtin)
            : fParent(parent)
            , fBuiltin(builtin) {}

    /**
     * Creates a new, empty SymbolTable between this SymbolTable and its current parent.
     * The new symbol table is returned, and is also accessible as `this->fParent`.
     * The original parent is accessible as `this->fParent->fParent`.
     */
    std::unique_ptr<SymbolTable> insertNewParent();

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
     * Looks up the requested symbol and instantiates an Expression reference to it; will return a
     * VariableReference, TypeReference, FunctionReference, FieldAccess, or nullptr.
     */
    std::unique_ptr<Expression> instantiateSymbolRef(const Context& context,
                                                     std::string_view name,
                                                     Position pos);

    /**
     * Assigns a new name to the passed-in symbol. The old name will continue to exist in the symbol
     * table and point to the symbol.
     */
    void renameSymbol(const Context& context, Symbol* symbol, std::string_view newName);

    /**
     * Removes a symbol from the symbol table. If this symbol table had ownership of the symbol, the
     * symbol is returned (and can be deleted or reinserted as desired); if not, null is returned.
     * In either event, the name will no longer correspond to the symbol.
     */
    std::unique_ptr<Symbol> removeSymbol(const Symbol* symbol);

    /**
     * Moves a symbol from this symbol table to another one. If this symbol table had ownership of
     * the symbol, the ownership will be transferred as well. (If the symbol does not actually exist
     * in this table at all, it will still be added to the other table.)
     */
    void moveSymbolTo(SymbolTable* otherTable, Symbol* sym, const Context& context);

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
    void addWithoutOwnershipOrDie(Symbol* symbol);
    void addWithoutOwnership(const Context& context, Symbol* symbol);

    /**
     * Adds a symbol to this symbol table, conferring ownership. The symbol table will always be
     * updated to reference the new symbol. If the symbol already exists, an error will be reported.
     */
    template <typename T>
    T* add(const Context& context, std::unique_ptr<T> symbol) {
        T* ptr = symbol.get();
        this->addWithoutOwnership(context, this->takeOwnershipOfSymbol(std::move(symbol)));
        return ptr;
    }

    /**
     * Adds a symbol to this symbol table, conferring ownership. The symbol table will always be
     * updated to reference the new symbol. If the symbol already exists, abort.
     */
    template <typename T>
    T* addOrDie(std::unique_ptr<T> symbol) {
        T* ptr = symbol.get();
        this->addWithoutOwnershipOrDie(this->takeOwnershipOfSymbol(std::move(symbol)));
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
    const Type* addArrayDimension(const Context& context, const Type* type, int arraySize);

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

    SymbolTable* fParent = nullptr;

    std::vector<std::unique_ptr<Symbol>> fOwnedSymbols;

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
    bool addWithoutOwnership(Symbol* symbol);

    bool fBuiltin = false;
    bool fAtModuleBoundary = false;
    std::forward_list<std::string> fOwnedStrings;
    skia_private::THashMap<SymbolKey, Symbol*, SymbolKey::Hash> fSymbols;
};

}  // namespace SkSL

#endif
