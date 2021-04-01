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

Expression::ComparisonResult ConstructorSplat::compareConstant(const Expression& other) const {
    SkASSERT(this->type() == other.type());
    if (!other.isAnyConstructor()) {
        return ComparisonResult::kUnknown;
    }

    return this->compareConstantConstructor(other.asAnyConstructor());
}

Expression::ComparisonResult ConstructorSplat::compareConstantConstructor(
        const AnyConstructor& other) const {
    ComparisonResult check = ComparisonResult::kEqual;
    for (const std::unique_ptr<Expression>& expr : other.argumentSpan()) {
        // We need to recurse to handle nested constructors like `half4(1) == half4(half2(1), 1, 1)`
        check = expr->isAnyConstructor()
                        ? this->compareConstantConstructor(expr->asAnyConstructor())
                        : argument()->compareConstant(*expr);
        if (check != ComparisonResult::kEqual) {
            break;
        }
    }

    return check;
}

}  // namespace SkSL
