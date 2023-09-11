/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONSTRUCTOR_ARRAY_CAST
#define SKSL_CONSTRUCTOR_ARRAY_CAST

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
 * Represents the typecasting of an array. Arrays cannot be directly casted in SkSL (or GLSL), but
 * type narrowing can cause an array to be implicitly casted. For instance, the expression
 * `myHalf2Array == float[2](a, b)` should be allowed when narrowing conversions are enabled; this
 * constructor allows the necessary array-type conversion to be represented in IR.
 *
 * These always contain exactly 1 array of matching size, and are never constant.
 */
class ConstructorArrayCast final : public SingleArgumentConstructor {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kConstructorArrayCast;

    ConstructorArrayCast(Position pos, const Type& type, std::unique_ptr<Expression> arg)
        : INHERITED(pos, kIRNodeKind, &type, std::move(arg)) {}

    static std::unique_ptr<Expression> Make(const Context& context,
                                            Position pos,
                                            const Type& type,
                                            std::unique_ptr<Expression> arg);

    std::unique_ptr<Expression> clone(Position pos) const override {
        return std::make_unique<ConstructorArrayCast>(pos, this->type(), argument()->clone());
    }

private:
    using INHERITED = SingleArgumentConstructor;
};

}  // namespace SkSL

#endif
