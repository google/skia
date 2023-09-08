/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FUNCTIONCALL
#define SKSL_FUNCTIONCALL

#include "src/sksl/SkSLDefines.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLIRNode.h"

#include <cstdint>
#include <memory>
#include <string>
#include <utility>

namespace SkSL {

class Context;
class FunctionDeclaration;
class Type;
enum class OperatorPrecedence : uint8_t;

/**
 * A function invocation.
 */
class FunctionCall final : public Expression {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kFunctionCall;

    FunctionCall(Position pos, const Type* type, const FunctionDeclaration* function,
                 ExpressionArray arguments)
        : INHERITED(pos, kIRNodeKind, type)
        , fFunction(*function)
        , fArguments(std::move(arguments)) {}

    // Resolves generic types, performs type conversion on arguments, determines return type, and
    // reports errors via the ErrorReporter.
    static std::unique_ptr<Expression> Convert(const Context& context,
                                               Position pos,
                                               const FunctionDeclaration& function,
                                               ExpressionArray arguments);

    static std::unique_ptr<Expression> Convert(const Context& context,
                                               Position pos,
                                               std::unique_ptr<Expression> functionValue,
                                               ExpressionArray arguments);

    // Creates the function call; reports errors via ASSERT.
    static std::unique_ptr<Expression> Make(const Context& context,
                                            Position pos,
                                            const Type* returnType,
                                            const FunctionDeclaration& function,
                                            ExpressionArray arguments);

    static const FunctionDeclaration* FindBestFunctionForCall(const Context& context,
                                                              const FunctionDeclaration* overloads,
                                                              const ExpressionArray& arguments);

    const FunctionDeclaration& function() const {
        return fFunction;
    }

    ExpressionArray& arguments() {
        return fArguments;
    }

    const ExpressionArray& arguments() const {
        return fArguments;
    }

    std::unique_ptr<Expression> clone(Position pos) const override;

    std::string description(OperatorPrecedence) const override;

private:
    const FunctionDeclaration& fFunction;
    ExpressionArray fArguments;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
