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
class BinaryExpression final : public Expression {
public:
    static constexpr Kind kExpressionKind = Kind::kBinary;

    BinaryExpression(int offset, std::unique_ptr<Expression> left, Token::Kind op,
                     std::unique_ptr<Expression> right, const Type* type)
    : INHERITED(offset, kExpressionKind, type)
    , fLeft(std::move(left))
    , fOperator(op)
    , fRight(std::move(right)) {
        // If we are assigning to a VariableReference, ensure that it is set to Write or ReadWrite
        SkASSERT(!Compiler::IsAssignment(op) || check_ref(*this->left()));
    }

    std::unique_ptr<Expression>& left() {
        return fLeft;
    }

    const std::unique_ptr<Expression>& left() const {
        return fLeft;
    }

    std::unique_ptr<Expression>& right() {
        return fRight;
    }

    const std::unique_ptr<Expression>& right() const {
        return fRight;
    }

    Token::Kind getOperator() const {
        return fOperator;
    }

    bool isConstantOrUniform() const override {
        return this->left()->isConstantOrUniform() && this->right()->isConstantOrUniform();
    }

    std::unique_ptr<Expression> constantPropagate(const IRGenerator& irGenerator,
                                                  const DefinitionMap& definitions) override {
        return irGenerator.constantFold(*this->left(),
                                        this->getOperator(),
                                        *this->right());
    }

    bool hasProperty(Property property) const override {
        if (property == Property::kSideEffects && Compiler::IsAssignment(this->getOperator())) {
            return true;
        }
        return this->left()->hasProperty(property) || this->right()->hasProperty(property);
    }

    std::unique_ptr<Expression> clone() const override {
        return std::unique_ptr<Expression>(new BinaryExpression(fOffset,
                                                                this->left()->clone(),
                                                                this->getOperator(),
                                                                this->right()->clone(),
                                                                &this->type()));
    }

    String description() const override {
        return "(" + this->left()->description() + " " +
               Compiler::OperatorName(this->getOperator()) + " " + this->right()->description() +
               ")";
    }

private:
    std::unique_ptr<Expression> fLeft;
    Token::Kind fOperator;
    std::unique_ptr<Expression> fRight;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
