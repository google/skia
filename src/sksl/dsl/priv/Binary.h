/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_BINARY
#define SKSL_DSL_BINARY

#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/dsl/priv/LValue.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"

namespace skslcode {

template<class Left, class Right, class Type>
class Binary : public Type {
public:
    Binary(Left left, SkSL::Token::Kind op, Right right)
        : fLeft(std::move(left))
        , fRight(std::move(right))
        , fOp(op) {}

    std::unique_ptr<SkSL::Expression> expression() const {
        return std::make_unique<SkSL::BinaryExpression>(/*offset=*/-1,
                                                        fLeft.expression(),
                                                        fOp,
                                                        fRight.expression(),
                                                        &this->type());
    }

    std::unique_ptr<SkSL::Statement> statement() const {
        return std::make_unique<SkSL::ExpressionStatement>(this->expression());
    }

private:
    Left fLeft;
    Right fRight;
    SkSL::Token::Kind fOp;
};

} // namespace skslcode

#endif
