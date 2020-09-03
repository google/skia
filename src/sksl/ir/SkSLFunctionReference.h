/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FUNCTIONREFERENCE
#define SKSL_FUNCTIONREFERENCE

#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"

namespace SkSL {

/**
 * An identifier referring to a function name. This is an intermediate value: FunctionReferences are
 * always eventually replaced by FunctionCalls in valid programs.
 */
struct FunctionReference : public Expression {
    static constexpr Kind kExpressionKind = kFunctionReference_Kind;

    FunctionReference(const Context& context, int offset,
                      std::vector<const FunctionDeclaration*> function)
    : INHERITED(offset, kExpressionKind, *context.fInvalid_Type)
    , fFunctions(function) {}

    bool hasProperty(Property property) const override {
        return false;
    }

    std::unique_ptr<Expression> clone() const override {
        return std::unique_ptr<Expression>(new FunctionReference(fOffset, fFunctions, &fType));
    }

    String description() const override {
        return String("<function>");
    }

    const std::vector<const FunctionDeclaration*> fFunctions;

    using INHERITED = Expression;

private:
    FunctionReference(int offset, std::vector<const FunctionDeclaration*> function,
                      const Type* type)
    : INHERITED(offset, kExpressionKind, *type)
    , fFunctions(function) {}};

}  // namespace SkSL

#endif
