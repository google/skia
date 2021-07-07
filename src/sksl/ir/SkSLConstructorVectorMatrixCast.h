/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONSTRUCTOR_VECTOR_MATRIX_CAST
#define SKSL_CONSTRUCTOR_VECTOR_MATRIX_CAST

#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLExpression.h"

#include <memory>

namespace SkSL {

/**
 * Represents a cast from a 4-element vector to a 2x2 matrix, or vice-versa.
 *
 * These always contain exactly one four-slot vector or matrix, and are never constant.
 */
class ConstructorVectorMatrixCast final : public SingleArgumentConstructor {
public:
    static constexpr Kind kExpressionKind = Kind::kConstructorVectorMatrixCast;

    ConstructorVectorMatrixCast(int offset, const Type& type, std::unique_ptr<Expression> arg)
        : INHERITED(offset, kExpressionKind, &type, std::move(arg)) {}

    static std::unique_ptr<Expression> Make(const Context& context,
                                            int offset,
                                            const Type& type,
                                            std::unique_ptr<Expression> arg);

    bool isCompileTimeConstant() const override {
        // If this were a compile-time constant, we would have made a ConstructorCompound instead.
        return false;
    }

    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<ConstructorVectorMatrixCast>(fOffset, this->type(),
                                                             argument()->clone());
    }

private:
    using INHERITED = SingleArgumentConstructor;
};

}  // namespace SkSL

#endif
