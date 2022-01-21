/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONSTRUCTOR_COMPOUND
#define SKSL_CONSTRUCTOR_COMPOUND

#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLExpression.h"

#include <memory>

namespace SkSL {

/**
 * Represents a vector or matrix that is composed from other expressions, such as
 * `half3(pos.xy, 1)` or `mat3(a.xyz, b.xyz, 0, 0, 1)`
 *
 * These can contain a mix of scalars and aggregates. The total number of scalar values inside the
 * constructor must always match the type's slot count. (e.g. `pos.xy` consumes two slots.)
 * The inner values must have the same component type as the vector/matrix.
 */
class ConstructorCompound final : public MultiArgumentConstructor {
public:
    inline static constexpr Kind kExpressionKind = Kind::kConstructorCompound;

    ConstructorCompound(int line, const Type& type, ExpressionArray args)
            : INHERITED(line, kExpressionKind, &type, std::move(args)) {}

    static std::unique_ptr<Expression> Make(const Context& context,
                                            int line,
                                            const Type& type,
                                            ExpressionArray args);

    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<ConstructorCompound>(fLine, this->type(),
                                                     this->arguments().clone());
    }

private:
    using INHERITED = MultiArgumentConstructor;
};

}  // namespace SkSL

#endif
