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

    FunctionCall(Position pos,
                 const Type* type,
                 const FunctionDeclaration* function,
                 ExpressionArray arguments,
                 const FunctionCall* stablePointer)
            : INHERITED(pos, kIRNodeKind, type)
            , fFunction(*function)
            , fArguments(std::move(arguments))
            , fStablePointer(stablePointer ? stablePointer : this) {}

    // Resolves generic types, performs type conversion on arguments, determines return type, and
    // chooses a unique stable ID. Reports errors via the ErrorReporter.
    static std::unique_ptr<Expression> Convert(const Context& context,
                                               Position pos,
                                               const FunctionDeclaration& function,
                                               ExpressionArray arguments);

    static std::unique_ptr<Expression> Convert(const Context& context,
                                               Position pos,
                                               std::unique_ptr<Expression> functionValue,
                                               ExpressionArray arguments);

    // Creates a function call with a given stable ID; reports errors via ASSERT.
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

    const FunctionCall* stablePointer() const {
        return fStablePointer;
    }

    std::unique_ptr<Expression> clone(Position pos) const override;

    std::string description(OperatorPrecedence) const override;

private:
    const FunctionDeclaration& fFunction;
    ExpressionArray fArguments;

    // The stable pointer uniquely identifies this FunctionCall across an entire SkSL program.
    // This allows us to clone() a FunctionCall but still find that call in a hash-map.
    const FunctionCall* fStablePointer = nullptr;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
