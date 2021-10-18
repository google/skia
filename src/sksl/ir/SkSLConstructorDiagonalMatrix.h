/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONSTRUCTOR_DIAGONAL_MATRIX
#define SKSL_CONSTRUCTOR_DIAGONAL_MATRIX

#include "include/private/SkSLDefines.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLLiteral.h"

#include <memory>

namespace SkSL {

/**
 * Represents the construction of a diagonal matrix, such as `half3x3(n)`.
 *
 * These always contain exactly 1 scalar.
 */
class ConstructorDiagonalMatrix final : public SingleArgumentConstructor {
public:
    inline static constexpr Kind kExpressionKind = Kind::kConstructorDiagonalMatrix;

    ConstructorDiagonalMatrix(int line, const Type& type, std::unique_ptr<Expression> arg)
        : INHERITED(line, kExpressionKind, &type, std::move(arg))
        , fZeroLiteral(line, /*value=*/0.0, &type.componentType()) {}

    static std::unique_ptr<Expression> Make(const Context& context,
                                            int line,
                                            const Type& type,
                                            std::unique_ptr<Expression> arg);

    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<ConstructorDiagonalMatrix>(fLine, this->type(),
                                                           argument()->clone());
    }

    bool allowsConstantSubexpressions() const override { return true; }
    const Expression* getConstantSubexpression(int n) const override;

private:
    const Literal fZeroLiteral;
    using INHERITED = SingleArgumentConstructor;
};

}  // namespace SkSL

#endif
