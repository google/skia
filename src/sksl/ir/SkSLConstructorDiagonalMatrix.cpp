/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLConstructorDiagonalMatrix.h"

#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

std::unique_ptr<Expression> ConstructorDiagonalMatrix::Make(const Context& context,
                                                            int line,
                                                            const Type& type,
                                                            std::unique_ptr<Expression> arg) {
    SkASSERT(type.isMatrix());
    SkASSERT(type.isAllowedInES2(context));
    SkASSERT(arg->type().isScalar());
    SkASSERT(arg->type() == type.componentType());

    // Look up the value of constant variables. This allows constant-expressions like `mat4(five)`
    // to be replaced with `mat4(5.0)`.
    arg = ConstantFolder::MakeConstantValueForVariable(std::move(arg));

    return std::make_unique<ConstructorDiagonalMatrix>(line, type, std::move(arg));
}

skstd::optional<double> ConstructorDiagonalMatrix::getConstantValue(int n) const {
    int rows = this->type().rows();
    int row = n % rows;
    int col = n / rows;

    SkASSERT(col >= 0);
    SkASSERT(row >= 0);
    SkASSERT(col < this->type().columns());
    SkASSERT(row < this->type().rows());

    return (col == row) ? this->argument()->getConstantValue(0) : 0.0;
}

}  // namespace SkSL
