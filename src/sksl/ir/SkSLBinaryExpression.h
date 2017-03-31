/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_BINARYEXPRESSION
#define SKSL_BINARYEXPRESSION

#include "SkSLExpression.h"
#include "SkSLExpression.h"
#include "../SkSLIRGenerator.h"
#include "../SkSLToken.h"

namespace SkSL {

/**
 * A binary operation.
 */
struct BinaryExpression : public Expression {
    BinaryExpression(Position position, std::unique_ptr<Expression> left, Token::Kind op,
                     std::unique_ptr<Expression> right, const Type& type)
    : INHERITED(position, kBinary_Kind, type)
    , fLeft(std::move(left))
    , fOperator(op)
    , fRight(std::move(right)) {}

    virtual std::unique_ptr<Expression> constantPropagate(
                                                        const IRGenerator& irGenerator,
                                                        const DefinitionMap& definitions) override {
        return irGenerator.constantFold(*fLeft,
                                        fOperator,
                                        *fRight);
    }

    virtual String description() const override {
        return "(" + fLeft->description() + " " + Token::OperatorName(fOperator) + " " +
               fRight->description() + ")";
    }

    std::unique_ptr<Expression> fLeft;
    const Token::Kind fOperator;
    std::unique_ptr<Expression> fRight;

    typedef Expression INHERITED;
};

} // namespace

#endif
