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
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"

namespace SkSL {

static inline bool check_ref(Expression* expr) {
    switch (expr->kind()) {
        case Expression::Kind::kExternalValue:
            return true;
        case Expression::Kind::kFieldAccess:
            return check_ref(((FieldAccess*) expr)->fBase.get());
        case Expression::Kind::kIndex:
            return check_ref(((IndexExpression*) expr)->fBase.get());
        case Expression::Kind::kSwizzle:
            return check_ref(((Swizzle*) expr)->fBase.get());
        case Expression::Kind::kTernary: {
            TernaryExpression* t = (TernaryExpression*) expr;
            return check_ref(t->fIfTrue.get()) && check_ref(t->fIfFalse.get());
        }
        case Expression::Kind::kVariableReference: {
            VariableReference* ref = (VariableReference*) expr;
            return ref->fRefKind == VariableReference::kWrite_RefKind ||
                   ref->fRefKind == VariableReference::kReadWrite_RefKind;
        }
        default:
            return false;
    }
}

/**
 * A binary operation.
 */
struct BinaryExpression : public Expression {
    static constexpr Kind kExpressionKind = Kind::kBinary;

    BinaryExpression(int offset, std::unique_ptr<Expression> left, Token::Kind op,
                     std::unique_ptr<Expression> right, const Type& type)
    : INHERITED(offset, kExpressionKind, type)
    , fLeft(std::move(left))
    , fOperator(op)
    , fRight(std::move(right)) {
        // If we are assigning to a VariableReference, ensure that it is set to Write or ReadWrite
        SkASSERT(!Compiler::IsAssignment(op) || check_ref(fLeft.get()));
    }

    bool isConstantOrUniform() const override {
        return fLeft->isConstantOrUniform() && fRight->isConstantOrUniform();
    }

    std::unique_ptr<Expression> constantPropagate(const IRGenerator& irGenerator,
                                                  const DefinitionMap& definitions) override {
        return irGenerator.constantFold(*fLeft,
                                        fOperator,
                                        *fRight);
    }

    bool hasProperty(Property property) const override {
        if (property == Property::kSideEffects && Compiler::IsAssignment(fOperator)) {
            return true;
        }
        return fLeft->hasProperty(property) || fRight->hasProperty(property);
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

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
