/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_VARIABLE
#define SKSL_VARIABLE

#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLModifiers.h"
#include "src/sksl/ir/SkSLSymbol.h"
#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

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
//        SkASSERT(!fReadCount && !fWriteCount);
    }

#ifdef SKSL_STANDALONE
    String constructionCode() const override {
        // For everything other than enums, We leave initialValue null here and fill it in later
        // when we reach the VarDeclaration
        String initialValue;
        if (fType.kind() == Type::kEnum_Kind && fInitialValue) {
            initialValue = "(Expression*) symbols->takeOwnership(std::unique_ptr<Expression>(" +
                           fInitialValue->constructionCode() + "))";
        } else {
            initialValue = "nullptr";
        }
        return String::printf("new Variable(-1, %s, \"%s\", *%s, (Variable::Storage) %d, %s)",
                              fModifiers.constructionCode().c_str(),
                              String(fName).c_str(), SymbolWriter::symbolCode(fType).c_str(),
                              fStorage, initialValue.c_str());
    }
#endif

    virtual String description() const override {
        return fModifiers.description() + fType.fName + " " + fName;
    }

    bool dead() const {
        if ((fStorage != kLocal_Storage && fReadCount) ||
            (fModifiers.fFlags & (Modifiers::kIn_Flag | Modifiers::kOut_Flag |
                                 Modifiers::kUniform_Flag | Modifiers::kVarying_Flag))) {
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
