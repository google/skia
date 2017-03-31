/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_VARIABLE
#define SKSL_VARIABLE

#include "SkSLModifiers.h"
#include "SkSLPosition.h"
#include "SkSLSymbol.h"
#include "SkSLType.h"

namespace SkSL {

/**
 * Represents a variable, whether local, global, or a function parameter. This represents the
 * variable itself (the storage location), which is shared between all VariableReferences which
 * read or write that storage location.
 */
struct Variable : public Symbol {
    enum Storage {
        kGlobal_Storage,
        kLocal_Storage,
        kParameter_Storage
    };

    Variable(Position position, Modifiers modifiers, String name, const Type& type,
             Storage storage)
    : INHERITED(position, kVariable_Kind, std::move(name))
    , fModifiers(modifiers)
    , fType(type)
    , fStorage(storage)
    , fReadCount(0)
    , fWriteCount(0) {}

    virtual String description() const override {
        return fModifiers.description() + fType.fName + " " + fName;
    }

    mutable Modifiers fModifiers;
    const Type& fType;
    const Storage fStorage;

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
