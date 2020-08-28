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
    static constexpr Kind kIRNodeKind = kTernary_Kind;

    TernaryExpression(int offset, std::unique_ptr<Expression> test,
                      std::unique_ptr<Expression> ifTrue, std::unique_ptr<Expression> ifFalse)
    : INHERITED(offset, kIRNodeKind, ifTrue->fType)
    , fTest(std::move(test))
    , fIfTrue(std::move(ifTrue))
    , fIfFalse(std::move(ifFalse)) {
        SkASSERT(fIfTrue->fType == fIfFalse->fType);
    }

    bool hasProperty(Property property) const override {
        return fTest->hasProperty(property) || fIfTrue->hasProperty(property) ||
               fIfFalse->hasProperty(property);
    }

    bool isConstantOrUniform() const override {
        return fTest->isConstantOrUniform() && fIfTrue->isConstantOrUniform() &&
               fIfFalse->isConstantOrUniform();
    }

    std::unique_ptr<IRNode> clone() const override {
        return std::unique_ptr<IRNode>(new TernaryExpression(fOffset, fTest->cloneExpression(),
                                                             fIfTrue->cloneExpression(),
                                                             fIfFalse->cloneExpression()));
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

}  // namespace SkSL

#endif
