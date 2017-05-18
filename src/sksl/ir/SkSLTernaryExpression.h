/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_TERNARYEXPRESSION
#define SKSL_TERNARYEXPRESSION

#include "SkSLExpression.h"
#include "../SkSLPosition.h"

namespace SkSL {

/**
 * A ternary expression (test ? ifTrue : ifFalse).
 */
struct TernaryExpression : public Expression {
    TernaryExpression(Position position, std::unique_ptr<Expression> test,
                      std::unique_ptr<Expression> ifTrue, std::unique_ptr<Expression> ifFalse)
    : INHERITED(position, kTernary_Kind, ifTrue->fType)
    , fTest(std::move(test))
    , fIfTrue(std::move(ifTrue))
    , fIfFalse(std::move(ifFalse)) {
        ASSERT(fIfTrue->fType == fIfFalse->fType);
    }

    bool hasSideEffects() const override {
        return fTest->hasSideEffects() || fIfTrue->hasSideEffects() || fIfFalse->hasSideEffects();
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
