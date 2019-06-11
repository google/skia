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

namespace SkSL {

struct Expression;

/**
 * Represents a variable, whether local, global, or a function parameter. This represents the
 * variable itself (the storage location), which is shared between all VariableReferences which
 * read or write that storage location.
 */
struct Variable : public Symbol {
    enum Storage {
        kGlobal_Storage,
        kInterfaceBlock_Storage,
        kLocal_Storage,
        kParameter_Storage
    };

    Variable(int offset, Modifiers modifiers, StringFragment name, const Type& type,
             Storage storage, Expression* initialValue = nullptr)
    : INHERITED(offset, kVariable_Kind, name)
    , fModifiers(modifiers)
    , fType(type)
    , fStorage(storage)
    , fInitialValue(initialValue)
    , fReadCount(0)
    , fWriteCount(initialValue ? 1 : 0) {}

    ~Variable() override {
        // can't destroy a variable while there are remaining references to it
        if (fInitialValue) {
            --fWriteCount;
        }
        SkASSERT(!fReadCount && !fWriteCount);
    }

    virtual String description() const override {
        return fModifiers.description() + fType.fName + " " + fName;
    }

    bool dead() const {
        if ((fStorage != kLocal_Storage && fReadCount) ||
            (fModifiers.fFlags & (Modifiers::kIn_Flag | Modifiers::kOut_Flag |
                                 Modifiers::kUniform_Flag))) {
            return false;
        }
        return !fWriteCount ||
               (!fReadCount && !(fModifiers.fFlags & (Modifiers::kPLS_Flag |
                                                      Modifiers::kPLSOut_Flag)));
    }

    mutable Modifiers fModifiers;
    const Type& fType;
    const Storage fStorage;

    Expression* fInitialValue = nullptr;

    // Tracks how many sites read from the variable. If this is zero for a non-out variable (or
    // becomes zero during optimization), the variable is dead and may be eliminated.
    mutable int fReadCount;
    // Tracks how many sites write to the variable. If this is zero, the variable is dead and may be
    // eliminated.
    mutable int fWriteCount;

    typedef Symbol INHERITED;
};

} // namespace SkSL

#endif
