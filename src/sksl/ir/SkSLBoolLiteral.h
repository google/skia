/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_BOOLLITERAL
#define SKSL_BOOLLITERAL

#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * Represents 'true' or 'false'.
 */
struct BoolLiteral : public Expression {
    BoolLiteral(const Context& context, int offset, bool value)
    : INHERITED(offset, kBoolLiteral_Kind, *context.fBool_Type)
    , fValue(value) {}

#ifdef SK_DEBUG
    String description() const override {
        return String(fValue ? "true" : "false");
    }
#endif

    bool hasProperty(Property property) const override {
        return false;
    }

    bool isConstant() const override {
        return true;
    }

    bool compareConstant(const Context& context, const Expression& other) const override {
        BoolLiteral& b = (BoolLiteral&) other;
        return fValue == b.fValue;
    }

    std::unique_ptr<Expression> clone() const override {
        return std::unique_ptr<Expression>(new BoolLiteral(fOffset, fValue, &fType));
    }

    const bool fValue;

    typedef Expression INHERITED;

private:
    BoolLiteral(int offset, bool value, const Type* type)
    : INHERITED(offset, kBoolLiteral_Kind, *type)
    , fValue(value) {}
};

} // namespace

#endif
