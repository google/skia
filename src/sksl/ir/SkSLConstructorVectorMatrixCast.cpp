/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLConstructorVectorMatrixCast.h"

#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLConstructorCompound.h"
#include "src/sksl/ir/SkSLConstructorDiagonalMatrix.h"
#include "src/sksl/ir/SkSLConstructorScalarCast.h"
#include "src/sksl/ir/SkSLConstructorSplat.h"

namespace SkSL {

static std::unique_ptr<Expression> cast_constant_vector_matrix(
        const Context& context,
        const Type& destType,
        std::unique_ptr<Expression> constCtor) {
    // Clone each slot of the passed-in constructor. Matrices and vectors interpret splats
    // differently, so we always write out each of the expression's slots individually.
    const int constCtorSlots = constCtor->type().slotCount();
    ExpressionArray typecastArgs;
    typecastArgs.reserve_back(constCtorSlots);
    for (int n = 0; n < constCtorSlots; ++n) {
        typecastArgs.push_back(constCtor->getConstantSubexpression(n)->clone());
    }

    // Wrap the cloned values in a compound constructor of the result type.
    return ConstructorCompound::Make(context, constCtor->fOffset, destType,
                                     std::move(typecastArgs));
}

std::unique_ptr<Expression> ConstructorVectorMatrixCast::Make(const Context& context,
                                                              int offset,
                                                              const Type& type,
                                                              std::unique_ptr<Expression> arg) {
    // Only four-slot vectors or matrices of the same component type are allowed.
    SkASSERT(type.isVector() || type.isMatrix());
    SkASSERT(arg->type().isVector() != type.isVector());
    SkASSERT(arg->type().isMatrix() != type.isMatrix());
    SkASSERT(type.componentType() == arg->type().componentType());
    SkASSERT(type.slotCount() == 4);
    SkASSERT(arg->type().slotCount() == 4);

    // When optimization is on, look up the value of constant variables. This allows expressions
    // like `half2x2(allSevenVec)` to be replaced with the compile-time constant `half2x2(7,7,7,7)`,
    // which is eligible for constant folding.
    if (context.fConfig->fSettings.fOptimize) {
        arg = ConstantFolder::MakeConstantValueForVariable(std::move(arg));
    }
    // We can cast compile-time constants at compile-time.
    if (arg->isCompileTimeConstant()) {
        return cast_constant_vector_matrix(context, type, std::move(arg));
    }
    return std::make_unique<ConstructorVectorMatrixCast>(offset, type, std::move(arg));
}

}  // namespace SkSL
