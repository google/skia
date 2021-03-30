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
#include "src/sksl/ir/SkSLExpression.h"

#include <memory>

namespace SkSL {

/**
 * Represents the construction of a diagonal matrix, such as `half3x3(n)`.
 *
 * These always contain exactly 1 scalar.
 */
class ConstructorDiagonalMatrix final : public Expression {
public:
    static constexpr Kind kExpressionKind = Kind::kConstructorDiagonalMatrix;

    ConstructorDiagonalMatrix(int offset, const Type& type, std::unique_ptr<Expression> arg)
        : INHERITED(offset, kExpressionKind, &type)
        , fArgument(std::move(arg)) {}

    static std::unique_ptr<Expression> Make(const Context& context,
                                            int offset,
                                            const Type& type,
                                            std::unique_ptr<Expression> arg);

    std::unique_ptr<Expression>& argument() {
        return fArgument;
    }

    const std::unique_ptr<Expression>& argument() const {
        return fArgument;
    }

    bool hasProperty(Property property) const override {
        return argument()->hasProperty(property);
    }

    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<ConstructorDiagonalMatrix>(fOffset, this->type(),
                                                           argument()->clone());
    }

    String description() const override {
        return this->type().description() + "(" + argument()->description() + ")";
    }

    const Type& componentType() const {
        return this->argument()->type();
    }

    bool isCompileTimeConstant() const override {
        return argument()->isCompileTimeConstant();
    }

    bool isConstantOrUniform() const override {
        return argument()->isConstantOrUniform();
    }

    ComparisonResult compareConstant(const Expression& other) const override;

    SKSL_FLOAT getMatComponent(int col, int row) const override;

private:
    std::unique_ptr<Expression> fArgument;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
