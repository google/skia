/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLConstructorSplat.h"

namespace SkSL {

thread_local bool gEncounteredMysteriousSplat = false;

std::unique_ptr<Expression> ConstructorSplat::Make(const Context& context,
                                                   int offset,
                                                   const Type& type,
                                                   std::unique_ptr<Expression> arg) {
    SkASSERT(arg->type() == type.componentType());
    if (arg->type() != type.componentType()) {
        gEncounteredMysteriousSplat = true;
    }

    SkASSERT(arg->type().isScalar());

    // A "splat" to a scalar type is a no-op and can be eliminated.
    if (type.isScalar()) {
        return arg;
    }

    SkASSERT(type.isVector());
    return std::make_unique<ConstructorSplat>(offset, type, std::move(arg));
}

}  // namespace SkSL
