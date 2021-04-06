/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLConstructorMatrixResize.h"

#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

std::unique_ptr<Expression> ConstructorMatrixResize::Make(const Context& context,
                                                          int offset,
                                                          const Type& type,
                                                          std::unique_ptr<Expression> arg) {
    SkASSERT(type.isMatrix());
    SkASSERT(arg->type().componentType() == type.componentType());

    // If the matrix isn't actually changing size, return it as-is.
    if (type.rows() == arg->type().rows() && type.columns() == arg->type().columns()) {
        return arg;
    }

    return std::make_unique<ConstructorMatrixResize>(offset, type, std::move(arg));
}

Expression::ComparisonResult ConstructorMatrixResize::compareConstant(
        const Expression& other) const {
    SkASSERT(other.type().isMatrix());
    SkASSERT(this->type() == other.type());
    SkASSERT(this->type().rows() == other.type().rows());
    SkASSERT(this->type().columns() == other.type().columns());

    // Check each cell individually.
    for (int col = 0; col < this->type().columns(); col++) {
        for (int row = 0; row < this->type().rows(); row++) {
            if (this->getMatComponent(col, row) != other.getMatComponent(col, row)) {
                return ComparisonResult::kNotEqual;
            }
        }
    }

    return ComparisonResult::kEqual;
}

SKSL_FLOAT ConstructorMatrixResize::getMatComponent(int col, int row) const {
    SkASSERT(this->isCompileTimeConstant());
    SkASSERT(col >= 0);
    SkASSERT(row >= 0);
    SkASSERT(col < this->type().columns());
    SkASSERT(row < this->type().rows());

    // GLSL resize matrices are of the form:
    //  |m m 0|
    //  |m m 0|
    //  |0 0 1|
    // Where `m` is the matrix being wrapped, and other cells contain the identity matrix.

    // Forward `getMatComponent` to the wrapped matrix if the position is in its bounds.
    if (col < this->argument()->type().columns() && row < this->argument()->type().rows()) {
        return this->argument()->getMatComponent(col, row);
    }

    // Synthesize an identity matrix for out-of-bounds positions.
    return (col == row) ? 1.0f : 0.0f;
}

}  // namespace SkSL
