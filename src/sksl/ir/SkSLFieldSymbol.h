/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FIELD
#define SKSL_FIELD

#include "src/sksl/ir/SkSLModifierFlags.h"
#include "src/sksl/ir/SkSLSymbol.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVariable.h"

namespace SkSL {

/**
 * A symbol which should be interpreted as a field access. Fields are added to the symboltable
 * whenever a bare reference to an identifier should refer to a struct field; in GLSL, this is the
 * result of declaring anonymous interface blocks.
 */
class FieldSymbol final : public Symbol {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kField;

    FieldSymbol(Position pos, const Variable* owner, int fieldIndex)
            : INHERITED(pos, kIRNodeKind, owner->type().fields()[fieldIndex].fName,
                        owner->type().fields()[fieldIndex].fType)
            , fOwner(owner)
            , fFieldIndex(fieldIndex) {}

    int fieldIndex() const {
        return fFieldIndex;
    }

    const Variable& owner() const {
        return *fOwner;
    }

    std::string description() const override {
        return this->owner().name().empty()
                       ? std::string(this->name())
                       : (this->owner().description() + "." + std::string(this->name()));
    }

private:
    const Variable* fOwner;
    int fFieldIndex;

    using INHERITED = Symbol;
};

} // namespace SkSL

#endif
