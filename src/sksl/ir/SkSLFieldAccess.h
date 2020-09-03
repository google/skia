/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FIELDACCESS
#define SKSL_FIELDACCESS

#include "src/sksl/SkSLUtil.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * An expression which extracts a field from a struct, as in 'foo.bar'.
 */
struct FieldAccess : public Expression {
    enum OwnerKind {
        kDefault_OwnerKind,
        // this field access is to a field of an anonymous interface block (and thus, the field name
        // is actually in global scope, so only the field name needs to be written in GLSL)
        kAnonymousInterfaceBlock_OwnerKind
    };

    static constexpr Kind kExpressionKind = kFieldAccess_Kind;

    FieldAccess(std::unique_ptr<Expression> base, int fieldIndex,
                OwnerKind ownerKind = kDefault_OwnerKind)
    : INHERITED(base->fOffset, kExpressionKind, *base->fType.fields()[fieldIndex].fType)
    , fBase(std::move(base))
    , fFieldIndex(fieldIndex)
    , fOwnerKind(ownerKind) {}

    bool hasProperty(Property property) const override {
        return fBase->hasProperty(property);
    }

    std::unique_ptr<Expression> clone() const override {
        return std::unique_ptr<Expression>(new FieldAccess(fBase->clone(), fFieldIndex,
                                                           fOwnerKind));
    }

    String description() const override {
        return fBase->description() + "." + fBase->fType.fields()[fFieldIndex].fName;
    }

    std::unique_ptr<Expression> fBase;
    const int fFieldIndex;
    const OwnerKind fOwnerKind;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
