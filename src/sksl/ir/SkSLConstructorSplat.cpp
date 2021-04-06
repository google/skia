/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLConstructorSplat.h"

namespace SkSL {

std::unique_ptr<Expression> ConstructorSplat::castConstantExpression(const Context& context,
                                                                     const Type& type) const {
    SkASSERT(type.columns() == this->type().columns());

    // Attempt to cast the argument to the requested type.
    std::unique_ptr<Expression> arg =
            this->argument()->castConstantExpression(context, type.componentType());
    if (arg) {
        return std::make_unique<ConstructorSplat>(fOffset, type, std::move(arg));
    }
    return nullptr;
}

std::unique_ptr<Expression> ConstructorSplat::Make(const Context& context,
                                                   int offset,
                                                   const Type& type,
                                                   std::unique_ptr<Expression> arg) {
    SkASSERT(arg->type() == type.componentType());
    SkASSERT(arg->type().isScalar());

    // A "splat" to a scalar type is a no-op and can be eliminated.
    if (type.isScalar()) {
        return arg;
    }

    SkASSERT(type.isVector());
    return std::make_unique<ConstructorSplat>(offset, type, std::move(arg));
}

}  // namespace SkSL
