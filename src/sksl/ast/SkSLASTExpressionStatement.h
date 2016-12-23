/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_ASTEXPRESSIONSTATEMENT
#define SKSL_ASTEXPRESSIONSTATEMENT

#include "SkSLASTStatement.h"

namespace SkSL {

/**
 * A lone expression being used as a statement. 
 */
struct ASTExpressionStatement : public ASTStatement {
    ASTExpressionStatement(sk_up<ASTExpression> expression)
            : INHERITED(expression->fPosition, kExpression_Kind)
            , fExpression(std::move(expression)) {}

    SkString description() const override {
        return fExpression->description() + ";";
    }

    const sk_up<ASTExpression> fExpression;

    typedef ASTStatement INHERITED;
};

} // namespace

#endif
