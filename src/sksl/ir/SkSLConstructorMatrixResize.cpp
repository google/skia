/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLConstructorMatrixResize.h"

#include "include/core/SkTypes.h"
#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

std::unique_ptr<Expression> ConstructorMatrixResize::Make(const Context& context,
                                                          Position pos,
                                                          const Type& type,
                                                          std::unique_ptr<Expression> arg) {
    SkASSERT(type.isMatrix());
    SkASSERT(type.isAllowedInES2(context));
    SkASSERT(arg->type().componentType().matches(type.componentType()));

    // If the matrix isn't actually changing size, return it as-is.
    if (type.rows() == arg->type().rows() && type.columns() == arg->type().columns()) {
        return arg;
    }

    return std::make_unique<ConstructorMatrixResize>(pos, type, std::move(arg));
}

std::optional<double> ConstructorMatrixResize::getConstantValue(int n) const {
    int rows = this->type().rows();
    int row = n % rows;
    int col = n / rows;

    SkASSERT(col >= 0);
    SkASSERT(row >= 0);
    SkASSERT(col < this->type().columns());
    SkASSERT(row < this->type().rows());

    // GLSL resize matrices are of the form:
    //  |m m 0|
    //  |m m 0|
    //  |0 0 1|
    // Where `m` is the matrix being wrapped, and other cells contain the identity matrix.

    // Forward `getConstantValue` to the wrapped matrix if the position is in its bounds.
    if (col < this->argument()->type().columns() && row < this->argument()->type().rows()) {
        // Recalculate `n` in terms of the inner matrix's dimensions.
        n = row + (col * this->argument()->type().rows());
        return this->argument()->getConstantValue(n);
    }

    // Synthesize an identity matrix for out-of-bounds positions.
    return (col == row) ? 1.0 : 0.0;
}

}  // namespace SkSL
