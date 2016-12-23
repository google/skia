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
    TernaryExpression(Position position, sk_up<Expression> test, sk_up<Expression> ifTrue,
                      sk_up<Expression> ifFalse)
            : INHERITED(position, kTernary_Kind, ifTrue->fType)
            , fTest(std::move(test))
            , fIfTrue(std::move(ifTrue))
            , fIfFalse(std::move(ifFalse)) {
        ASSERT(fIfTrue->fType == fIfFalse->fType);
    }

    SkString description() const override {
        return "(" + fTest->description() + " ? " + fIfTrue->description() + " : " + 
               fIfFalse->description() + ")";
    }

    const sk_up<Expression> fTest;
    const sk_up<Expression> fIfTrue;
    const sk_up<Expression> fIfFalse;

    typedef Expression INHERITED;
};

} // namespace

#endif
