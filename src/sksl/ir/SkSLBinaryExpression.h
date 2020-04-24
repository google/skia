/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_BINARYEXPRESSION
#define SKSL_BINARYEXPRESSION

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/SkSLLexer.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * A binary operation.
 */
struct BinaryExpression : public Expression {
    BinaryExpression(int offset, std::unique_ptr<Expression> left, Token::Kind op,
                     std::unique_ptr<Expression> right, const Type& type)
    : INHERITED(offset, kBinary_Kind, type)
    , fLeft(std::move(left))
    , fOperator(op)
    , fRight(std::move(right)) {}

    std::unique_ptr<Expression> constantPropagate(const IRGenerator& irGenerator,
                                                  const DefinitionMap& definitions) override {
        return irGenerator.constantFold(*fLeft,
                                        fOperator,
                                        *fRight);
    }

    bool hasSideEffects() const override {
        return Compiler::IsAssignment(fOperator) || fLeft->hasSideEffects() ||
               fRight->hasSideEffects();
    }

    std::unique_ptr<Expression> clone() const override {
        return std::unique_ptr<Expression>(new BinaryExpression(fOffset, fLeft->clone(), fOperator,
                                                                fRight->clone(), fType));
    }

    String description() const override {
        return "(" + fLeft->description() + " " + Compiler::OperatorName(fOperator) + " " +
               fRight->description() + ")";
    }

    std::unique_ptr<Expression> fLeft;
    const Token::Kind fOperator;
    std::unique_ptr<Expression> fRight;

    typedef Expression INHERITED;
};

} // namespace

#endif
