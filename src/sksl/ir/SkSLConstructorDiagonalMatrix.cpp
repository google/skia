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

std::unique_ptr<Expression> ConstructorDiagonalMatrix::castConstantExpression(
        const Context& context,
        const Type& type) const {
    SkASSERT(type.columns() == this->type().columns());
    SkASSERT(type.rows() == this->type().rows());

    // Attempt to cast the argument to the requested type.
    std::unique_ptr<Expression> arg =
            this->argument()->castConstantExpression(context, type.componentType());
    if (arg) {
        return std::make_unique<ConstructorDiagonalMatrix>(fOffset, type, std::move(arg));
    }
    return nullptr;
}

std::unique_ptr<Expression> ConstructorDiagonalMatrix::Make(const Context& context,
                                                            int offset,
                                                            const Type& type,
                                                            std::unique_ptr<Expression> arg) {
    SkASSERT(type.isMatrix());
    SkASSERT(arg->type() == type.componentType());
    return std::make_unique<ConstructorDiagonalMatrix>(offset, type, std::move(arg));
}

const Expression* ConstructorDiagonalMatrix::getConstantSubexpression(int n) const {
    int rows = this->type().rows();
    int row = n % rows;
    int col = n / rows;

    SkASSERT(col >= 0);
    SkASSERT(row >= 0);
    SkASSERT(col < this->type().columns());
    SkASSERT(row < this->type().rows());

    return (col == row) ? this->argument()->getConstantSubexpression(0) : &fZeroLiteral;
}

}  // namespace SkSL
