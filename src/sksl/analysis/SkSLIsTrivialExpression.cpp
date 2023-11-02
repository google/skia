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
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLType.h"

#include <memory>

namespace SkSL {

bool Analysis::IsTrivialExpression(const Expression& expr) {
    switch (expr.kind()) {
        case Expression::Kind::kLiteral:
        case Expression::Kind::kVariableReference:
            return true;

        case Expression::Kind::kSwizzle:
            // All swizzles are considered to be trivial.
            return IsTrivialExpression(*expr.as<Swizzle>().base());

        case Expression::Kind::kPrefix: {
            const PrefixExpression& prefix = expr.as<PrefixExpression>();
            switch (prefix.getOperator().kind()) {
                case OperatorKind::PLUS:
                case OperatorKind::MINUS:
                case OperatorKind::LOGICALNOT:
                case OperatorKind::BITWISENOT:
                    return IsTrivialExpression(*prefix.operand());

                default:
                    return false;
            }
        }
        case Expression::Kind::kFieldAccess:
            // Accessing a field is trivial.
            return IsTrivialExpression(*expr.as<FieldAccess>().base());

        case Expression::Kind::kIndex: {
            // Accessing a constant array index is trivial.
            const IndexExpression& inner = expr.as<IndexExpression>();
            return inner.index()->isIntLiteral() && IsTrivialExpression(*inner.base());
        }
        case Expression::Kind::kConstructorArray:
        case Expression::Kind::kConstructorStruct:
            // Only consider small arrays/structs of compile-time-constants to be trivial.
            return expr.type().slotCount() <= 4 && IsCompileTimeConstant(expr);

        case Expression::Kind::kConstructorArrayCast:
        case Expression::Kind::kConstructorMatrixResize:
            // These operations require function calls in Metal, so they're never trivial.
            return false;

        case Expression::Kind::kConstructorCompound:
            // Only compile-time-constant compound constructors are considered to be trivial.
            return IsCompileTimeConstant(expr);

        case Expression::Kind::kConstructorCompoundCast:
        case Expression::Kind::kConstructorScalarCast:
        case Expression::Kind::kConstructorSplat:
        case Expression::Kind::kConstructorDiagonalMatrix: {
            // Single-argument constructors are trivial when their inner expression is trivial.
            SkASSERT(expr.asAnyConstructor().argumentSpan().size() == 1);
            const Expression& inner = *expr.asAnyConstructor().argumentSpan().front();
            return IsTrivialExpression(inner);
        }
        default:
            return false;
    }
}

}  // namespace SkSL
