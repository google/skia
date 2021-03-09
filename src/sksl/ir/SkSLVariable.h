/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_VARIABLE
#define SKSL_VARIABLE

#include "include/private/SkSLModifiers.h"
#include "include/private/SkSLSymbol.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVariableReference.h"

namespace SkSL {

class Expression;
class VarDeclaration;

namespace dsl {
class DSLCore;
} // namespace dsl

enum class VariableStorage : int8_t {
    kGlobal,
    kInterfaceBlock,
    kLocal,
    kParameter
};

/**
 * Represents a variable, whether local, global, or a function parameter. This represents the
 * variable itself (the storage location), which is shared between all VariableReferences which
 * read or write that storage location.
 */
class Variable final : public Symbol {
public:
    using Storage = VariableStorage;

    static constexpr Kind kSymbolKind = Kind::kVariable;

    Variable(int offset, const Modifiers* modifiers, StringFragment name, const Type* type,
             bool builtin, Storage storage)
    : INHERITED(offset, kSymbolKind, name, type)
    , fModifiers(modifiers)
    , fStorage(storage)
    , fBuiltin(builtin) {}

    const Modifiers& modifiers() const {
        return *fModifiers;
    }

    bool isBuiltin() const {
        return fBuiltin;
    }

    Storage storage() const {
        return (Storage) fStorage;
    }

    const Expression* initialValue() const;

    void setDeclaration(VarDeclaration* declaration) {
        SkASSERT(!fDeclaration);
        fDeclaration = declaration;
    }

    String description() const override {
        return this->modifiers().description() + this->type().name() + " " + this->name();
    }

private:
    VarDeclaration* fDeclaration = nullptr;
    const Modifiers* fModifiers;
    VariableStorage fStorage;
    bool fBuiltin;

    using INHERITED = Symbol;

    friend class dsl::DSLCore;
    friend class VariableReference;
};

} // namespace SkSL

#endif
