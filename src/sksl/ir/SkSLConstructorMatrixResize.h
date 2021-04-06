/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONSTRUCTOR_MATRIX_RESIZE
#define SKSL_CONSTRUCTOR_MATRIX_RESIZE

#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFloatLiteral.h"

#include <memory>

namespace SkSL {

/**
 * Represents the construction of a matrix resize operation, such as `mat4x4(myMat2x2)`.
 *
 * These always contain exactly 1 matrix of non-matching size. Cells that aren't present in the
 * input matrix are populated with the identity matrix.
 */
class ConstructorMatrixResize final : public SingleArgumentConstructor {
public:
    static constexpr Kind kExpressionKind = Kind::kConstructorMatrixResize;

    ConstructorMatrixResize(int offset, const Type& type, std::unique_ptr<Expression> arg)
            : INHERITED(offset, kExpressionKind, &type, std::move(arg))
            , fZeroLiteral(offset, /*value=*/0.0f, &type.componentType())
            , fOneLiteral(offset, /*value=*/1.0f, &type.componentType()) {}

    static std::unique_ptr<Expression> Make(const Context& context,
                                            int offset,
                                            const Type& type,
                                            std::unique_ptr<Expression> arg);

    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<ConstructorMatrixResize>(fOffset, this->type(),
                                                         argument()->clone());
    }

    const Expression* getConstantSubexpression(int n) const override;

private:
    using INHERITED = SingleArgumentConstructor;
    const FloatLiteral fZeroLiteral;
    const FloatLiteral fOneLiteral;
};

}  // namespace SkSL

#endif
