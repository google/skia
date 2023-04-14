/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONSTRUCTOR_COMPOUND_CAST
#define SKSL_CONSTRUCTOR_COMPOUND_CAST

#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLIRNode.h"

#include <memory>
#include <utility>

namespace SkSL {

class Context;
class Type;

/**
 * Represents the construction of a vector/matrix typecast, such as `half3(myInt3)` or
 * `float4x4(myHalf4x4)`. Matrix resizes are done in ConstructorMatrixResize, not here.
 *
 * These always contain exactly 1 vector or matrix of matching size, and are never constant.
 */
class ConstructorCompoundCast final : public SingleArgumentConstructor {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kConstructorCompoundCast;

    ConstructorCompoundCast(Position pos, const Type& type, std::unique_ptr<Expression> arg)
        : INHERITED(pos, kIRNodeKind, &type, std::move(arg)) {}

    static std::unique_ptr<Expression> Make(const Context& context,
                                            Position pos,
                                            const Type& type,
                                            std::unique_ptr<Expression> arg);

    std::unique_ptr<Expression> clone(Position pos) const override {
        return std::make_unique<ConstructorCompoundCast>(pos, this->type(), argument()->clone());
    }

private:
    using INHERITED = SingleArgumentConstructor;
};

}  // namespace SkSL

#endif
