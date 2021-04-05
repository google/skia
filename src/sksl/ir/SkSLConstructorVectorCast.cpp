/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLConstructorVectorCast.h"

#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLConstructorScalarCast.h"
#include "src/sksl/ir/SkSLConstructorSplat.h"

namespace SkSL {

std::unique_ptr<Expression> ConstructorVectorCast::Make(const Context& context,
                                                        int offset,
                                                        const Type& type,
                                                        std::unique_ptr<Expression> arg) {
    // The types must both be vectors of the same size.
    SkASSERT(type.isVector());
    SkASSERT(arg->type().isVector());
    SkASSERT(type.columns() == arg->type().columns());

    // If this is a no-op cast, return the expression as-is.
    if (type == arg->type()) {
        return arg;
    }
    // We can cast a vector of compile-time constants at compile-time.
    if (std::unique_ptr<Expression> converted = arg->castConstantExpression(context, type)) {
        return converted;
    }
    return std::make_unique<ConstructorVectorCast>(offset, type, std::move(arg));
}

}  // namespace SkSL
