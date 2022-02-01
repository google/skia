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
#include "src/sksl/ir/SkSLLiteral.h"

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
    inline static constexpr Kind kExpressionKind = Kind::kConstructorMatrixResize;

    ConstructorMatrixResize(int line, const Type& type, std::unique_ptr<Expression> arg)
            : INHERITED(line, kExpressionKind, &type, std::move(arg)) {}

    static std::unique_ptr<Expression> Make(const Context& context,
                                            int line,
                                            const Type& type,
                                            std::unique_ptr<Expression> arg);

    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<ConstructorMatrixResize>(fLine, this->type(),
                                                         argument()->clone());
    }

    bool supportsConstantValues() const override { return true; }
    skstd::optional<double> getConstantValue(int n) const override;

private:
    using INHERITED = SingleArgumentConstructor;
};

}  // namespace SkSL

#endif
