/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLConstructorVectorCast.h"

#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLConstructorSplat.h"

namespace SkSL {

static std::unique_ptr<Expression> cast_constant_scalar(const Context& context,
                                                        const Type& scalarType,
                                                        std::unique_ptr<Expression> arg) {
    // Wrap the scalar in a constructor of the appropriate type and typecasting happens.
    // TODO(skia:11032): once we have ConstructorScalarCast::Make, use it.
    int offset = arg->fOffset;
    ExpressionArray ctorArgs;
    ctorArgs.push_back(std::move(arg));
    auto ctor = Constructor::Convert(context, offset, scalarType, std::move(ctorArgs));
    SkASSERT(ctor);
    return ctor;
}

static std::unique_ptr<Expression> cast_constant_vector(const Context& context,
                                                        const Type& destType,
                                                        std::unique_ptr<Expression> arg) {
    const Type& scalarType = destType.componentType();
    if (arg->is<ConstructorSplat>()) {
        // This is a vector-cast of a splat containing a constant value, e.g. `half4(7)`. We can
        // replace it with a splat of a different type, e.g. `int4(7)`.
        ConstructorSplat& splat = arg->as<ConstructorSplat>();
        return ConstructorSplat::Make(
                context, arg->fOffset, destType,
                cast_constant_scalar(context, scalarType, std::move(splat.argument())));
    }

    // Create a vector Constructor(literal, ...) which typecasts each argument inside.
    ExpressionArray ctorArgs;
    ctorArgs.reserve_back(arg->asAnyConstructor().argumentSpan().size());
    for (std::unique_ptr<Expression>& arg : arg->asAnyConstructor().argumentSpan()) {
        if (arg->type().isScalar()) {
            ctorArgs.push_back(cast_constant_scalar(context, scalarType, std::move(arg)));
        } else {
            // Convert inner vectors recursively (to maintain the same overall structure).
            SkASSERT(arg->type().isVector());
            ctorArgs.push_back(cast_constant_vector(
                    context,
                    scalarType.toCompound(context, /*columns=*/arg->type().columns(), /*rows=*/1),
                    std::move(arg)));
        }
    }

    // TODO(skia:11032): once we have ConstructorVector::Make, use it.
    auto ctor = Constructor::Convert(context, arg->fOffset, destType, std::move(ctorArgs));
    SkASSERT(ctor);
    return ctor;
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

    if (arg->isCompileTimeConstant()) {
        // `arg` is a compile-time constant; rather than wrap it in a cast, convert it to the
        // desired type.
        return cast_constant_vector(context, type, std::move(arg));
    }

    if (type == arg->type()) {
        return arg;
    }

    return std::make_unique<ConstructorVectorCast>(offset, type, std::move(arg));
}

}  // namespace SkSL
