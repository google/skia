/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLCFGGenerator.h"

#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLExternalFunctionCall.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLVarDeclarationsStatement.h"
#include "src/sksl/ir/SkSLWhileStatement.h"

namespace SkSL {

BlockId CFG::newBlock() {
    BlockId result = fBlocks.size();
    fBlocks.emplace_back();
    if (fBlocks.size() > 1) {
        this->addExit(fCurrent, result);
    }
    fCurrent = result;
    return result;
}

BlockId CFG::newIsolatedBlock() {
    BlockId result = fBlocks.size();
    fBlocks.emplace_back();
    return result;
}

void CFG::addExit(BlockId from, BlockId to) {
    if (from == 0 || fBlocks[from].fEntrances.size()) {
        fBlocks[from].fExits.insert(to);
        fBlocks[to].fEntrances.insert(from);
    }
}

#ifdef SK_DEBUG
void CFG::dump() {
    for (size_t i = 0; i < fBlocks.size(); i++) {
        printf("Block %d\n-------\nBefore: ", (int) i);
        const char* separator = "";
        for (auto iter = fBlocks[i].fBefore.begin(); iter != fBlocks[i].fBefore.end(); iter++) {
            printf("%s%s = %s", separator, iter->first->description().c_str(),
                   iter->second ? (*iter->second)->description().c_str() : "<undefined>");
            separator = ", ";
        }
        printf("\nEntrances: ");
        separator = "";
        for (BlockId b : fBlocks[i].fEntrances) {
            printf("%s%d", separator, (int) b);
            separator = ", ";
        }
        printf("\n");
        for (size_t j = 0; j < fBlocks[i].fNodes.size(); j++) {
            BasicBlock::Node& n = fBlocks[i].fNodes[j];
            printf("Node %d (%p): %s\n", (int) j, &n, n.fKind == BasicBlock::Node::kExpression_Kind
                                                         ? (*n.expression())->description().c_str()
                                                         : (*n.statement())->description().c_str());
        }
        printf("Exits: ");
        separator = "";
        for (BlockId b : fBlocks[i].fExits) {
            printf("%s%d", separator, (int) b);
            separator = ", ";
        }
        printf("\n\n");
    }
}
#endif

bool BasicBlock::tryRemoveExpressionBefore(std::vector<BasicBlock::Node>::iterator* iter,
                                           Expression* e) {
    if (e->fKind == Expression::kTernary_Kind) {
        return false;
    }
    bool result;
    if ((*iter)->fKind == BasicBlock::Node::kExpression_Kind) {
        SkASSERT((*iter)->expression()->get() != e);
        Expression* old = (*iter)->expression()->get();
        do {
            if ((*iter) == fNodes.begin()) {
                return false;
            }
            --(*iter);
        } while ((*iter)->fKind != BasicBlock::Node::kExpression_Kind ||
                 (*iter)->expression()->get() != e);
        result = this->tryRemoveExpression(iter);
        while ((*iter)->fKind != BasicBlock::Node::kExpression_Kind ||
               (*iter)->expression()->get() != old) {
            SkASSERT(*iter != fNodes.end());
            ++(*iter);
        }
    } else {
        Statement* old = (*iter)->statement()->get();
        do {
            if ((*iter) == fNodes.begin()) {
                return false;
            }
            --(*iter);
        } while ((*iter)->fKind != BasicBlock::Node::kExpression_Kind ||
                 (*iter)->expression()->get() != e);
        result = this->tryRemoveExpression(iter);
        while ((*iter)->fKind != BasicBlock::Node::kStatement_Kind ||
               (*iter)->statement()->get() != old) {
            SkASSERT(*iter != fNodes.end());
            ++(*iter);
        }
    }
    return result;
}

bool BasicBlock::tryRemoveLValueBefore(std::vector<BasicBlock::Node>::iterator* iter,
                                       Expression* lvalue) {
    switch (lvalue->fKind) {
        case Expression::kExternalValue_Kind: // fall through
        case Expression::kVariableReference_Kind:
            return true;
        case Expression::kSwizzle_Kind:
            return this->tryRemoveLValueBefore(iter, lvalue->as<Swizzle>().fBase.get());
        case Expression::kFieldAccess_Kind:
            return this->tryRemoveLValueBefore(iter, lvalue->as<FieldAccess>().fBase.get());
        case Expression::kIndex_Kind: {
            IndexExpression& indexExpr = lvalue->as<IndexExpression>();
            if (!this->tryRemoveLValueBefore(iter, indexExpr.fBase.get())) {
                return false;
            }
            return this->tryRemoveExpressionBefore(iter, indexExpr.fIndex.get());
        }
        case Expression::kTernary_Kind: {
            TernaryExpression& ternary = lvalue->as<TernaryExpression>();
            if (!this->tryRemoveExpressionBefore(iter, ternary.fTest.get())) {
                return false;
            }
            if (!this->tryRemoveLValueBefore(iter, ternary.fIfTrue.get())) {
                return false;
            }
            return this->tryRemoveLValueBefore(iter, ternary.fIfFalse.get());
        }
        default:
#ifdef SK_DEBUG
            ABORT("invalid lvalue: %s\n", lvalue->description().c_str());
#endif
            return false;
    }
}

bool BasicBlock::tryRemoveExpression(std::vector<BasicBlock::Node>::iterator* iter) {
    Expression* expr = (*iter)->expression()->get();
    switch (expr->fKind) {
        case Expression::kBinary_Kind: {
            BinaryExpression& b = expr->as<BinaryExpression>();
            if (b.fOperator == Token::Kind::TK_EQ) {
                if (!this->tryRemoveLValueBefore(iter, b.fLeft.get())) {
                    return false;
                }
            } else if (!this->tryRemoveExpressionBefore(iter, b.fLeft.get())) {
                return false;
            }
            if (!this->tryRemoveExpressionBefore(iter, b.fRight.get())) {
                return false;
            }
            SkASSERT((*iter)->expression()->get() == expr);
            *iter = fNodes.erase(*iter);
            return true;
        }
        case Expression::kTernary_Kind: {
            // ternaries cross basic block boundaries, must regenerate the CFG to remove it
            return false;
        }
        case Expression::kFieldAccess_Kind: {
            FieldAccess& f = expr->as<FieldAccess>();
            if (!this->tryRemoveExpressionBefore(iter, f.fBase.get())) {
                return false;
            }
            *iter = fNodes.erase(*iter);
            return true;
        }
        case Expression::kSwizzle_Kind: {
            Swizzle& s = expr->as<Swizzle>();
            if (s.fBase && !this->tryRemoveExpressionBefore(iter, s.fBase.get())) {
                return false;
            }
            *iter = fNodes.erase(*iter);
            return true;
        }
        case Expression::kIndex_Kind: {
            IndexExpression& idx = expr->as<IndexExpression>();
            if (!this->tryRemoveExpressionBefore(iter, idx.fBase.get())) {
                return false;
            }
            if (!this->tryRemoveExpressionBefore(iter, idx.fIndex.get())) {
                return false;
            }
            *iter = fNodes.erase(*iter);
            return true;
        }
        case Expression::kConstructor_Kind: {
            Constructor& c = expr->as<Constructor>();
            for (auto& arg : c.fArguments) {
                if (!this->tryRemoveExpressionBefore(iter, arg.get())) {
                    return false;
                }
                SkASSERT((*iter)->expression()->get() == expr);
            }
            *iter = fNodes.erase(*iter);
            return true;
        }
        case Expression::kFunctionCall_Kind: {
            FunctionCall& f = expr->as<FunctionCall>();
            for (auto& arg : f.fArguments) {
                if (!this->tryRemoveExpressionBefore(iter, arg.get())) {
                    return false;
                }
                SkASSERT((*iter)->expression()->get() == expr);
            }
            *iter = fNodes.erase(*iter);
            return true;
        }
        case Expression::kPrefix_Kind:
            if (!this->tryRemoveExpressionBefore(iter,
                                                 expr->as<PrefixExpression>().fOperand.get())) {
                return false;
            }
            *iter = fNodes.erase(*iter);
            return true;
        case Expression::kPostfix_Kind:
            if (!this->tryRemoveExpressionBefore(iter,
                                                 expr->as<PrefixExpression>().fOperand.get())) {
                return false;
            }
            *iter = fNodes.erase(*iter);
            return true;
        case Expression::kBoolLiteral_Kind:  // fall through
        case Expression::kFloatLiteral_Kind: // fall through
        case Expression::kIntLiteral_Kind:   // fall through
        case Expression::kSetting_Kind:      // fall through
        case Expression::kVariableReference_Kind:
            *iter = fNodes.erase(*iter);
            return true;
        default:
#ifdef SK_DEBUG
            ABORT("unhandled expression: %s\n", expr->description().c_str());
#endif
            return false;
    }
}

bool BasicBlock::tryInsertExpression(std::vector<BasicBlock::Node>::iterator* iter,
                                     std::unique_ptr<Expression>* expr) {
    switch ((*expr)->fKind) {
        case Expression::kBinary_Kind: {
            BinaryExpression& b = expr->get()->as<BinaryExpression>();
            if (!this->tryInsertExpression(iter, &b.fRight)) {
                return false;
            }
            ++(*iter);
            if (!this->tryInsertExpression(iter, &b.fLeft)) {
                return false;
            }
            ++(*iter);
            BasicBlock::Node node = { BasicBlock::Node::kExpression_Kind, true, expr, nullptr };
            *iter = fNodes.insert(*iter, node);
            return true;
        }
        case Expression::kBoolLiteral_Kind:  // fall through
        case Expression::kFloatLiteral_Kind: // fall through
        case Expression::kIntLiteral_Kind:   // fall through
        case Expression::kVariableReference_Kind: {
            BasicBlock::Node node = { BasicBlock::Node::kExpression_Kind, true, expr, nullptr };
            *iter = fNodes.insert(*iter, node);
            return true;
        }
        case Expression::kConstructor_Kind: {
            Constructor& c = expr->get()->as<Constructor>();
            for (auto& arg : c.fArguments) {
                if (!this->tryInsertExpression(iter, &arg)) {
                    return false;
                }
                ++(*iter);
            }
            BasicBlock::Node node = { BasicBlock::Node::kExpression_Kind, true, expr, nullptr };
            *iter = fNodes.insert(*iter, node);
            return true;
        }
        case Expression::kSwizzle_Kind: {
            Swizzle& s = expr->get()->as<Swizzle>();
            if (!this->tryInsertExpression(iter, &s.fBase)) {
                return false;
            }
            ++(*iter);
            BasicBlock::Node node = { BasicBlock::Node::kExpression_Kind, true, expr, nullptr };
            *iter = fNodes.insert(*iter, node);
            return true;
        }
        default:
            return false;
    }
}

void CFGGenerator::addExpression(CFG& cfg, std::unique_ptr<Expression>* e, bool constantPropagate) {
    SkASSERT(e);
    switch ((*e)->fKind) {
        case Expression::kBinary_Kind: {
            BinaryExpression& b = e->get()->as<BinaryExpression>();
            switch (b.fOperator) {
                case Token::Kind::TK_LOGICALAND: // fall through
                case Token::Kind::TK_LOGICALOR: {
                    // this isn't as precise as it could be -- we don't bother to track that if we
                    // early exit from a logical and/or, we know which branch of an 'if' we're going
                    // to hit -- but it won't make much difference in practice.
                    this->addExpression(cfg, &b.fLeft, constantPropagate);
                    BlockId start = cfg.fCurrent;
                    cfg.newBlock();
                    this->addExpression(cfg, &b.fRight, constantPropagate);
                    cfg.newBlock();
                    cfg.addExit(start, cfg.fCurrent);
                    cfg.fBlocks[cfg.fCurrent].fNodes.push_back({
                        BasicBlock::Node::kExpression_Kind,
                        constantPropagate,
                        e,
                        nullptr
                    });
                    break;
                }
                case Token::Kind::TK_EQ: {
                    this->addExpression(cfg, &b.fRight, constantPropagate);
                    this->addLValue(cfg, &b.fLeft);
                    cfg.fBlocks[cfg.fCurrent].fNodes.push_back({
                        BasicBlock::Node::kExpression_Kind,
                        constantPropagate,
                        e,
                        nullptr
                    });
                    break;
                }
                default:
                    this->addExpression(cfg, &b.fLeft, !Compiler::IsAssignment(b.fOperator));
                    this->addExpression(cfg, &b.fRight, constantPropagate);
                    cfg.fBlocks[cfg.fCurrent].fNodes.push_back({
                        BasicBlock::Node::kExpression_Kind,
                        constantPropagate,
                        e,
                        nullptr
                    });
            }
            break;
        }
        case Expression::kConstructor_Kind: {
            Constructor& c = e->get()->as<Constructor>();
            for (auto& arg : c.fArguments) {
                this->addExpression(cfg, &arg, constantPropagate);
            }
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kExpression_Kind,
                                                         constantPropagate, e, nullptr });
            break;
        }
        case Expression::kExternalFunctionCall_Kind: {
            ExternalFunctionCall& c = e->get()->as<ExternalFunctionCall>();
            for (auto& arg : c.fArguments) {
                this->addExpression(cfg, &arg, constantPropagate);
            }
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kExpression_Kind,
                                                         constantPropagate, e, nullptr });
            break;
        }
        case Expression::kFunctionCall_Kind: {
            FunctionCall& c = e->get()->as<FunctionCall>();
            for (auto& arg : c.fArguments) {
                this->addExpression(cfg, &arg, constantPropagate);
            }
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kExpression_Kind,
                                                         constantPropagate, e, nullptr });
            break;
        }
        case Expression::kFieldAccess_Kind:
            this->addExpression(cfg, &e->get()->as<FieldAccess>().fBase, constantPropagate);
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kExpression_Kind,
                                                         constantPropagate, e, nullptr });
            break;
        case Expression::kIndex_Kind: {
            IndexExpression& indexExpr = e->get()->as<IndexExpression>();

            this->addExpression(cfg, &indexExpr.fBase, constantPropagate);
            this->addExpression(cfg, &indexExpr.fIndex, constantPropagate);
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kExpression_Kind,
                                                         constantPropagate, e, nullptr });
            break;
        }
        case Expression::kPrefix_Kind: {
            PrefixExpression& p = e->get()->as<PrefixExpression>();
            this->addExpression(cfg, &p.fOperand, constantPropagate &&
                                                  p.fOperator != Token::Kind::TK_PLUSPLUS &&
                                                  p.fOperator != Token::Kind::TK_MINUSMINUS);
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kExpression_Kind,
                                                         constantPropagate, e, nullptr });
            break;
        }
        case Expression::kPostfix_Kind:
            this->addExpression(cfg, &e->get()->as<PostfixExpression>().fOperand, false);
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kExpression_Kind,
                                                         constantPropagate, e, nullptr });
            break;
        case Expression::kSwizzle_Kind:
            this->addExpression(cfg, &e->get()->as<Swizzle>().fBase, constantPropagate);
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kExpression_Kind,
                                                         constantPropagate, e, nullptr });
            break;
        case Expression::kBoolLiteral_Kind:   // fall through
        case Expression::kExternalValue_Kind: // fall through
        case Expression::kFloatLiteral_Kind:  // fall through
        case Expression::kIntLiteral_Kind:    // fall through
        case Expression::kNullLiteral_Kind:   // fall through
        case Expression::kSetting_Kind:       // fall through
        case Expression::kVariableReference_Kind:
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kExpression_Kind,
                                                         constantPropagate, e, nullptr });
            break;
        case Expression::kTernary_Kind: {
            TernaryExpression& t = e->get()->as<TernaryExpression>();
            this->addExpression(cfg, &t.fTest, constantPropagate);
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kExpression_Kind,
                                                         constantPropagate, e, nullptr });
            BlockId start = cfg.fCurrent;
            cfg.newBlock();
            this->addExpression(cfg, &t.fIfTrue, constantPropagate);
            BlockId next = cfg.newBlock();
            cfg.fCurrent = start;
            cfg.newBlock();
            this->addExpression(cfg, &t.fIfFalse, constantPropagate);
            cfg.addExit(cfg.fCurrent, next);
            cfg.fCurrent = next;
            break;
        }
        case Expression::kFunctionReference_Kind: // fall through
        case Expression::kTypeReference_Kind:     // fall through
        case Expression::kDefined_Kind:
            SkASSERT(false);
            break;
    }
}

// adds expressions that are evaluated as part of resolving an lvalue
void CFGGenerator::addLValue(CFG& cfg, std::unique_ptr<Expression>* e) {
    switch ((*e)->fKind) {
        case Expression::kFieldAccess_Kind:
            this->addLValue(cfg, &e->get()->as<FieldAccess>().fBase);
            break;
        case Expression::kIndex_Kind: {
            IndexExpression& indexExpr = e->get()->as<IndexExpression>();
            this->addLValue(cfg, &indexExpr.fBase);
            this->addExpression(cfg, &indexExpr.fIndex, /*constantPropagate=*/true);
            break;
        }
        case Expression::kSwizzle_Kind:
            this->addLValue(cfg, &e->get()->as<Swizzle>().fBase);
            break;
        case Expression::kExternalValue_Kind: // fall through
        case Expression::kVariableReference_Kind:
            break;
        case Expression::kTernary_Kind: {
            TernaryExpression& ternary = e->get()->as<TernaryExpression>();
            this->addExpression(cfg, &ternary.fTest, /*constantPropagate=*/true);
            // Technically we will of course only evaluate one or the other, but if the test turns
            // out to be constant, the ternary will get collapsed down to just one branch anyway. So
            // it should be ok to pretend that we always evaluate both branches here.
            this->addLValue(cfg, &ternary.fIfTrue);
            this->addLValue(cfg, &ternary.fIfFalse);
            break;
        }
        default:
            // not an lvalue, can't happen
            SkASSERT(false);
            break;
    }
}

static bool is_true(Expression& expr) {
    return expr.fKind == Expression::kBoolLiteral_Kind && expr.as<BoolLiteral>().fValue;
}

void CFGGenerator::addStatement(CFG& cfg, std::unique_ptr<Statement>* s) {
    switch ((*s)->fKind) {
        case Statement::kBlock_Kind:
            for (auto& child : (*s)->as<Block>().fStatements) {
                addStatement(cfg, &child);
            }
            break;
        case Statement::kIf_Kind: {
            IfStatement& ifs = (*s)->as<IfStatement>();
            this->addExpression(cfg, &ifs.fTest, /*constantPropagate=*/true);
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kStatement_Kind, false,
                                                         nullptr, s });
            BlockId start = cfg.fCurrent;
            cfg.newBlock();
            this->addStatement(cfg, &ifs.fIfTrue);
            BlockId next = cfg.newBlock();
            if (ifs.fIfFalse) {
                cfg.fCurrent = start;
                cfg.newBlock();
                this->addStatement(cfg, &ifs.fIfFalse);
                cfg.addExit(cfg.fCurrent, next);
                cfg.fCurrent = next;
            } else {
                cfg.addExit(start, next);
            }
            break;
        }
        case Statement::kExpression_Kind: {
            this->addExpression(cfg, &(*s)->as<ExpressionStatement>().fExpression,
                                /*constantPropagate=*/true);
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kStatement_Kind, false,
                                                         nullptr, s });
            break;
        }
        case Statement::kVarDeclarations_Kind: {
            VarDeclarationsStatement& decls = (*s)->as<VarDeclarationsStatement>();
            for (auto& stmt : decls.fDeclaration->fVars) {
                if (stmt->fKind == Statement::kNop_Kind) {
                    continue;
                }
                VarDeclaration& vd = stmt->as<VarDeclaration>();
                if (vd.fValue) {
                    this->addExpression(cfg, &vd.fValue, /*constantPropagate=*/true);
                }
                cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kStatement_Kind,
                                                             false, nullptr, &stmt });
            }
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kStatement_Kind, false,
                                                         nullptr, s });
            break;
        }
        case Statement::kDiscard_Kind:
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kStatement_Kind, false,
                                                         nullptr, s });
            cfg.fCurrent = cfg.newIsolatedBlock();
            break;
        case Statement::kReturn_Kind: {
            ReturnStatement& r = (*s)->as<ReturnStatement>();
            if (r.fExpression) {
                this->addExpression(cfg, &r.fExpression, /*constantPropagate=*/true);
            }
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kStatement_Kind, false,
                                                         nullptr, s });
            cfg.fCurrent = cfg.newIsolatedBlock();
            break;
        }
        case Statement::kBreak_Kind:
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kStatement_Kind, false,
                                                         nullptr, s });
            cfg.addExit(cfg.fCurrent, fLoopExits.top());
            cfg.fCurrent = cfg.newIsolatedBlock();
            break;
        case Statement::kContinue_Kind:
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kStatement_Kind, false,
                                                         nullptr, s });
            cfg.addExit(cfg.fCurrent, fLoopContinues.top());
            cfg.fCurrent = cfg.newIsolatedBlock();
            break;
        case Statement::kWhile_Kind: {
            WhileStatement& w = (*s)->as<WhileStatement>();
            BlockId loopStart = cfg.newBlock();
            fLoopContinues.push(loopStart);
            BlockId loopExit = cfg.newIsolatedBlock();
            fLoopExits.push(loopExit);
            this->addExpression(cfg, &w.fTest, /*constantPropagate=*/true);
            BlockId test = cfg.fCurrent;
            if (!is_true(*w.fTest)) {
                cfg.addExit(test, loopExit);
            }
            cfg.newBlock();
            this->addStatement(cfg, &w.fStatement);
            cfg.addExit(cfg.fCurrent, loopStart);
            fLoopContinues.pop();
            fLoopExits.pop();
            cfg.fCurrent = loopExit;
            break;
        }
        case Statement::kDo_Kind: {
            DoStatement& d = (*s)->as<DoStatement>();
            BlockId loopStart = cfg.newBlock();
            fLoopContinues.push(loopStart);
            BlockId loopExit = cfg.newIsolatedBlock();
            fLoopExits.push(loopExit);
            this->addStatement(cfg, &d.fStatement);
            this->addExpression(cfg, &d.fTest, /*constantPropagate=*/true);
            cfg.addExit(cfg.fCurrent, loopExit);
            cfg.addExit(cfg.fCurrent, loopStart);
            fLoopContinues.pop();
            fLoopExits.pop();
            cfg.fCurrent = loopExit;
            break;
        }
        case Statement::kFor_Kind: {
            ForStatement& f = (*s)->as<ForStatement>();
            if (f.fInitializer) {
                this->addStatement(cfg, &f.fInitializer);
            }
            BlockId loopStart = cfg.newBlock();
            BlockId next = cfg.newIsolatedBlock();
            fLoopContinues.push(next);
            BlockId loopExit = cfg.newIsolatedBlock();
            fLoopExits.push(loopExit);
            if (f.fTest) {
                this->addExpression(cfg, &f.fTest, /*constantPropagate=*/true);
                // this isn't quite right; we should have an exit from here to the loop exit, and
                // remove the exit from the loop body to the loop exit. Structuring it like this
                // forces the optimizer to believe that the loop body is always executed at least
                // once. While not strictly correct, this avoids incorrect "variable not assigned"
                // errors on variables which are assigned within the loop. The correct solution to
                // this is to analyze the loop to see whether or not at least one iteration is
                // guaranteed to happen, but for the time being we take the easy way out.
            }
            cfg.newBlock();
            this->addStatement(cfg, &f.fStatement);
            cfg.addExit(cfg.fCurrent, next);
            cfg.fCurrent = next;
            if (f.fNext) {
                this->addExpression(cfg, &f.fNext, /*constantPropagate=*/true);
            }
            cfg.addExit(cfg.fCurrent, loopStart);
            cfg.addExit(cfg.fCurrent, loopExit);
            fLoopContinues.pop();
            fLoopExits.pop();
            cfg.fCurrent = loopExit;
            break;
        }
        case Statement::kSwitch_Kind: {
            SwitchStatement& ss = (*s)->as<SwitchStatement>();
            this->addExpression(cfg, &ss.fValue, /*constantPropagate=*/true);
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kStatement_Kind, false,
                                                         nullptr, s });
            BlockId start = cfg.fCurrent;
            BlockId switchExit = cfg.newIsolatedBlock();
            fLoopExits.push(switchExit);
            for (const auto& c : ss.fCases) {
                cfg.newBlock();
                cfg.addExit(start, cfg.fCurrent);
                if (c->fValue) {
                    // technically this should go in the start block, but it doesn't actually matter
                    // because it must be constant. Not worth running two loops for.
                    this->addExpression(cfg, &c->fValue, /*constantPropagate=*/true);
                }
                for (auto& caseStatement : c->fStatements) {
                    this->addStatement(cfg, &caseStatement);
                }
            }
            cfg.addExit(cfg.fCurrent, switchExit);
            // note that unlike GLSL, our grammar requires the default case to be last
            if (0 == ss.fCases.size() || ss.fCases[ss.fCases.size() - 1]->fValue) {
                // switch does not have a default clause, mark that it can skip straight to the end
                cfg.addExit(start, switchExit);
            }
            fLoopExits.pop();
            cfg.fCurrent = switchExit;
            break;
        }
        case Statement::kNop_Kind:
            break;
        default:
#ifdef SK_DEBUG
            ABORT("unsupported statement: %s\n", (*s)->description().c_str());
#endif
            break;
    }
}

CFG CFGGenerator::getCFG(FunctionDefinition& f) {
    CFG result;
    result.fStart = result.newBlock();
    result.fCurrent = result.fStart;
    this->addStatement(result, &f.fBody);
    result.newBlock();
    result.fExit = result.fCurrent;
    return result;
}

}  // namespace SkSL
