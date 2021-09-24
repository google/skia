/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FUNCTIONCALL
#define SKSL_FUNCTIONCALL

#include "include/private/SkTArray.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"

namespace SkSL {

/**
 * A function invocation.
 */
class FunctionCall final : public Expression {
public:
    static constexpr Kind kExpressionKind = Kind::kFunctionCall;

    FunctionCall(int line, const Type* type, const FunctionDeclaration* function,
                 ExpressionArray arguments)
        : INHERITED(line, kExpressionKind, type)
        , fFunction(*function)
        , fArguments(std::move(arguments)) {}

    // Resolves generic types, performs type conversion on arguments, determines return type, and
    // reports errors via the ErrorReporter.
    static std::unique_ptr<Expression> Convert(const Context& context,
                                               int line,
                                               const FunctionDeclaration& function,
                                               ExpressionArray arguments);

    // Creates the function call; reports errors via ASSERT.
    static std::unique_ptr<Expression> Make(const Context& context,
                                            int line,
                                            const Type* returnType,
                                            const FunctionDeclaration& function,
                                            ExpressionArray arguments);

    const FunctionDeclaration& function() const {
        return fFunction;
    }

    ExpressionArray& arguments() {
        return fArguments;
    }

    const ExpressionArray& arguments() const {
        return fArguments;
    }

    bool hasProperty(Property property) const override;

    std::unique_ptr<Expression> clone() const override;

    String description() const override;

private:
    const FunctionDeclaration& fFunction;
    ExpressionArray fArguments;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
