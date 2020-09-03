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

static std::vector<std::unique_ptr<Expression>> make_vector(std::unique_ptr<Expression> left,
                                                            std::unique_ptr<Expression> right) {
    std::vector<std::unique_ptr<Expression>> result;
    result.push_back(std::move(left));
    result.push_back(std::move(right));
    return result;
}

/**
 * A binary operation.
 */
struct BinaryExpression : public Expression {
    static constexpr Kind kExpressionKind = Kind::kBinary;

    BinaryExpression(int offset, std::unique_ptr<Expression> left, Token::Kind op,
                     std::unique_ptr<Expression> right, const Type* type)
    : INHERITED(offset, kExpressionKind, TypeTokenData{type, op}, make_vector(std::move(left),
                                                                              std::move(right))) {
        // If we are assigning to a VariableReference, ensure that it is set to Write or ReadWrite
        SkASSERT(!Compiler::IsAssignment(op) || check_ref(&this->left()));
    }

    Expression& left() const {
        return this->expressionChild(0);
    }

    std::unique_ptr<Expression>& leftPointer() {
        return this->expressionPointer(0);
    }

    Expression& right() const {
        return this->expressionChild(1);
    }

    std::unique_ptr<Expression>& rightPointer() {
        return this->expressionPointer(1);
    }

    Token::Kind getOperator() const {
        return this->typeTokenData().fToken;
    }

    const Type& type() const override {
        return *this->typeTokenData().fType;
    }

    bool isConstantOrUniform() const override {
        return this->left().isConstantOrUniform() && this->right().isConstantOrUniform();
    }

    std::unique_ptr<Expression> constantPropagate(const IRGenerator& irGenerator,
                                                  const DefinitionMap& definitions) override {
        return irGenerator.constantFold(this->left(),
                                        this->getOperator(),
                                        this->right());
    }

    bool hasProperty(Property property) const override {
        if (property == Property::kSideEffects && Compiler::IsAssignment(this->getOperator())) {
            return true;
        }
        return this->left().hasProperty(property) || this->right().hasProperty(property);
    }

    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<BinaryExpression>(fOffset, this->left().clone(),
                                                  this->getOperator(), this->right().clone(),
                                                  &this->type());
    }

    String description() const override {
        return "(" + this->left().description() + " " +
               Compiler::OperatorName(this->getOperator()) + " " +
               this->right().description() + ")";
    }

    typedef Expression INHERITED;
};

}  // namespace SkSL

#endif
