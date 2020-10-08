/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FIELD
#define SKSL_FIELD

#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLModifiers.h"
#include "src/sksl/ir/SkSLSymbol.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVariable.h"

namespace SkSL {

/**
 * A symbol which should be interpreted as a field access. Fields are added to the symboltable
 * whenever a bare reference to an identifier should refer to a struct field; in GLSL, this is the
 * result of declaring anonymous interface blocks.
 */
class Field : public Symbol {
public:
    static constexpr Kind kSymbolKind = Kind::kField;

    Field(int offset, const Variable* owner, int fieldIndex)
    : INHERITED(offset, FieldData{owner->type().fields()[fieldIndex].fName,
                                  owner->type().fields()[fieldIndex].fType,
                                  owner,
                                  fieldIndex}) {}

    const Type& type() const override {
        return *this->fieldData().fType;
    }

    StringFragment name() const override {
        return this->fieldData().fName;
    }

    int fieldIndex() const {
        return this->fieldData().fFieldIndex;
    }

    const Variable& owner() const {
        return *this->fieldData().fOwner;
    }

    String description() const override {
        return this->owner().description() + "." + this->name();
    }

private:
    using INHERITED = Symbol;
};

} // namespace SkSL

#endif
