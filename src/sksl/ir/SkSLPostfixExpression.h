/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_POSTFIXEXPRESSION
#define SKSL_POSTFIXEXPRESSION

#include "src/sksl/SkSLOperator.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLIRNode.h"

#include <memory>
#include <string>
#include <utility>

namespace SkSL {

class Context;

/**
 * An expression modified by a unary operator appearing after it, such as 'i++'.
 */
class PostfixExpression final : public Expression {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kPostfix;

    PostfixExpression(Position pos, std::unique_ptr<Expression> operand, Operator op)
            : INHERITED(pos, kIRNodeKind, &operand->type())
            , fOperand(std::move(operand))
            , fOperator(op) {}

    // Creates an SkSL postfix expression; uses the ErrorReporter to report errors.
    static std::unique_ptr<Expression> Convert(const Context& context,
                                               Position pos,
                                               std::unique_ptr<Expression> base,
                                               Operator op);

    // Creates an SkSL postfix expression; reports errors via ASSERT.
    static std::unique_ptr<Expression> Make(const Context& context,
                                            Position pos,
                                            std::unique_ptr<Expression> base,
                                            Operator op);

    Operator getOperator() const {
        return fOperator;
    }

    std::unique_ptr<Expression>& operand() {
        return fOperand;
    }

    const std::unique_ptr<Expression>& operand() const {
        return fOperand;
    }

    std::unique_ptr<Expression> clone(Position pos) const override {
        return std::make_unique<PostfixExpression>(pos, this->operand()->clone(),
                                                   this->getOperator());
    }

    std::string description(OperatorPrecedence parentPrecedence) const override;

private:
    std::unique_ptr<Expression> fOperand;
    Operator fOperator;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
