/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLConstructorSplat.h"

namespace SkSL {

std::unique_ptr<Expression> ConstructorSplat::Make(const Context& context,
                                                   int offset,
                                                   const Type& type,
                                                   std::unique_ptr<Expression> arg) {
    SkASSERT(type.isVector());
    SkASSERT(arg->type() == type.componentType());
    return std::make_unique<ConstructorSplat>(offset, type, std::move(arg));
}

/*
template <typename T>
static ComparisonResult compare_splat(const Expression& value, const AnyConstructor& right) {
    ComparisonResult check = ComparisonResult::kEqual;
    for (const std::unique_ptr<Expression>& expr : right.argumentSpan()) {
        check = expr->compareConstant(value);
        if (check != ComparisonResult::kEqual) {
            break;
        }
    }

    return check;
}
*/
Expression::ComparisonResult ConstructorSplat::compareConstant(const Expression& other) const {
    SkASSERT(this->type() == other.type());
    if (!other.isAnyConstructor()) {
        return ComparisonResult::kNotEqual;
    }
/*
    const Type& scalarType = this->type().componentType();
    if (scalarType.isFloat()) {
        return compare_splat<SKSL_FLOAT>(*argument(), other.asAnyConstructor());
    }
    if (scalarType.isInteger()) {
        return compare_splat<SKSL_INT>(*argument(), other.asAnyConstructor());
    }
    if (scalarType.isBoolean()) {
        return compare_splat<bool>(*argument(), other.asAnyConstructor());
    }
*/

    ComparisonResult check = ComparisonResult::kEqual;
    for (const std::unique_ptr<Expression>& expr : other.asAnyConstructor().argumentSpan()) {
        check = argument()->compareConstant(*expr);
        if (check != ComparisonResult::kEqual) {
            break;
        }
    }

    return check;
}

}  // namespace SkSL
