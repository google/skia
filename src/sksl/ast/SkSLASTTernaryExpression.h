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
    ASTTernaryExpression(
            sk_up<ASTExpression> test, sk_up<ASTExpression> ifTrue, sk_up<ASTExpression> ifFalse)
            : INHERITED(test->fPosition, kTernary_Kind)
            , fTest(std::move(test))
            , fIfTrue(std::move(ifTrue))
            , fIfFalse(std::move(ifFalse)) {}

    SkString description() const override {
        return "(" + fTest->description() + " ? " + fIfTrue->description() + " : " +
               fIfFalse->description() + ")";        
    }

    const sk_up<ASTExpression> fTest;
    const sk_up<ASTExpression> fIfTrue;
    const sk_up<ASTExpression> fIfFalse;

    typedef ASTExpression INHERITED;
};

} // namespace

#endif
