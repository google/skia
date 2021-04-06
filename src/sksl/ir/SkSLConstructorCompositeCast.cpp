/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLConstructorCompositeCast.h"

#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLConstructorComposite.h"
#include "src/sksl/ir/SkSLConstructorDiagonalMatrix.h"
#include "src/sksl/ir/SkSLConstructorScalarCast.h"
#include "src/sksl/ir/SkSLConstructorSplat.h"

namespace SkSL {

std::unique_ptr<Expression> ConstructorCompositeCast::Make(const Context& context,
                                                           int offset,
                                                           const Type& type,
                                                           std::unique_ptr<Expression> arg) {
    // Only vectors or matrices of the same dimensions are allowed.
    SkASSERT(type.isVector() || type.isMatrix());
    SkASSERT(arg->type().isVector() == type.isVector());
    SkASSERT(arg->type().isMatrix() == type.isMatrix());
    SkASSERT(type.columns() == arg->type().columns());
    SkASSERT(type.rows() == arg->type().rows());

    // If this is a no-op cast, return the expression as-is.
    if (type == arg->type()) {
        return arg;
    }
    // We can cast a vector or matrix of compile-time constants at compile-time.
    if (std::unique_ptr<Expression> converted = arg->castConstantExpression(context, type)) {
        return converted;
    }
    return std::make_unique<ConstructorCompositeCast>(offset, type, std::move(arg));
}

}  // namespace SkSL
