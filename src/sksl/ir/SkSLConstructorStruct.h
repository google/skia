/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONSTRUCTOR_STRUCT
#define SKSL_CONSTRUCTOR_STRUCT

#include "src/sksl/ir/SkSLConstructor.h"

namespace SkSL {

/**
 * Represents the construction of an struct object, such as "Color(red, green, blue, 1)".
 */
class ConstructorStruct final : public MultiArgumentConstructor {
public:
    inline static constexpr Kind kExpressionKind = Kind::kConstructorStruct;

    ConstructorStruct(int line, const Type& type, ExpressionArray arguments)
        : INHERITED(line, kExpressionKind, &type, std::move(arguments)) {}

    // ConstructorStruct::Convert will typecheck and create struct-constructor expressions.
    // Reports errors via the ErrorReporter; returns null on error.
    static std::unique_ptr<Expression> Convert(const Context& context,
                                               int line,
                                               const Type& type,
                                               ExpressionArray args);

    // ConstructorStruct::Make creates struct-constructor expressions; errors reported via SkASSERT.
    static std::unique_ptr<Expression> Make(const Context& context,
                                            int line,
                                            const Type& type,
                                            ExpressionArray args);

    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<ConstructorStruct>(fLine, this->type(), this->cloneArguments());
    }

private:
    using INHERITED = MultiArgumentConstructor;
};

}  // namespace SkSL

#endif
