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
class TernaryExpression : public Expression {
public:
    static constexpr Kind kExpressionKind = Kind::kTernary;

    TernaryExpression(int offset, std::unique_ptr<Expression> test,
                      std::unique_ptr<Expression> ifTrue, std::unique_ptr<Expression> ifFalse)
    : INHERITED(offset, kExpressionKind, &ifTrue->type()) {
        SkASSERT(ifTrue->type() == ifFalse->type());
        fExpressionChildren.reserve_back(3);
        fExpressionChildren.push_back(std::move(test));
        fExpressionChildren.push_back(std::move(ifTrue));
        fExpressionChildren.push_back(std::move(ifFalse));
    }

    std::unique_ptr<Expression>& test() {
        return fExpressionChildren[0];
    }

    const std::unique_ptr<Expression>& test() const {
        return fExpressionChildren[0];
    }

    std::unique_ptr<Expression>& ifTrue() {
        return fExpressionChildren[1];
    }

    const std::unique_ptr<Expression>& ifTrue() const {
        return fExpressionChildren[1];
    }

    std::unique_ptr<Expression>& ifFalse() {
        return fExpressionChildren[2];
    }

    const std::unique_ptr<Expression>& ifFalse() const {
        return fExpressionChildren[2];
    }

    bool hasProperty(Property property) const override {
        return this->test()->hasProperty(property) || this->ifTrue()->hasProperty(property) ||
               this->ifFalse()->hasProperty(property);
    }

    bool isConstantOrUniform() const override {
        return this->test()->isConstantOrUniform() && this->ifTrue()->isConstantOrUniform() &&
               this->ifFalse()->isConstantOrUniform();
    }

    std::unique_ptr<Expression> clone() const override {
        return std::unique_ptr<Expression>(new TernaryExpression(fOffset, this->test()->clone(),
                                                                 this->ifTrue()->clone(),
                                                                 this->ifFalse()->clone()));
    }

    String description() const override {
        return "(" + this->test()->description() + " ? " + this->ifTrue()->description() + " : " +
               this->ifFalse()->description() + ")";
    }

private:
    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
