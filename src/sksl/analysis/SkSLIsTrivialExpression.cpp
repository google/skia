/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLLiteral.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLVariableReference.h"

#include <memory>

namespace SkSL {

bool Analysis::IsTrivialExpression(const Expression& expr) {
    return expr.is<Literal>() ||
           expr.is<VariableReference>() ||
           (expr.is<Swizzle>() &&
            IsTrivialExpression(*expr.as<Swizzle>().base())) ||
           (expr.is<FieldAccess>() &&
            IsTrivialExpression(*expr.as<FieldAccess>().base())) ||
           (expr.isAnyConstructor() &&
            expr.asAnyConstructor().argumentSpan().size() == 1 &&
            IsTrivialExpression(*expr.asAnyConstructor().argumentSpan().front())) ||
           (expr.isAnyConstructor() &&
            expr.isConstantOrUniform()) ||
           (expr.is<IndexExpression>() &&
            expr.as<IndexExpression>().index()->isIntLiteral() &&
            IsTrivialExpression(*expr.as<IndexExpression>().base()));
}

}  // namespace SkSL
