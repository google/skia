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

static inline bool check_ref(const Expression& expr) {
    switch (expr.kind()) {
        case Expression::Kind::kExternalValue:
            return true;
        case Expression::Kind::kFieldAccess:
            return check_ref(*expr.as<FieldAccess>().base());
        case Expression::Kind::kIndex:
            return check_ref(*expr.as<IndexExpression>().base());
        case Expression::Kind::kSwizzle:
            return check_ref(*expr.as<Swizzle>().base());
        case Expression::Kind::kTernary: {
            const TernaryExpression& t = expr.as<TernaryExpression>();
            return check_ref(*t.ifTrue()) && check_ref(*t.ifFalse());
        }
        case Expression::Kind::kVariableReference: {
            const VariableReference& ref = expr.as<VariableReference>();
            return ref.refKind() == VariableReference::RefKind::kWrite ||
                   ref.refKind() == VariableReference::RefKind::kReadWrite;
        }
        default:
            return false;
    }
}

/**
 * A binary operation.
 */
class BinaryExpression : public Expression {
public:
    static constexpr Kind kExpressionKind = Kind::kBinary;

    BinaryExpression(int offset, std::unique_ptr<Expression> left, Token::Kind op,
                     std::unique_ptr<Expression> right, const Type* type)
    : INHERITED(offset, kExpressionKind, TypeTokenData{type, op}) {
        fExpressionChildren.reserve_back(2);
        fExpressionChildren.push_back(std::move(left));
        fExpressionChildren.push_back(std::move(right));
        // If we are assigning to a VariableReference, ensure that it is set to Write or ReadWrite
        SkASSERT(!Compiler::IsAssignment(op) || check_ref(this->left()));
    }

    const Type& type() const override {
        return *this->typeTokenData().fType;
    }

    Expression& left() const {
        return this->expressionChild(0);
    }

    std::unique_ptr<Expression>& leftPointer() {
        return this->expressionPointer(0);
    }

    const std::unique_ptr<Expression>& leftPointer() const {
        return this->expressionPointer(0);
    }

    Expression& right() const {
        return this->expressionChild(1);
    }

    std::unique_ptr<Expression>& rightPointer() {
        return this->expressionPointer(1);
    }

    const std::unique_ptr<Expression>& rightPointer() const {
        return this->expressionPointer(1);
    }

    Token::Kind getOperator() const {
        return this->typeTokenData().fToken;
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
        return std::unique_ptr<Expression>(new BinaryExpression(fOffset,
                                                                this->left().clone(),
                                                                this->getOperator(),
                                                                this->right().clone(),
                                                                &this->type()));
    }

    String description() const override {
        return "(" + this->left().description() + " " +
               Compiler::OperatorName(this->getOperator()) + " " + this->right().description() +
               ")";
    }

private:
    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
