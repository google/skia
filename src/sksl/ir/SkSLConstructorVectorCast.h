/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONSTRUCTOR_VECTOR_CAST
#define SKSL_CONSTRUCTOR_VECTOR_CAST

#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLExpression.h"

#include <memory>

namespace SkSL {

/**
 * Represents the construction of a vector cast, such as `half3(myInt3)`.
 *
 * These always contain exactly 1 vector of matching size, and are never constant.
 */
class ConstructorVectorCast final : public SingleArgumentConstructor {
public:
    static constexpr Kind kExpressionKind = Kind::kConstructorVectorCast;

    ConstructorVectorCast(int offset, const Type& type, std::unique_ptr<Expression> arg)
        : INHERITED(offset, kExpressionKind, &type, std::move(arg)) {}

    static std::unique_ptr<Expression> Make(const Context& context,
                                            int offset,
                                            const Type& type,
                                            std::unique_ptr<Expression> arg);

    bool isCompileTimeConstant() const override {
        // If this were a compile-time constant, we would have made a ConstructorComposite instead.
        return false;
    }

    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<ConstructorVectorCast>(fOffset, this->type(), argument()->clone());
    }

private:
    using INHERITED = SingleArgumentConstructor;
};

}  // namespace SkSL

#endif
