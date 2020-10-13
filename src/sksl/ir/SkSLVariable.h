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
    : INHERITED(offset, VariableData{name, type, initialValue, modifiers, /*readCount=*/0,
                                     /*writeCount=*/(int16_t) (initialValue ? 1 : 0),
                                     storage, builtin}) {}

    ~Variable() override {
        // can't destroy a variable while there are remaining references to it
        if (this->initialValue()) {
            --this->variableData().fWriteCount;
        }
        SkASSERT(!this->variableData().fReadCount && !this->variableData().fWriteCount);
    }

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
        SkASSERT(this->variableData().fWriteCount == 0);
        this->variableData().fInitialValue = initialValue;
        ++this->variableData().fWriteCount;
    }

    int readCount() const {
        return this->variableData().fReadCount;
    }

    int writeCount() const {
        return this->variableData().fWriteCount;
    }

    StringFragment name() const override {
        return this->variableData().fName;
    }

    String description() const override {
        return this->modifiers().description() + this->type().name() + " " + this->name();
    }

    bool dead() const {
        const VariableData& data = this->variableData();
        const Modifiers& modifiers = this->modifiers();
        if ((data.fStorage != Storage::kLocal && this->variableData().fReadCount) ||
            (modifiers.fFlags & (Modifiers::kIn_Flag | Modifiers::kOut_Flag |
                                 Modifiers::kUniform_Flag | Modifiers::kVarying_Flag))) {
            return false;
        }
        return !data.fWriteCount ||
               (!data.fReadCount && !(modifiers.fFlags & (Modifiers::kPLS_Flag |
                                                          Modifiers::kPLSOut_Flag)));
    }

private:
    void referenceCreated(VariableReference::RefKind refKind) const {
        if (refKind != VariableReference::RefKind::kRead) {
            ++this->variableData().fWriteCount;
        }
        if (refKind != VariableReference::RefKind::kWrite) {
            ++this->variableData().fReadCount;
        }
    }

    void referenceDestroyed(VariableReference::RefKind refKind) const {
        if (refKind != VariableReference::RefKind::kRead) {
            --this->variableData().fWriteCount;
        }
        if (refKind != VariableReference::RefKind::kWrite) {
            --this->variableData().fReadCount;
        }
    }

    using INHERITED = Symbol;

    friend class VariableReference;
};

} // namespace SkSL

#endif
