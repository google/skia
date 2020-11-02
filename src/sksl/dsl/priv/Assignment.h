/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_ASSIGNMENT
#define SKSL_DSL_ASSIGNMENT

#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/dsl/priv/LValue.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"

namespace skslcode {

template<class Left, class Right, class Type>
class Assignment : public Type {
public:
    Assignment(Left left, Right right)
        : fLeft(std::move(left))
        , fRight(std::move(right)) {}

    std::unique_ptr<SkSL::Expression> expression() const {
        return std::make_unique<SkSL::BinaryExpression>(/*offset=*/-1,
                                                        fLeft.lvalue(),
                                                        SkSL::Token::Kind::TK_EQ,
                                                        fRight.expression(),
                                                        &this->type());
    }

    std::unique_ptr<SkSL::Statement> statement() const {
        return std::make_unique<SkSL::ExpressionStatement>(this->expression());
    }

private:
    Left fLeft;
    Right fRight;
};

} // namespace skslcode

#endif
