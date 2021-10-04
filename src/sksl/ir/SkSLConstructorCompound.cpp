/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/ir/SkSLConstructorCompound.h"

#include <algorithm>
#include <numeric>

namespace SkSL {

std::unique_ptr<Expression> ConstructorCompound::Make(const Context& context,
                                                      int line,
                                                      const Type& type,
                                                      ExpressionArray args) {
    SkASSERT(type.isAllowedInES2(context));

    // A scalar "composite" type with a single scalar argument is a no-op and can be eliminated.
    // (Pedantically, this isn't a composite at all, but it's harmless to allow and simplifies
    // call sites which need to narrow a vector and may sometimes end up with a scalar.)
    if (type.isScalar() && args.size() == 1 && args.front()->type() == type) {
        return std::move(args.front());
    }

    // The type must be a vector or matrix, and all the arguments must have matching component type.
    SkASSERT(type.isVector() || type.isMatrix());
    SkASSERT(std::all_of(args.begin(), args.end(), [&](const std::unique_ptr<Expression>& arg) {
        const Type& argType = arg->type();
        return (argType.isScalar() || argType.isVector() || argType.isMatrix()) &&
               (argType.componentType() == type.componentType());
    }));

    // The slot count of the combined argument list must match the composite type's slot count.
    SkASSERT(type.slotCount() ==
             std::accumulate(args.begin(), args.end(), /*initial value*/ (size_t)0,
                             [](size_t n, const std::unique_ptr<Expression>& arg) {
                                 return n + arg->type().slotCount();
                             }));

    if (context.fConfig->fSettings.fOptimize) {
        // Find ConstructorCompounds embedded inside other ConstructorCompounds and flatten them.
        //   -  float4(float2(1, 2), 3, 4)                -->  float4(1, 2, 3, 4)
        //   -  float4(w, float3(sin(x), cos(y), tan(z))) -->  float4(w, sin(x), cos(y), tan(z))
        //   -  mat2(float2(a, b), float2(c, d))          -->  mat2(a, b, c, d)

        // See how many fields we would have if composite constructors were flattened out.
        size_t fields = 0;
        for (const std::unique_ptr<Expression>& arg : args) {
            fields += arg->is<ConstructorCompound>()
                              ? arg->as<ConstructorCompound>().arguments().size()
                              : 1;
        }

        // If we added up more fields than we're starting with, we found at least one input that can
        // be flattened out.
        if (fields > args.size()) {
            ExpressionArray flattened;
            flattened.reserve_back(fields);
            for (std::unique_ptr<Expression>& arg : args) {
                // For non-ConstructorCompound fields, move them over as-is.
                if (!arg->is<ConstructorCompound>()) {
                    flattened.push_back(std::move(arg));
                    continue;
                }
                // For ConstructorCompound fields, move over their inner arguments individually.
                ConstructorCompound& compositeCtor = arg->as<ConstructorCompound>();
                for (std::unique_ptr<Expression>& innerArg : compositeCtor.arguments()) {
                    flattened.push_back(std::move(innerArg));
                }
            }
            args = std::move(flattened);
        }
    }

    // Replace constant variables with their corresponding values, so `float2(one, two)` can
    // compile down to `float2(1.0, 2.0)` (the latter is a compile-time constant).
    for (std::unique_ptr<Expression>& arg : args) {
        arg = ConstantFolder::MakeConstantValueForVariable(std::move(arg));
    }

    return std::make_unique<ConstructorCompound>(line, type, std::move(args));
}

}  // namespace SkSL
