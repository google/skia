/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkSLDefines.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLConstructorCompound.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLLiteral.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/transform/SkSLTransform.h"

#include <cstdint>
#include <memory>
#include <utility>

namespace SkSL {

std::unique_ptr<Expression> Transform::RewriteIndexedSwizzle(const Context& context,
                                                             const IndexExpression& indexExpr) {
    // The index expression _must_ have a swizzle base for this transformation to be valid.
    if (!indexExpr.base()->is<Swizzle>()) {
        return nullptr;
    }
    const Swizzle& swizzle = indexExpr.base()->as<Swizzle>();

    // Convert the swizzle components to a literal array.
    ExpressionArray uvecArray;
    uvecArray.reserve(swizzle.components().size());
    for (int8_t comp : swizzle.components()) {
        uvecArray.push_back(Literal::Make(indexExpr.fPosition, comp, context.fTypes.fUInt.get()));
    }

    // Make a compound constructor with the literal array.
    const Type& uvecType = context.fTypes.fUInt->toCompound(context, uvecArray.size(), /*rows=*/1);
    std::unique_ptr<Expression> uvec =
            ConstructorCompound::Make(context, indexExpr.fPosition, uvecType, std::move(uvecArray));

    // Create a rewritten inner-expression corresponding to `uvec(1,2,3)[originalIndex]`.
    std::unique_ptr<Expression> innerExpr = IndexExpression::Make(
            context, indexExpr.fPosition, std::move(uvec), indexExpr.index()->clone());

    // Return a rewritten outer-expression corresponding to `base[uvec(1,2,3)[originalIndex]]`.
    return IndexExpression::Make(
            context, indexExpr.fPosition, swizzle.base()->clone(), std::move(innerExpr));
}

}  // namespace SkSL
