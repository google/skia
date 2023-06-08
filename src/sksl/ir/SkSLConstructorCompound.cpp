/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLConstructorCompound.h"

#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLConstructorSplat.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLLiteral.h"
#include "src/sksl/ir/SkSLType.h"

#include <algorithm>
#include <cstddef>
#include <numeric>
#include <string>

namespace SkSL {

static bool is_safe_to_eliminate(const Type& type, const Expression& arg) {
    if (type.isScalar()) {
        // A scalar "compound type" with a single scalar argument is a no-op and can be eliminated.
        // (Pedantically, this isn't a compound at all, but it's harmless to allow and simplifies
        // call sites which need to narrow a vector and may sometimes end up with a scalar.)
        SkASSERTF(arg.type().matches(type), "Creating type '%s' from '%s'",
                  type.description().c_str(), arg.type().description().c_str());
        return true;
    }
    if (type.isVector() && arg.type().matches(type)) {
        // A vector compound constructor containing a single argument of matching type can trivially
        // be eliminated.
        return true;
    }
    // This is a meaningful single-argument compound constructor (e.g. vector-from-matrix,
    // matrix-from-vector).
    return false;
}

static const Expression* make_splat_from_arguments(const Type& type, const ExpressionArray& args) {
    // Splats cannot represent a matrix.
    if (type.isMatrix()) {
        return nullptr;
    }
    const Expression* splatExpression = nullptr;
    for (int index = 0; index < args.size(); ++index) {
        // Arguments must only be scalars or a splat constructors (which can only contain scalars).
        const Expression* expr;
        if (args[index]->type().isScalar()) {
            expr = args[index].get();
        } else if (args[index]->is<ConstructorSplat>()) {
            expr = args[index]->as<ConstructorSplat>().argument().get();
        } else {
            return nullptr;
        }
        // On the first iteration, just remember the expression we encountered.
        if (index == 0) {
            splatExpression = expr;
            continue;
        }
        // On subsequent iterations, ensure that the expression we found matches the first one.
        // (Note that IsSameExpressionTree will always reject an Expression with side effects.)
        if (!Analysis::IsSameExpressionTree(*expr, *splatExpression)) {
            return nullptr;
        }
    }

    return splatExpression;
}

std::unique_ptr<Expression> ConstructorCompound::Make(const Context& context,
                                                      Position pos,
                                                      const Type& type,
                                                      ExpressionArray args) {
    SkASSERT(type.isAllowedInES2(context));

    // All the arguments must have matching component type.
    SkASSERT(std::all_of(args.begin(), args.end(), [&](const std::unique_ptr<Expression>& arg) {
        const Type& argType = arg->type();
        return (argType.isScalar() || argType.isVector() || argType.isMatrix()) &&
               (argType.componentType().matches(type.componentType()));
    }));

    // The slot count of the combined argument list must match the composite type's slot count.
    SkASSERT(type.slotCount() ==
             std::accumulate(args.begin(), args.end(), /*initial value*/ (size_t)0,
                             [](size_t n, const std::unique_ptr<Expression>& arg) {
                                 return n + arg->type().slotCount();
                             }));
    // No-op compound constructors (containing a single argument of the same type) are eliminated.
    // (Even though this is a "compound constructor," we let scalars pass through here; it's
    // harmless to allow and simplifies call sites which need to narrow a vector and may sometimes
    // end up with a scalar.)
    if (args.size() == 1 && is_safe_to_eliminate(type, *args.front())) {
        args.front()->fPosition = pos;
        return std::move(args.front());
    }
    // Beyond this point, the type must be a vector or matrix.
    SkASSERT(type.isVector() || type.isMatrix());

    if (context.fConfig->fSettings.fOptimize) {
        // Find ConstructorCompounds embedded inside other ConstructorCompounds and flatten them.
        //   -  float4(float2(1, 2), 3, 4)                -->  float4(1, 2, 3, 4)
        //   -  float4(w, float3(sin(x), cos(y), tan(z))) -->  float4(w, sin(x), cos(y), tan(z))
        //   -  mat2(float2(a, b), float2(c, d))          -->  mat2(a, b, c, d)

        // See how many fields we would have if composite constructors were flattened out.
        int fields = 0;
        for (const std::unique_ptr<Expression>& arg : args) {
            fields += arg->is<ConstructorCompound>()
                              ? arg->as<ConstructorCompound>().arguments().size()
                              : 1;
        }

        // If we added up more fields than we're starting with, we found at least one input that can
        // be flattened out.
        if (fields > args.size()) {
            ExpressionArray flattened;
            flattened.reserve_exact(fields);
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
        arg = ConstantFolder::MakeConstantValueForVariable(pos, std::move(arg));
    }

    if (context.fConfig->fSettings.fOptimize) {
        // Reduce compound constructors to splats where possible.
        if (const Expression* splat = make_splat_from_arguments(type, args)) {
            return ConstructorSplat::Make(context, pos, type, splat->clone());
        }
    }

    return std::make_unique<ConstructorCompound>(pos, type, std::move(args));
}

std::unique_ptr<Expression> ConstructorCompound::MakeFromConstants(const Context& context,
                                                                   Position pos,
                                                                   const Type& returnType,
                                                                   const double value[]) {
    int numSlots = returnType.slotCount();
    ExpressionArray array;
    array.reserve_exact(numSlots);
    for (int index = 0; index < numSlots; ++index) {
        array.push_back(Literal::Make(pos, value[index], &returnType.componentType()));
    }
    return ConstructorCompound::Make(context, pos, returnType, std::move(array));
}

}  // namespace SkSL
