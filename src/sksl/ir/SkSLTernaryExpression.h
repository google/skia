/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_TERNARYEXPRESSION
#define SKSL_TERNARYEXPRESSION

#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * A ternary expression (test ? ifTrue : ifFalse).
 */
struct TernaryExpression : public Expression {
    TernaryExpression(int offset, std::unique_ptr<Expression> test,
                      std::unique_ptr<Expression> ifTrue, std::unique_ptr<Expression> ifFalse)
    : INHERITED(offset, kTernary_Kind, ifTrue->fType)
    , fTest(std::move(test))
    , fIfTrue(std::move(ifTrue))
    , fIfFalse(std::move(ifFalse)) {
        SkASSERT(fIfTrue->fType == fIfFalse->fType);
    }

    bool hasSideEffects() const override {
        return fTest->hasSideEffects() || fIfTrue->hasSideEffects() || fIfFalse->hasSideEffects();
    }

    std::unique_ptr<Expression> clone() const override {
        return std::unique_ptr<Expression>(new TernaryExpression(fOffset, fTest->clone(),
                                                                 fIfTrue->clone(),
                                                                 fIfFalse->clone()));
    }

    String description() const override {
        return "(" + fTest->description() + " ? " + fIfTrue->description() + " : " +
               fIfFalse->description() + ")";
    }

    std::unique_ptr<Expression> fTest;
    std::unique_ptr<Expression> fIfTrue;
    std::unique_ptr<Expression> fIfFalse;

    typedef Expression INHERITED;
};

} // namespace

#endif
