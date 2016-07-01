/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_FIELDACCESS
#define SKSL_FIELDACCESS

#include "SkSLExpression.h"
#include "SkSLUtil.h"

namespace SkSL {

/**
 * An expression which extracts a field from a struct, as in 'foo.bar'.
 */
struct FieldAccess : public Expression {
    FieldAccess(std::unique_ptr<Expression> base, int fieldIndex)
    : INHERITED(base->fPosition, kFieldAccess_Kind, base->fType->fields()[fieldIndex].fType)
    , fBase(std::move(base))
    , fFieldIndex(fieldIndex) {}

    virtual std::string description() const override {
        return fBase->description() + "." + fBase->fType->fields()[fFieldIndex].fName;
    }

    const std::unique_ptr<Expression> fBase;
    const int fFieldIndex;

    typedef Expression INHERITED;
};

} // namespace

#endif
