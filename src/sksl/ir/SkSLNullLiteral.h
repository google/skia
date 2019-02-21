/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_NULLLITERAL
#define SKSL_NULLLITERAL

#include "SkSLContext.h"
#include "SkSLExpression.h"

namespace SkSL {

/**
 * Represents 'null'.
 */
struct NullLiteral : public Expression {
    NullLiteral(const Context& context, int offset)
    : INHERITED(offset, kNullLiteral_Kind, *context.fNull_Type) {}

    NullLiteral(int offset, const Type& type)
    : INHERITED(offset, kNullLiteral_Kind, type) {}

    String description() const override {
        return "null";
    }

    bool hasSideEffects() const override {
        return false;
    }

    bool isConstant() const override {
        return true;
    }

    bool compareConstant(const Context& context, const Expression& other) const override {
        return true;
    }

    std::unique_ptr<Expression> clone() const override {
        return std::unique_ptr<Expression>(new NullLiteral(fOffset, fType));
    }

    typedef Expression INHERITED;
};

} // namespace

#endif
