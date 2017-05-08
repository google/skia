/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTTERNARYEXPRESSION
#define SKSL_ASTTERNARYEXPRESSION

#include "SkSLASTExpression.h"

namespace SkSL {

/**
 * A ternary expression (test ? ifTrue : ifFalse).
 */
struct ASTTernaryExpression : public ASTExpression {
    ASTTernaryExpression(std::unique_ptr<ASTExpression> test,
                         std::unique_ptr<ASTExpression> ifTrue,
                         std::unique_ptr<ASTExpression> ifFalse)
    : INHERITED(test->fPosition, kTernary_Kind)
    , fTest(std::move(test))
    , fIfTrue(std::move(ifTrue))
    , fIfFalse(std::move(ifFalse)) {}

    String description() const override {
        return "(" + fTest->description() + " ? " + fIfTrue->description() + " : " +
               fIfFalse->description() + ")";
    }

    const std::unique_ptr<ASTExpression> fTest;
    const std::unique_ptr<ASTExpression> fIfTrue;
    const std::unique_ptr<ASTExpression> fIfFalse;

    typedef ASTExpression INHERITED;
};

} // namespace

#endif
