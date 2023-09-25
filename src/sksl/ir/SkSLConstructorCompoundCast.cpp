/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLConstructorCompoundCast.h"

#include "include/core/SkTypes.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/ir/SkSLConstructorCompound.h"
#include "src/sksl/ir/SkSLConstructorDiagonalMatrix.h"
#include "src/sksl/ir/SkSLConstructorScalarCast.h"
#include "src/sksl/ir/SkSLConstructorSplat.h"
#include "src/sksl/ir/SkSLType.h"

#include <cstddef>
#include <iterator>
#include <optional>

namespace SkSL {

static std::unique_ptr<Expression> cast_constant_composite(const Context& context,
                                                           Position pos,
                                                           const Type& destType,
                                                           std::unique_ptr<Expression> constCtor) {
    const Type& scalarType = destType.componentType();

    // We generate nicer code for splats and diagonal matrices by handling them separately instead
    // of relying on the constant-subexpression code below. This is not truly necessary but it makes
    // our output look a little better; human beings prefer `half4(0)` to `half4(0, 0, 0, 0)`.
    if (constCtor->is<ConstructorSplat>()) {
        // This is a typecast of a splat containing a constant value, e.g. `half4(7)`. We can
        // replace it with a splat of a different type, e.g. `int4(7)`.
        ConstructorSplat& splat = constCtor->as<ConstructorSplat>();
        return ConstructorSplat::Make(
                context, pos, destType,
                ConstructorScalarCast::Make(context, pos, scalarType, std::move(splat.argument())));
    }

    if (constCtor->is<ConstructorDiagonalMatrix>() && destType.isMatrix()) {
        // This is a typecast of a constant diagonal matrix, e.g. `float3x3(2)`. We can replace it
        // with a diagonal matrix of a different type, e.g. `half3x3(2)`.
        ConstructorDiagonalMatrix& matrixCtor = constCtor->as<ConstructorDiagonalMatrix>();
        return ConstructorDiagonalMatrix::Make(
                context, pos, destType,
                ConstructorScalarCast::Make(context, pos, scalarType,
                                            std::move(matrixCtor.argument())));
    }

    // Create a compound Constructor(literal, ...) which typecasts each scalar value inside.
    size_t numSlots = destType.slotCount();
    SkASSERT(numSlots == constCtor->type().slotCount());

    double typecastArgs[16];
    SkASSERT(numSlots <= std::size(typecastArgs));
    for (size_t index = 0; index < numSlots; ++index) {
        std::optional<double> slotVal = constCtor->getConstantValue(index);
        if (scalarType.checkForOutOfRangeLiteral(context, *slotVal, constCtor->fPosition)) {
            // We've reported an error because the literal is out of range for this type. Zero out
            // the value to avoid a cascade of errors.
            *slotVal = 0.0;
        }
        typecastArgs[index] = *slotVal;
    }

    return ConstructorCompound::MakeFromConstants(context, pos, destType, typecastArgs);
}

std::unique_ptr<Expression> ConstructorCompoundCast::Make(const Context& context,
                                                          Position pos,
                                                          const Type& type,
                                                          std::unique_ptr<Expression> arg) {
    // Only vectors or matrices of the same dimensions are allowed.
    SkASSERT(type.isVector() || type.isMatrix());
    SkASSERT(type.isAllowedInES2(context));
    SkASSERT(arg->type().isVector() == type.isVector());
    SkASSERT(arg->type().isMatrix() == type.isMatrix());
    SkASSERT(type.columns() == arg->type().columns());
    SkASSERT(type.rows() == arg->type().rows());

    // If this is a no-op cast, return the expression as-is.
    if (type.matches(arg->type())) {
        arg->setPosition(pos);
        return arg;
    }
    // Look up the value of constant variables. This allows constant-expressions like
    // `int4(colorGreen)` to be replaced with the compile-time constant `int4(0, 1, 0, 1)`.
    arg = ConstantFolder::MakeConstantValueForVariable(pos, std::move(arg));

    // We can cast a vector of compile-time constants at compile-time.
    if (Analysis::IsCompileTimeConstant(*arg)) {
        return cast_constant_composite(context, pos, type, std::move(arg));
    }
    return std::make_unique<ConstructorCompoundCast>(pos, type, std::move(arg));
}

}  // namespace SkSL
