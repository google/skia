/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <memory>

#include "src/sksl/SkSLDefinitionMap.h"

#include "src/sksl/SkSLCFGGenerator.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"

namespace SkSL {

/**
 * Maps all local variables in the function to null, indicating that their value is initially
 * unknown.
 */
void DefinitionMap::computeStartState(const CFG& cfg) {
    fDefinitions.reset();

    for (const BasicBlock& block : cfg.fBlocks) {
        for (const BasicBlock::Node& node : block.fNodes) {
            if (node.isStatement()) {
                const Statement* s = node.statement()->get();
                if (s->is<VarDeclaration>()) {
                    fDefinitions[&s->as<VarDeclaration>().var()] = nullptr;
                }
            }
        }
    }
}

Expression* DefinitionMap::getKnownDefinition(const Variable* var) const {
    // Find this variable in the definition map.
    std::unique_ptr<Expression>** exprPtr = fDefinitions.find(var);
    if (!exprPtr || !*exprPtr) {
        return nullptr;
    }
    // Return known expressions only; return "null" for defined-but-unknown expressions.
    Expression* expr = (*exprPtr)->get();
    return (expr->kind() != Expression::Kind::kDefined) ? expr : nullptr;
}

// Add compile-time constants to the definition map. Non-compile-time constant expressions are saved
// as just "defined" since we can't guarantee that the expression will remain alive until the end of
// optimization.
void DefinitionMap::addDefinition(const Context& context, const Variable* var,
                                  std::unique_ptr<Expression>* expr) {
    fDefinitions.set(var, (*expr)->isCompileTimeConstant()
                                  ? expr
                                  : (std::unique_ptr<Expression>*)&context.fDefined_Expression);
}

// Add the definition created by assigning to the lvalue to the definition map.
void DefinitionMap::addDefinition(const Context& context, const Expression* lvalue,
                                  std::unique_ptr<Expression>* expr) {
    switch (lvalue->kind()) {
        case Expression::Kind::kVariableReference: {
            const Variable& var = *lvalue->as<VariableReference>().variable();
            if (var.storage() == Variable::Storage::kLocal) {
                this->addDefinition(context, &var, expr);
            }
            break;
        }
        case Expression::Kind::kSwizzle:
            // We consider the variable written to as long as at least some of its components have
            // been written to. This will lead to some false negatives (we won't catch it if you
            // write to foo.x and then read foo.y), but being stricter could lead to false positives
            // (we write to foo.x, and then pass foo to a function which happens to only read foo.x,
            // but since we pass foo as a whole it is flagged as an error) unless we perform a much
            // more complicated whole-program analysis. This is probably good enough.
            this->addDefinition(context, lvalue->as<Swizzle>().base().get(),
                                (std::unique_ptr<Expression>*)&context.fDefined_Expression);
            break;

        case Expression::Kind::kIndex:
            // see comments in Swizzle
            this->addDefinition(context, lvalue->as<IndexExpression>().base().get(),
                                (std::unique_ptr<Expression>*)&context.fDefined_Expression);
            break;

        case Expression::Kind::kFieldAccess:
            // see comments in Swizzle
            this->addDefinition(context, lvalue->as<FieldAccess>().base().get(),
                                (std::unique_ptr<Expression>*)&context.fDefined_Expression);
            break;

        case Expression::Kind::kTernary:
            // To simplify analysis, we just pretend that we write to both sides of the ternary.
            // This allows for false positives (meaning we fail to detect that a variable might not
            // have been assigned), but is preferable to false negatives.
            this->addDefinition(context, lvalue->as<TernaryExpression>().ifTrue().get(),
                                (std::unique_ptr<Expression>*)&context.fDefined_Expression);
            this->addDefinition(context, lvalue->as<TernaryExpression>().ifFalse().get(),
                                (std::unique_ptr<Expression>*)&context.fDefined_Expression);
            break;

        default:
            SkDEBUGFAILF("expression is not an lvalue: %s", lvalue->description().c_str());
            break;
    }
}

// Add local variables defined by this node to the map.
void DefinitionMap::addDefinitions(const Context& context, const BasicBlock::Node& node) {
    if (node.isExpression()) {
        Expression* expr = node.expression()->get();
        switch (expr->kind()) {
            case Expression::Kind::kBinary: {
                BinaryExpression* b = &expr->as<BinaryExpression>();
                if (b->getOperator() == Token::Kind::TK_EQ) {
                    this->addDefinition(context, b->left().get(), &b->right());
                } else if (Operators::IsAssignment(b->getOperator())) {
                    this->addDefinition(
                            context, b->left().get(),
                            (std::unique_ptr<Expression>*)&context.fDefined_Expression);
                }
                break;
            }
            case Expression::Kind::kFunctionCall: {
                const FunctionCall& c = expr->as<FunctionCall>();
                const std::vector<const Variable*>& parameters = c.function().parameters();
                for (size_t i = 0; i < parameters.size(); ++i) {
                    if (parameters[i]->modifiers().fFlags & Modifiers::kOut_Flag) {
                        this->addDefinition(
                                context, c.arguments()[i].get(),
                                (std::unique_ptr<Expression>*)&context.fDefined_Expression);
                    }
                }
                break;
            }
            case Expression::Kind::kPrefix: {
                const PrefixExpression* p = &expr->as<PrefixExpression>();
                if (p->getOperator() == Token::Kind::TK_MINUSMINUS ||
                    p->getOperator() == Token::Kind::TK_PLUSPLUS) {
                    this->addDefinition(
                            context, p->operand().get(),
                            (std::unique_ptr<Expression>*)&context.fDefined_Expression);
                }
                break;
            }
            case Expression::Kind::kPostfix: {
                const PostfixExpression* p = &expr->as<PostfixExpression>();
                if (p->getOperator() == Token::Kind::TK_MINUSMINUS ||
                    p->getOperator() == Token::Kind::TK_PLUSPLUS) {
                    this->addDefinition(
                            context, p->operand().get(),
                            (std::unique_ptr<Expression>*)&context.fDefined_Expression);
                }
                break;
            }
            case Expression::Kind::kVariableReference: {
                const VariableReference* v = &expr->as<VariableReference>();
                if (v->refKind() != VariableReference::RefKind::kRead) {
                    this->addDefinition(
                            context, v,
                            (std::unique_ptr<Expression>*)&context.fDefined_Expression);
                }
                break;
            }
            default:
                break;
        }
    } else if (node.isStatement()) {
        Statement* stmt = node.statement()->get();
        if (stmt->is<VarDeclaration>()) {
            VarDeclaration& vd = stmt->as<VarDeclaration>();
            if (vd.value()) {
                this->addDefinition(context, &vd.var(), &vd.value());
            }
        }
    }
}

}  // namespace SkSL
