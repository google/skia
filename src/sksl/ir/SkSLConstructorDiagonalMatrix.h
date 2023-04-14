/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONSTRUCTOR_DIAGONAL_MATRIX
#define SKSL_CONSTRUCTOR_DIAGONAL_MATRIX

#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLIRNode.h"

#include <memory>
#include <optional>
#include <utility>

namespace SkSL {

class Context;
class Type;

/**
 * Represents the construction of a diagonal matrix, such as `half3x3(n)`.
 *
 * These always contain exactly 1 scalar.
 */
class ConstructorDiagonalMatrix final : public SingleArgumentConstructor {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kConstructorDiagonalMatrix;

    ConstructorDiagonalMatrix(Position pos, const Type& type, std::unique_ptr<Expression> arg)
        : INHERITED(pos, kIRNodeKind, &type, std::move(arg)) {}

    static std::unique_ptr<Expression> Make(const Context& context,
                                            Position pos,
                                            const Type& type,
                                            std::unique_ptr<Expression> arg);

    std::unique_ptr<Expression> clone(Position pos) const override {
        return std::make_unique<ConstructorDiagonalMatrix>(pos, this->type(), argument()->clone());
    }

    bool supportsConstantValues() const override { return true; }
    std::optional<double> getConstantValue(int n) const override;

private:
    using INHERITED = SingleArgumentConstructor;
};

}  // namespace SkSL

#endif
