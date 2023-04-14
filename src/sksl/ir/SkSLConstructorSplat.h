/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONSTRUCTOR_SPLAT
#define SKSL_CONSTRUCTOR_SPLAT

#include "include/core/SkTypes.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLType.h"

#include <memory>
#include <optional>
#include <utility>

namespace SkSL {

class Context;

/**
 * Represents the construction of a vector splat, such as `half3(n)`.
 *
 * These always contain exactly 1 scalar.
 */
class ConstructorSplat final : public SingleArgumentConstructor {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kConstructorSplat;

    ConstructorSplat(Position pos, const Type& type, std::unique_ptr<Expression> arg)
        : INHERITED(pos, kIRNodeKind, &type, std::move(arg)) {}

    // The input argument must be scalar. A "splat" to a scalar type will be optimized into a no-op.
    static std::unique_ptr<Expression> Make(const Context& context,
                                            Position pos,
                                            const Type& type,
                                            std::unique_ptr<Expression> arg);

    std::unique_ptr<Expression> clone(Position pos) const override {
        return std::make_unique<ConstructorSplat>(pos, this->type(), argument()->clone());
    }

    bool supportsConstantValues() const override {
        return true;
    }

    std::optional<double> getConstantValue(int n) const override {
        SkASSERT(n >= 0 && n < this->type().columns());
        return this->argument()->getConstantValue(0);
    }

private:
    using INHERITED = SingleArgumentConstructor;
};

}  // namespace SkSL

#endif
