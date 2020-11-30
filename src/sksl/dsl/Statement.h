/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_STATEMENT
#define SKSL_DSL_STATEMENT

#include "src/sksl/dsl/Expression.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"

namespace SkSL {

namespace dsl {

class Block;

class Statement {
public:
    Statement(std::unique_ptr<SkSL::Statement> stmt)
        : fStatement(std::move(stmt)) {
        SkASSERT(fStatement);
    }

    Statement(Expression expr) {
        std::unique_ptr<SkSL::Expression> skslExpr = expr.release();
        if (skslExpr) {
            fStatement = std::make_unique<SkSL::ExpressionStatement>(std::move(skslExpr));
        }
    }

    Statement(Block block);

    ~Statement() {
        SkASSERTF(!fStatement, "Statement destroyed without being incorporated into output tree");
    }

    std::unique_ptr<SkSL::Statement> release() {
        return std::move(fStatement);
    }

private:
    std::unique_ptr<SkSL::Statement> fStatement;
};

} // namespace dsl

} // namespace SkSL

#endif
