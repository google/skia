/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONSTRUCTOR_MATRIX_RESIZE
#define SKSL_CONSTRUCTOR_MATRIX_RESIZE

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
 * Represents the construction of a matrix resize operation, such as `mat4x4(myMat2x2)`.
 *
 * These always contain exactly 1 matrix of non-matching size. Cells that aren't present in the
 * input matrix are populated with the identity matrix.
 */
class ConstructorMatrixResize final : public SingleArgumentConstructor {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kConstructorMatrixResize;

    ConstructorMatrixResize(Position pos, const Type& type, std::unique_ptr<Expression> arg)
            : INHERITED(pos, kIRNodeKind, &type, std::move(arg)) {}

    static std::unique_ptr<Expression> Make(const Context& context,
                                            Position pos,
                                            const Type& type,
                                            std::unique_ptr<Expression> arg);

    std::unique_ptr<Expression> clone(Position pos) const override {
        return std::make_unique<ConstructorMatrixResize>(pos, this->type(), argument()->clone());
    }

    bool supportsConstantValues() const override { return true; }
    std::optional<double> getConstantValue(int n) const override;

private:
    using INHERITED = SingleArgumentConstructor;
};

}  // namespace SkSL

#endif
