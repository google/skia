/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_NULLLITERAL
#define SKSL_NULLLITERAL

#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * Represents 'null'.
 */
struct NullLiteral : public Expression {
    static constexpr Kind kIRNodeKind = kNullLiteral_Kind;

    NullLiteral(const Context& context, int offset)
    : INHERITED(offset, kIRNodeKind, *context.fNull_Type) {}

    NullLiteral(int offset, const Type& type)
    : INHERITED(offset, kIRNodeKind, type) {}

    String description() const override {
        return "null";
    }

    bool hasProperty(Property property) const override {
        return false;
    }

    bool isCompileTimeConstant() const override {
        return true;
    }

    bool compareConstant(const Context& context, const Expression& other) const override {
        return true;
    }

    std::unique_ptr<IRNode> clone() const override {
        return std::unique_ptr<IRNode>(new NullLiteral(fOffset, fType));
    }

    typedef Expression INHERITED;
};

}  // namespace SkSL

#endif
