/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FUNCTIONREFERENCE
#define SKSL_FUNCTIONREFERENCE

#include "SkSLContext.h"
#include "SkSLExpression.h"
#include "SkSLFunctionDeclaration.h"

namespace SkSL {

/**
 * An identifier referring to a function name. This is an intermediate value: FunctionReferences are
 * always eventually replaced by FunctionCalls in valid programs.
 */
struct FunctionReference : public Expression {
    FunctionReference(const Context& context, Position position,
                      std::vector<const FunctionDeclaration*> function)
    : INHERITED(position, kFunctionReference_Kind, *context.fInvalid_Type)
    , fFunctions(function) {}

    bool hasSideEffects() const override {
        return false;
    }

    String description() const override {
        ASSERT(false);
        return String("<function>");
    }

    const std::vector<const FunctionDeclaration*> fFunctions;

    typedef Expression INHERITED;
};

} // namespace

#endif
