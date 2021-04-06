/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLConstructorDiagonalMatrix.h"

#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

std::unique_ptr<Expression> ConstructorDiagonalMatrix::Make(const Context& context,
                                                            int offset,
                                                            const Type& type,
                                                            std::unique_ptr<Expression> arg) {
    SkASSERT(type.isMatrix());
    SkASSERT(arg->type() == type.componentType());
    return std::make_unique<ConstructorDiagonalMatrix>(offset, type, std::move(arg));
}

Expression::ComparisonResult ConstructorDiagonalMatrix::compareConstant(
        const Expression& other) const {
    SkASSERT(other.type().isMatrix());
    SkASSERT(this->type() == other.type());

    // The other constructor might not be DiagonalMatrix-based, so we check each cell individually.
    for (int col = 0; col < this->type().columns(); col++) {
        for (int row = 0; row < this->type().rows(); row++) {
            if (this->getMatComponent(col, row) != other.getMatComponent(col, row)) {
                return ComparisonResult::kNotEqual;
            }
        }
    }

    return ComparisonResult::kEqual;
}

SKSL_FLOAT ConstructorDiagonalMatrix::getMatComponent(int col, int row) const {
    SkASSERT(this->isCompileTimeConstant());
    SkASSERT(col >= 0);
    SkASSERT(row >= 0);
    SkASSERT(col < this->type().columns());
    SkASSERT(row < this->type().rows());

    // Our matrix is of the form:
    //  |x 0 0|
    //  |0 x 0|
    //  |0 0 x|
    return (col == row) ? this->argument()->getConstantFloat() : 0.0;
}

}  // namespace SkSL
