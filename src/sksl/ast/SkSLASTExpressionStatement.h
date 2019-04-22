/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTEXPRESSIONSTATEMENT
#define SKSL_ASTEXPRESSIONSTATEMENT

#include "src/sksl/ast/SkSLASTStatement.h"

namespace SkSL {

/**
 * A lone expression being used as a statement.
 */
struct ASTExpressionStatement : public ASTStatement {
    ASTExpressionStatement(std::unique_ptr<ASTExpression> expression)
    : INHERITED(expression->fOffset, kExpression_Kind)
    , fExpression(std::move(expression)) {}

    String description() const override {
        return fExpression->description() + ";";
    }

    const std::unique_ptr<ASTExpression> fExpression;

    typedef ASTStatement INHERITED;
};

} // namespace

#endif
