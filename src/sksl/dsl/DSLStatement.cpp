/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/DSLStatement.h"

#include "include/private/SkSLDefines.h"
#include "include/sksl/DSLBlock.h"
#include "include/sksl/DSLExpression.h"
#include "include/sksl/SkSLPosition.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLNop.h"

namespace SkSL {

namespace dsl {

DSLStatement::DSLStatement() {}

DSLStatement::DSLStatement(DSLBlock block)
    : fStatement(block.release()) {}

DSLStatement::DSLStatement(DSLExpression expr) {
    std::unique_ptr<SkSL::Expression> skslExpr = expr.release();
    if (skslExpr) {
        fStatement = SkSL::ExpressionStatement::Convert(ThreadContext::Context(),
                                                        std::move(skslExpr));
    }
}

DSLStatement::DSLStatement(std::unique_ptr<SkSL::Expression> expr)
    : fStatement(SkSL::ExpressionStatement::Convert(ThreadContext::Context(), std::move(expr))) {
    SkASSERT(this->hasValue());
}

DSLStatement::DSLStatement(std::unique_ptr<SkSL::Statement> stmt)
    : fStatement(std::move(stmt)) {
    SkASSERT(this->hasValue());
}

DSLStatement::DSLStatement(std::unique_ptr<SkSL::Statement> stmt, Position pos)
        : fStatement(stmt ? std::move(stmt) : SkSL::Nop::Make()) {
    if (pos.valid() && !fStatement->fPosition.valid()) {
        fStatement->fPosition = pos;
    }
}

DSLStatement::~DSLStatement() {}

DSLStatement operator,(DSLStatement left, DSLStatement right) {
    Position pos = left.fStatement->fPosition;
    StatementArray stmts;
    stmts.reserve_back(2);
    stmts.push_back(left.release());
    stmts.push_back(right.release());
    return DSLStatement(SkSL::Block::Make(pos, std::move(stmts), Block::Kind::kCompoundStatement));
}

} // namespace dsl

} // namespace SkSL
