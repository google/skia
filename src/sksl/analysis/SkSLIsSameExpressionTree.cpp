/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLOperator.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLLiteral.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVariableReference.h"

#include <cstddef>
#include <memory>

namespace SkSL {

bool Analysis::IsSameExpressionTree(const Expression& left, const Expression& right) {
    if (left.kind() != right.kind() || !left.type().matches(right.type())) {
        return false;
    }

    // This isn't a fully exhaustive list of expressions by any stretch of the imagination; for
    // instance, `x[y+1] = x[y+1]` isn't detected because we don't look at BinaryExpressions.
    // Since this is intended to be used for optimization purposes, handling the common cases is
    // sufficient.
    switch (left.kind()) {
        case Expression::Kind::kLiteral:
            return left.as<Literal>().value() == right.as<Literal>().value();

        case Expression::Kind::kConstructorArray:
        case Expression::Kind::kConstructorArrayCast:
        case Expression::Kind::kConstructorCompound:
        case Expression::Kind::kConstructorCompoundCast:
        case Expression::Kind::kConstructorDiagonalMatrix:
        case Expression::Kind::kConstructorMatrixResize:
        case Expression::Kind::kConstructorScalarCast:
        case Expression::Kind::kConstructorStruct:
        case Expression::Kind::kConstructorSplat: {
            if (left.kind() != right.kind()) {
                return false;
            }
            const AnyConstructor& leftCtor = left.asAnyConstructor();
            const AnyConstructor& rightCtor = right.asAnyConstructor();
            const auto leftSpan = leftCtor.argumentSpan();
            const auto rightSpan = rightCtor.argumentSpan();
            if (leftSpan.size() != rightSpan.size()) {
                return false;
            }
            for (size_t index = 0; index < leftSpan.size(); ++index) {
                if (!IsSameExpressionTree(*leftSpan[index], *rightSpan[index])) {
                    return false;
                }
            }
            return true;
        }
        case Expression::Kind::kFieldAccess:
            return left.as<FieldAccess>().fieldIndex() == right.as<FieldAccess>().fieldIndex() &&
                   IsSameExpressionTree(*left.as<FieldAccess>().base(),
                                        *right.as<FieldAccess>().base());

        case Expression::Kind::kIndex:
            return IsSameExpressionTree(*left.as<IndexExpression>().index(),
                                        *right.as<IndexExpression>().index()) &&
                   IsSameExpressionTree(*left.as<IndexExpression>().base(),
                                        *right.as<IndexExpression>().base());

        case Expression::Kind::kPrefix:
            return (left.as<PrefixExpression>().getOperator().kind() ==
                    right.as<PrefixExpression>().getOperator().kind()) &&
                   IsSameExpressionTree(*left.as<PrefixExpression>().operand(),
                                        *right.as<PrefixExpression>().operand());

        case Expression::Kind::kSwizzle:
            return left.as<Swizzle>().components() == right.as<Swizzle>().components() &&
                   IsSameExpressionTree(*left.as<Swizzle>().base(), *right.as<Swizzle>().base());

        case Expression::Kind::kVariableReference:
            return left.as<VariableReference>().variable() ==
                   right.as<VariableReference>().variable();

        default:
            return false;
    }
}

}  // namespace SkSL
