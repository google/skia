/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/ir/SkSLConstructorSplat.h"

namespace SkSL {

std::unique_ptr<Expression> ConstructorSplat::Make(const Context& context,
                                                   int offset,
                                                   const Type& type,
                                                   std::unique_ptr<Expression> arg) {
    SkASSERT(arg->type().scalarTypeForLiteral() == type.componentType().scalarTypeForLiteral());
    SkASSERT(arg->type().isScalar());

    // A "splat" to a scalar type is a no-op and can be eliminated.
    if (type.isScalar()) {
        return arg;
    }

    if (context.fConfig->fSettings.fOptimize) {
        // Replace constant variables with their corresponding values, so `float3(five)` can
        // compile down to `float3(5.0)` (the latter is a compile-time constant).
        arg = ConstantFolder::MakeConstantValueForVariable(std::move(arg));
    }

    SkASSERT(type.isVector());
    return std::make_unique<ConstructorSplat>(offset, type, std::move(arg));
}

}  // namespace SkSL
