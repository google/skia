/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONSTRUCTOR_SCALAR_CAST
#define SKSL_CONSTRUCTOR_SCALAR_CAST

#include "include/private/SkSLDefines.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLExpression.h"

#include <memory>

namespace SkSL {

/**
 * Represents the construction of a scalar cast, such as `float(intVariable)`.
 *
 * These always contain exactly 1 scalar of a differing type, and are never constant.
 */
class ConstructorScalarCast final : public SingleArgumentConstructor {
public:
    static constexpr Kind kExpressionKind = Kind::kConstructorScalarCast;

    ConstructorScalarCast(int line, const Type& type, std::unique_ptr<Expression> arg)
        : INHERITED(line, kExpressionKind, &type, std::move(arg)) {}

    // ConstructorScalarCast::Convert will typecheck and create scalar-constructor expressions.
    // Reports errors via the ErrorReporter; returns null on error.
    static std::unique_ptr<Expression> Convert(const Context& context,
                                               int line,
                                               const Type& rawType,
                                               ExpressionArray args);

    // ConstructorScalarCast::Make casts a scalar expression. Casts that can be evaluated at
    // compile-time will do so (e.g. `int(4.1)` --> `Literal(int 4)`). Errors reported via SkASSERT.
    static std::unique_ptr<Expression> Make(const Context& context,
                                            int line,
                                            const Type& type,
                                            std::unique_ptr<Expression> arg);

    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<ConstructorScalarCast>(fLine, this->type(), argument()->clone());
    }

    bool isCompileTimeConstant() const override {
        // If this were a compile-time constant, we would have created a literal instead.
        return false;
    }

private:
    using INHERITED = SingleArgumentConstructor;
};

}  // namespace SkSL

#endif
