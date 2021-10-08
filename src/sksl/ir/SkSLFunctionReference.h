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
class FunctionReference final : public Expression {
public:
    inline static constexpr Kind kExpressionKind = Kind::kFunctionReference;

    FunctionReference(const Context& context, int line,
                      std::vector<const FunctionDeclaration*> functions)
        : INHERITED(line, kExpressionKind, context.fTypes.fInvalid.get())
        , fFunctions(std::move(functions)) {}

    const std::vector<const FunctionDeclaration*>& functions() const {
        return fFunctions;
    }

    bool hasProperty(Property property) const override {
        return false;
    }

    std::unique_ptr<Expression> clone() const override {
        return std::unique_ptr<Expression>(new FunctionReference(fLine, this->functions(),
                                                                 &this->type()));
    }

    String description() const override {
        return String("<function>");
    }

private:
    FunctionReference(int line, std::vector<const FunctionDeclaration*> functions,
                      const Type* type)
        : INHERITED(line, kExpressionKind, type)
        , fFunctions(std::move(functions)) {}

    std::vector<const FunctionDeclaration*> fFunctions;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
