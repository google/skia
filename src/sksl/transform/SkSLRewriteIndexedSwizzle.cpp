/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLConstructorCompound.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/transform/SkSLTransform.h"

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
    double vecArray[4];
    for (int index = 0; index < swizzle.components().size(); ++index) {
        vecArray[index] = swizzle.components()[index];
    }

    // Make a compound constructor with the literal array.
    const Type& vecType =
            context.fTypes.fInt->toCompound(context, swizzle.components().size(), /*rows=*/1);
    std::unique_ptr<Expression> vec =
            ConstructorCompound::MakeFromConstants(context, indexExpr.fPosition, vecType, vecArray);

    // Create a rewritten inner-expression corresponding to `vec(1,2,3)[originalIndex]`.
    std::unique_ptr<Expression> innerExpr = IndexExpression::Make(
            context, indexExpr.fPosition, std::move(vec), indexExpr.index()->clone());

    // Return a rewritten outer-expression corresponding to `base[vec(1,2,3)[originalIndex]]`.
    return IndexExpression::Make(
            context, indexExpr.fPosition, swizzle.base()->clone(), std::move(innerExpr));
}

}  // namespace SkSL
