/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONSTRUCTOR_VECTOR
#define SKSL_CONSTRUCTOR_VECTOR

#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLExpression.h"

#include <memory>

namespace SkSL {

/**
 * Represents the construction of a vector, such as `half3(pos.xy, 1)`.
 *
 * These can contain a mix of scalars and vectors; the total number of scalar values inside the
 * constructor must always match the type width. (e.g. `pos.xy` counts for two scalars)
 * The inner values must have the same component type as the vector.
 */
class ConstructorVector final : public MultiArgumentConstructor {
public:
    static constexpr Kind kExpressionKind = Kind::kConstructorVector;

    ConstructorVector(int offset, const Type& type, ExpressionArray args)
            : INHERITED(offset, kExpressionKind, &type, std::move(args)) {}

    static std::unique_ptr<Expression> Make(const Context& context,
                                            int offset,
                                            const Type& type,
                                            ExpressionArray args);

    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<ConstructorVector>(fOffset, this->type(), this->cloneArguments());
    }

private:
    using INHERITED = MultiArgumentConstructor;
};

}  // namespace SkSL

#endif
