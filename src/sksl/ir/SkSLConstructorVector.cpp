/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLConstructorVector.h"

#include <algorithm>
#include <numeric>

namespace SkSL {

std::unique_ptr<Expression> ConstructorVector::Make(const Context& context,
                                                    int offset,
                                                    const Type& type,
                                                    ExpressionArray args) {
    // A scalar "vector" type with a single scalar argument is a no-op and can be eliminated.
    // (Pedantically, this isn't a vector, but it's harmless to allow and simplifies things for
    // callers which need to narrow a vector.)
    if (type.isScalar() && args.size() == 1 && args.front()->type() == type) {
        return std::move(args.front());
    }

    // The type must be a vector, and all the arguments must have matching component type.
    SkASSERT(type.isVector());
    SkASSERT(std::all_of(args.begin(), args.end(), [&](const std::unique_ptr<Expression>& arg) {
        const Type& argType = arg->type();
        return (argType.isScalar() || argType.isVector()) &&
               (argType.componentType() == type.componentType());
    }));

    // The slot count of the combined argument list must match the composite type's slot count.
    SkASSERT(type.slotCount() ==
             std::accumulate(args.begin(), args.end(), /*initial value*/ (size_t)0,
                             [](size_t n, const std::unique_ptr<Expression>& arg) {
                                 return n + arg->type().slotCount();
                             }));

    return std::make_unique<ConstructorVector>(offset, type, std::move(args));
}

}  // namespace SkSL
