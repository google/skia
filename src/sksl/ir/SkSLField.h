/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FIELD
#define SKSL_FIELD

#include "include/private/SkSLModifiers.h"
#include "include/private/SkSLSymbol.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVariable.h"

namespace SkSL {

/**
 * A symbol which should be interpreted as a field access. Fields are added to the symboltable
 * whenever a bare reference to an identifier should refer to a struct field; in GLSL, this is the
 * result of declaring anonymous interface blocks.
 */
class Field final : public Symbol {
public:
    static constexpr Kind kSymbolKind = Kind::kField;

    Field(int line, const Variable* owner, int fieldIndex)
        : INHERITED(line, kSymbolKind, owner->type().fields()[fieldIndex].fName,
                    owner->type().fields()[fieldIndex].fType)
        , fOwner(owner)
        , fFieldIndex(fieldIndex) {}

    int fieldIndex() const {
        return fFieldIndex;
    }

    const Variable& owner() const {
        return *fOwner;
    }

    String description() const override {
        return this->owner().description() + "." + this->name();
    }

private:
    const Variable* fOwner;
    int fFieldIndex;

    using INHERITED = Symbol;
};

} // namespace SkSL

#endif
