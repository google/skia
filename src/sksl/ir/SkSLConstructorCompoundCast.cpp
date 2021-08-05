/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLConstructorCompoundCast.h"

#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLConstructorCompound.h"
#include "src/sksl/ir/SkSLConstructorDiagonalMatrix.h"
#include "src/sksl/ir/SkSLConstructorScalarCast.h"
#include "src/sksl/ir/SkSLConstructorSplat.h"

namespace SkSL {

static std::unique_ptr<Expression> cast_constant_composite(const Context& context,
                                                           const Type& destType,
                                                           std::unique_ptr<Expression> constCtor) {
    const Type& scalarType = destType.componentType();
    if (constCtor->is<ConstructorSplat>()) {
        // This is a composite-cast of a splat containing a constant value, e.g. `half4(7)`. We can
        // replace it with a splat of a different type, e.g. `int4(7)`.
        ConstructorSplat& splat = constCtor->as<ConstructorSplat>();
        return ConstructorSplat::Make(
                context, constCtor->fOffset, destType,
                ConstructorScalarCast::Make(context, constCtor->fOffset, scalarType,
                                            std::move(splat.argument())));
    }

    if (constCtor->is<ConstructorDiagonalMatrix>()) {
        // This is a composite-cast of a diagonal matrix, e.g. `float3x3(2)`. We can
        // replace it with a splat of a different type, e.g. `half3x3(2)`.
        ConstructorDiagonalMatrix& splat = constCtor->as<ConstructorDiagonalMatrix>();
        return ConstructorDiagonalMatrix::Make(
                context, constCtor->fOffset, destType,
                ConstructorScalarCast::Make(context, constCtor->fOffset, scalarType,
                                            std::move(splat.argument())));
    }

    // Create a composite Constructor(literal, ...) which typecasts each argument inside.
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
            // Convert inner constant-composites recursively.
            SkASSERT(argType.isVector() || (argType.isMatrix() && argType.slotCount() == 4));
            typecastArgs.push_back(cast_constant_composite(
                    context,
                    scalarType.toCompound(context, /*columns=*/argType.slotCount(), /*rows=*/1),
                    std::move(arg)));
        }
    }

    return ConstructorCompound::Make(context, constCtor->fOffset, destType,
                                     std::move(typecastArgs));
}

std::unique_ptr<Expression> ConstructorCompoundCast::Make(const Context& context,
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
    // When optimization is on, look up the value of constant variables. This allows expressions
    // like `int4(colorGreen)` to be replaced with the compile-time constant `int4(0, 1, 0, 1)`,
    // which is eligible for constant folding.
    if (context.fConfig->fSettings.fOptimize) {
        arg = ConstantFolder::MakeConstantValueForVariable(std::move(arg));
    }
    // We can cast a vector of compile-time constants at compile-time.
    if (arg->isCompileTimeConstant()) {
        return cast_constant_composite(context, type, std::move(arg));
    }
    return std::make_unique<ConstructorCompoundCast>(offset, type, std::move(arg));
}

}  // namespace SkSL
