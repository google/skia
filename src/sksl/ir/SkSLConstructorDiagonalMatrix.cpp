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

const Expression* ConstructorDiagonalMatrix::getConstantSubexpression(int n) const {
    SkASSERT(n >= 0);
    SkASSERT(n < this->type().columns() * this->type().rows());

    int rows = this->type().rows();
    return ((n % rows) == (n / rows)) ? this->argument()->getConstantSubexpression(0)
                                      : &fZeroLiteral;
}

}  // namespace SkSL
