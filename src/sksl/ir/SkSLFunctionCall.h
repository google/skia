/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FUNCTIONCALL
#define SKSL_FUNCTIONCALL

#include "include/private/SkTArray.h"
#include "src/sksl/SkSLIntrinsicList.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"

namespace SkSL {

// Make an enum holding every intrinsic supported by SkSL.
#define SKSL_INTRINSIC(name) k_##name##_IntrinsicKind,
enum IntrinsicKind {
    kNotIntrinsic = -1,
    SKSL_INTRINSIC_LIST
};
#undef SKSL_INTRINSIC

/**
 * A function invocation.
 */
class FunctionCall final : public Expression {
public:
    static constexpr Kind kExpressionKind = Kind::kFunctionCall;

    FunctionCall(int offset, const Type* type, const FunctionDeclaration* function,
                 ExpressionArray arguments, IntrinsicKind intrinsicKind)
        : INHERITED(offset, kExpressionKind, type)
        , fFunction(*function)
        , fArguments(std::move(arguments))
        , fIntrinsicKind(intrinsicKind) {}

    // Resolves generic types, performs type conversion on arguments, determines return type, and
    // reports errors via the ErrorReporter.
    static std::unique_ptr<Expression> Convert(const Context& context,
                                               int offset,
                                               const FunctionDeclaration& function,
                                               ExpressionArray arguments);

    // Creates the function call; reports errors via ASSERT.
    static std::unique_ptr<Expression> Make(const Context& context,
                                            int offset,
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

    IntrinsicKind intrinsicKind() const {
        return fIntrinsicKind;
    }

    bool isIntrinsic() const {
        return this->intrinsicKind() != kNotIntrinsic;
    }

    bool hasProperty(Property property) const override;

    std::unique_ptr<Expression> clone() const override;

    String description() const override;

private:
    const FunctionDeclaration& fFunction;
    ExpressionArray fArguments;
    IntrinsicKind fIntrinsicKind = kNotIntrinsic;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
