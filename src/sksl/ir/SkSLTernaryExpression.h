/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_TERNARYEXPRESSION
#define SKSL_TERNARYEXPRESSION

#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * A ternary expression (test ? ifTrue : ifFalse).
 */
class TernaryExpression final : public Expression {
public:
    inline static constexpr Kind kExpressionKind = Kind::kTernary;

    TernaryExpression(int line, std::unique_ptr<Expression> test,
                      std::unique_ptr<Expression> ifTrue, std::unique_ptr<Expression> ifFalse)
        : INHERITED(line, kExpressionKind, &ifTrue->type())
        , fTest(std::move(test))
        , fIfTrue(std::move(ifTrue))
        , fIfFalse(std::move(ifFalse)) {
        SkASSERT(this->ifTrue()->type() == this->ifFalse()->type());
    }

    // Creates a potentially-simplified form of the ternary. Typechecks and coerces input
    // expressions; reports errors via ErrorReporter.
    static std::unique_ptr<Expression> Convert(const Context& context,
                                            std::unique_ptr<Expression> test,
                                            std::unique_ptr<Expression> ifTrue,
                                            std::unique_ptr<Expression> ifFalse);

    // Creates a potentially-simplified form of the ternary; reports errors via ASSERT.
    static std::unique_ptr<Expression> Make(const Context& context,
                                            std::unique_ptr<Expression> test,
                                            std::unique_ptr<Expression> ifTrue,
                                            std::unique_ptr<Expression> ifFalse);

    std::unique_ptr<Expression>& test() {
        return fTest;
    }

    const std::unique_ptr<Expression>& test() const {
        return fTest;
    }

    std::unique_ptr<Expression>& ifTrue() {
        return fIfTrue;
    }

    const std::unique_ptr<Expression>& ifTrue() const {
        return fIfTrue;
    }

    std::unique_ptr<Expression>& ifFalse() {
        return fIfFalse;
    }

    const std::unique_ptr<Expression>& ifFalse() const {
        return fIfFalse;
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
        return std::make_unique<TernaryExpression>(fLine, this->test()->clone(),
                                                   this->ifTrue()->clone(),
                                                   this->ifFalse()->clone());
    }

    String description() const override {
        return "(" + this->test()->description() + " ? " + this->ifTrue()->description() + " : " +
               this->ifFalse()->description() + ")";
    }

private:
    std::unique_ptr<Expression> fTest;
    std::unique_ptr<Expression> fIfTrue;
    std::unique_ptr<Expression> fIfFalse;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
