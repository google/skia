/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_VARIABLE
#define SKSL_VARIABLE

#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLModifiers.h"
#include "src/sksl/ir/SkSLSymbol.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVariableReference.h"

namespace SkSL {

class Expression;

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
class Variable : public Symbol {
public:
    using Storage = VariableStorage;

    static constexpr Kind kSymbolKind = Kind::kVariable;

    Variable(int offset, ModifiersPool::Handle modifiers, StringFragment name, const Type* type,
             bool builtin, Storage storage, const Expression* initialValue = nullptr)
    : INHERITED(offset, VariableData{name, type, initialValue, modifiers, storage, builtin}) {}

    const Type& type() const override {
        return *this->variableData().fType;
    }

    const Modifiers& modifiers() const {
        return *this->variableData().fModifiersHandle;
    }

    const ModifiersPool::Handle& modifiersHandle() const {
        return this->variableData().fModifiersHandle;
    }

    bool isBuiltin() const {
        return this->variableData().fBuiltin;
    }

    Storage storage() const {
        return (Storage) this->variableData().fStorage;
    }

    const Expression* initialValue() const {
        return this->variableData().fInitialValue;
    }

    void setInitialValue(const Expression* initialValue) {
        SkASSERT(!this->initialValue());
        this->variableData().fInitialValue = initialValue;
    }

    StringFragment name() const override {
        return this->variableData().fName;
    }

    String description() const override {
        return this->modifiers().description() + this->type().name() + " " + this->name();
    }

private:
    using INHERITED = Symbol;

    friend class VariableReference;
};

} // namespace SkSL

#endif
