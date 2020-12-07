/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/DSLStatement.h"

#include "src/sksl/dsl/DSLBlock.h"
#include "src/sksl/dsl/DSLExpression.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"

namespace SkSL {

namespace dsl {

DSLStatement::DSLStatement(DSLBlock block)
    : fStatement(block.release()) {}

DSLStatement::DSLStatement(DSLExpression expr) {
    std::unique_ptr<SkSL::Expression> skslExpr = expr.release();
    if (skslExpr) {
        fStatement = std::make_unique<SkSL::ExpressionStatement>(std::move(skslExpr));
    }
}

DSLStatement::DSLStatement(std::unique_ptr<SkSL::Expression> expr)
    : fStatement(std::make_unique<SkSL::ExpressionStatement>(std::move(expr))) {}

} // namespace dsl

} // namespace SkSL
