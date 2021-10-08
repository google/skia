/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONSTRUCTOR_ARRAY
#define SKSL_CONSTRUCTOR_ARRAY

#include "src/sksl/ir/SkSLConstructor.h"

namespace SkSL {

/**
 * Represents the construction of an array type, such as "float[5](x, y, z, w, 1)".
 */
class ConstructorArray final : public MultiArgumentConstructor {
public:
    inline static constexpr Kind kExpressionKind = Kind::kConstructorArray;

    ConstructorArray(int line, const Type& type, ExpressionArray arguments)
        : INHERITED(line, kExpressionKind, &type, std::move(arguments)) {}

    // ConstructorArray::Convert will typecheck and create array-constructor expressions.
    // Reports errors via the ErrorReporter; returns null on error.
    static std::unique_ptr<Expression> Convert(const Context& context,
                                               int line,
                                               const Type& type,
                                               ExpressionArray args);

    // ConstructorArray::Make creates array-constructor expressions; errors reported via SkASSERT.
    static std::unique_ptr<Expression> Make(const Context& context,
                                            int line,
                                            const Type& type,
                                            ExpressionArray args);

    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<ConstructorArray>(fLine, this->type(), this->cloneArguments());
    }

private:
    using INHERITED = MultiArgumentConstructor;
};

}  // namespace SkSL

#endif
