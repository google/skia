/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ir/SkSLSymbol.h"

namespace SkSL {

class String;
class Type;

class ExternalValue : public Symbol {
public:
    // if the value does not have a type (i.e. is neither readable nor writable), use fVoid_Type
    ExternalValue(const char* name, const Type& type)
        : INHERITED(-1, kExternal_Kind, name)
        , fType(type) {}

    virtual bool canRead() const {
        return false;
    }

    virtual bool canWrite() const {
        return false;
    }

    /**
     * Reads the external value and stores the resulting data in target. The caller must ensure
     * that target is a valid pointer to a region of sufficient size to hold the data contained
     * in this external value.
     */
    virtual void read(void* target) {
        SkASSERT(false);
    }

    /**
     * Copies the value in src into this external value. The caller must ensure that src is a
     * pointer to the type of data expected by this external value.
     */
    virtual void write(void* src) {
        SkASSERT(false);
    }

    const Type& type() const {
        return fType;
    }

    /**
     * Resolves 'name' within this context and returns an ExternalValue which represents it, or
     * null if no such child exists. If the implementation of this method creates new
     * ExternalValues and there isn't a more convenient place for ownership of the objects to
     * reside, the compiler's takeOwnership method may be useful.
     *
     * The 'name' string may not persist after this call; do not store this pointer.
     */
    virtual ExternalValue* getChild(const char* name) const {
        return nullptr;
    }

    String description() const override {
        return String("external<") + fName + ">";
    }

private:
    typedef Symbol INHERITED;

    const Type& fType;
};

} // namespace
