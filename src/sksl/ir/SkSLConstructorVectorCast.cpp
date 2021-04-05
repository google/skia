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

static std::unique_ptr<Expression> cast_constant_vector(const Context& context,
                                                        const Type& destType,
                                                        std::unique_ptr<Expression> constCtor) {
    const Type& scalarType = destType.componentType();
    if (constCtor->is<ConstructorSplat>()) {
        // This is a vector-cast of a splat containing a constant value, e.g. `half4(7)`. We can
        // replace it with a splat of a different type, e.g. `int4(7)`.
        ConstructorSplat& splat = constCtor->as<ConstructorSplat>();
        return ConstructorSplat::Make(
                context, constCtor->fOffset, destType,
                ConstructorScalarCast::Make(context, constCtor->fOffset, scalarType,
                                            std::move(splat.argument())));
    }

    // Create a vector Constructor(literal, ...) which typecasts each argument inside.
    auto inputArgs = constCtor->asAnyConstructor().argumentSpan();
    ExpressionArray typecastArgs;
    typecastArgs.reserve_back(inputArgs.size());
    for (std::unique_ptr<Expression>& arg : inputArgs) {
        const Type& argType = arg->type();
        if (argType.isScalar()) {
            int offset = arg->fOffset;
            typecastArgs.push_back(ConstructorScalarCast::Make(context, offset, scalarType,
                                                               std::move(arg)));
        } else {
            // Convert inner constant-vectors recursively.
            SkASSERT(argType.isVector());
            typecastArgs.push_back(cast_constant_vector(
                    context,
                    scalarType.toCompound(context, /*columns=*/argType.columns(), /*rows=*/1),
                    std::move(arg)));
        }
    }

    // TODO(skia:11032): once we have ConstructorVector::Make, use it.
    auto typecastVec = Constructor::Convert(context, constCtor->fOffset, destType,
                                            std::move(typecastArgs));
    SkASSERT(typecastVec);
    return typecastVec;
}

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
    if (arg->isCompileTimeConstant()) {
        return cast_constant_vector(context, type, std::move(arg));
    }
    return std::make_unique<ConstructorVectorCast>(offset, type, std::move(arg));
}

}  // namespace SkSL
