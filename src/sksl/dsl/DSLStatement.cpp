/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/DSLStatement.h"

#include "include/sksl/DSLBlock.h"
#include "include/sksl/DSLExpression.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLNop.h"

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#endif

namespace SkSL {

namespace dsl {

DSLStatement::DSLStatement() {}

DSLStatement::DSLStatement(DSLBlock block)
    : fStatement(block.release()) {}

DSLStatement::DSLStatement(DSLExpression expr) {
    std::unique_ptr<SkSL::Expression> skslExpr = expr.release();
    if (skslExpr) {
        fStatement = SkSL::ExpressionStatement::Make(ThreadContext::Context(), std::move(skslExpr));
    }
}

DSLStatement::DSLStatement(std::unique_ptr<SkSL::Expression> expr)
    : fStatement(SkSL::ExpressionStatement::Make(ThreadContext::Context(), std::move(expr))) {
    SkASSERT(this->hasValue());
}

DSLStatement::DSLStatement(std::unique_ptr<SkSL::Statement> stmt)
    : fStatement(std::move(stmt)) {
    SkASSERT(this->hasValue());
}

DSLStatement::DSLStatement(DSLPossibleExpression expr, PositionInfo pos)
    : DSLStatement(DSLExpression(std::move(expr), pos)) {}

DSLStatement::DSLStatement(DSLPossibleStatement stmt, PositionInfo pos) {
    ThreadContext::ReportErrors(pos);
    if (stmt.hasValue()) {
        fStatement = std::move(stmt.fStatement);
    } else {
        fStatement = SkSL::Nop::Make();
    }
    if (pos.line() != -1) {
        fStatement->fLine = pos.line();
    }
}

DSLStatement::~DSLStatement() {
#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
    if (fStatement && ThreadContext::InFragmentProcessor()) {
        ThreadContext::CurrentEmitArgs()->fFragBuilder->codeAppend(this->release());
        return;
    }
#endif
    SkASSERTF(!fStatement || !ThreadContext::Settings().fAssertDSLObjectsReleased,
              "Statement destroyed without being incorporated into program (see "
              "ProgramSettings::fAssertDSLObjectsReleased)");
}

DSLPossibleStatement::DSLPossibleStatement(std::unique_ptr<SkSL::Statement> statement)
    : fStatement(std::move(statement)) {}

DSLPossibleStatement::~DSLPossibleStatement() {
    if (fStatement) {
        // this handles incorporating the expression into the output tree
        DSLStatement(std::move(fStatement));
    }
}

DSLStatement operator,(DSLStatement left, DSLStatement right) {
    int line = left.fStatement->fLine;
    StatementArray stmts;
    stmts.reserve_back(2);
    stmts.push_back(left.release());
    stmts.push_back(right.release());
    return DSLStatement(SkSL::Block::MakeUnscoped(line, std::move(stmts)));
}

} // namespace dsl

} // namespace SkSL
