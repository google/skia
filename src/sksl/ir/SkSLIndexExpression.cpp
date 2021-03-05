/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLSwizzle.h"

namespace SkSL {

const Type& IndexExpression::IndexType(const Context& context, const Type& type) {
    if (type.isMatrix()) {
        if (type.componentType() == *context.fTypes.fFloat) {
            switch (type.rows()) {
                case 2: return *context.fTypes.fFloat2;
                case 3: return *context.fTypes.fFloat3;
                case 4: return *context.fTypes.fFloat4;
                default: SkASSERT(false);
            }
        } else if (type.componentType() == *context.fTypes.fHalf) {
            switch (type.rows()) {
                case 2: return *context.fTypes.fHalf2;
                case 3: return *context.fTypes.fHalf3;
                case 4: return *context.fTypes.fHalf4;
                default: SkASSERT(false);
            }
        }
    }
    return type.componentType();
}

std::unique_ptr<Expression> IndexExpression::Convert(const Context& context,
                                                     std::unique_ptr<Expression> base,
                                                     std::unique_ptr<Expression> index) {
    // Convert an index expression with an expression inside of it: `arr[a * 3]`.
    const Type& baseType = base->type();
    if (!baseType.isArray() && !baseType.isMatrix() && !baseType.isVector()) {
        context.fErrors.error(base->fOffset,
                              "expected array, but found '" + baseType.displayName() + "'");
        return nullptr;
    }
    if (!index->type().isInteger()) {
        index = context.fTypes.fInt->coerceExpression(std::move(index), context);
        if (!index) {
            return nullptr;
        }
    }
    // Perform compile-time bounds checking on constant indices.
    if (index->is<IntLiteral>()) {
        SKSL_INT indexValue = index->as<IntLiteral>().value();

        const int upperBound = (baseType.isArray() && baseType.columns() == Type::kUnsizedArray)
                                       ? INT_MAX
                                       : baseType.columns();
        if (indexValue < 0 || indexValue >= upperBound) {
            context.fErrors.error(base->fOffset, "index " + to_string(indexValue) + " out of range "
                                                 "for '" + baseType.displayName() + "'");
            return nullptr;
        }
    }
    return IndexExpression::Make(context, std::move(base), std::move(index));
}

std::unique_ptr<Expression> IndexExpression::Make(const Context& context,
                                                  std::unique_ptr<Expression> base,
                                                  std::unique_ptr<Expression> index) {
    const Type& baseType = base->type();
    SkASSERT(baseType.isArray() || baseType.isMatrix() || baseType.isVector());
    SkASSERT(index->type().isInteger());

    // Constant array indexes on vectors can be converted to swizzles: `myHalf4.z`.
    // Swizzling is harmless and can unlock further simplifications for some base types.
    if (index->is<IntLiteral>() && baseType.isVector()) {
        SKSL_INT indexValue = index->as<IntLiteral>().value();
        SkASSERT(indexValue >= 0);
        SkASSERT(indexValue < baseType.columns());
        return Swizzle::Make(context, std::move(base), ComponentArray{(int8_t)indexValue});
    }

    return std::make_unique<IndexExpression>(context, std::move(base), std::move(index));
}

}  // namespace SkSL
