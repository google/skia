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
void CFG::dump() const {
    for (size_t i = 0; i < fBlocks.size(); i++) {
        printf("Block %zu\n-------\n", i);
        fBlocks[i].dump();
    }
}

void BasicBlock::dump() const {
    printf("Before: [");
    const char* separator = "";
    for (auto iter = fBefore.begin(); iter != fBefore.end(); iter++) {
        printf("%s%s = %s", separator, iter->first->description().c_str(),
               iter->second ? (*iter->second)->description().c_str() : "<undefined>");
        separator = ", ";
    }
    printf("]\nEntrances: [");
    separator = "";
    for (BlockId b : fEntrances) {
        printf("%s%zu", separator, b);
        separator = ", ";
    }
    printf("]\n");
    for (size_t j = 0; j < fNodes.size(); j++) {
        const BasicBlock::Node& n = fNodes[j];
        printf("Node %zu (%p): %s\n", j, &n, n.description().c_str());
    }
    printf("Exits: [");
    separator = "";
    for (BlockId b : fExits) {
        printf("%s%zu", separator, b);
        separator = ", ";
    }
    printf("]\n\n");
}
#endif

bool BasicBlock::tryRemoveExpressionBefore(std::vector<BasicBlock::Node>::iterator* iter,
                                           Expression* e) {
    if (e->kind() == Expression::Kind::kTernary) {
        return false;
    }
    bool result;
    if ((*iter)->isExpression()) {
        SkASSERT((*iter)->expression()->get() != e);
        Expression* old = (*iter)->expression()->get();
        do {
            if ((*iter) == fNodes.begin()) {
                return false;
            }
            --(*iter);
        } while (!(*iter)->isExpression() || (*iter)->expression()->get() != e);

        result = this->tryRemoveExpression(iter);

        while (!(*iter)->isExpression() || (*iter)->expression()->get() != old) {
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
        } while (!(*iter)->isExpression() || (*iter)->expression()->get() != e);

        result = this->tryRemoveExpression(iter);

        while (!(*iter)->isStatement() || (*iter)->statement()->get() != old) {
            SkASSERT(*iter != fNodes.end());
            ++(*iter);
        }
    }
    return result;
}

bool BasicBlock::tryRemoveLValueBefore(std::vector<BasicBlock::Node>::iterator* iter,
                                       Expression* lvalue) {
    switch (lvalue->kind()) {
        case Expression::Kind::kExternalValue: // fall through
        case Expression::Kind::kVariableReference:
            return true;
        case Expression::Kind::kSwizzle:
            return this->tryRemoveLValueBefore(iter, lvalue->as<Swizzle>().fBase.get());
        case Expression::Kind::kFieldAccess:
            return this->tryRemoveLValueBefore(iter, lvalue->as<FieldAccess>().fBase.get());
        case Expression::Kind::kIndex: {
            IndexExpression& indexExpr = lvalue->as<IndexExpression>();
            if (!this->tryRemoveLValueBefore(iter, indexExpr.fBase.get())) {
                return false;
            }
            return this->tryRemoveExpressionBefore(iter, indexExpr.fIndex.get());
        }
        case Expression::Kind::kTernary: {
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
    switch (expr->kind()) {
        case Expression::Kind::kBinary: {
            BinaryExpression& b = expr->as<BinaryExpression>();
            if (b.getOperator() == Token::Kind::TK_EQ) {
                if (!this->tryRemoveLValueBefore(iter, &b.left())) {
                    return false;
                }
            } else if (!this->tryRemoveExpressionBefore(iter, &b.left())) {
                return false;
            }
            if (!this->tryRemoveExpressionBefore(iter, &b.right())) {
                return false;
            }
            SkASSERT((*iter)->expression()->get() == expr);
            *iter = fNodes.erase(*iter);
            return true;
        }
        case Expression::Kind::kTernary: {
            // ternaries cross basic block boundaries, must regenerate the CFG to remove it
            return false;
        }
        case Expression::Kind::kFieldAccess: {
            FieldAccess& f = expr->as<FieldAccess>();
            if (!this->tryRemoveExpressionBefore(iter, f.fBase.get())) {
                return false;
            }
            *iter = fNodes.erase(*iter);
            return true;
        }
        case Expression::Kind::kSwizzle: {
            Swizzle& s = expr->as<Swizzle>();
            if (s.fBase && !this->tryRemoveExpressionBefore(iter, s.fBase.get())) {
                return false;
            }
            *iter = fNodes.erase(*iter);
            return true;
        }
        case Expression::Kind::kIndex: {
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
        case Expression::Kind::kConstructor: {
            Constructor& c = expr->as<Constructor>();
            for (auto& arg : c.arguments()) {
                if (!this->tryRemoveExpressionBefore(iter, arg.get())) {
                    return false;
                }
                SkASSERT((*iter)->expression()->get() == expr);
            }
            *iter = fNodes.erase(*iter);
            return true;
        }
        case Expression::Kind::kFunctionCall: {
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
        case Expression::Kind::kPrefix:
            if (!this->tryRemoveExpressionBefore(iter,
                                                 expr->as<PrefixExpression>().fOperand.get())) {
                return false;
            }
            *iter = fNodes.erase(*iter);
            return true;
        case Expression::Kind::kPostfix:
            if (!this->tryRemoveExpressionBefore(iter,
                                                 expr->as<PrefixExpression>().fOperand.get())) {
                return false;
            }
            *iter = fNodes.erase(*iter);
            return true;
        case Expression::Kind::kBoolLiteral:  // fall through
        case Expression::Kind::kFloatLiteral: // fall through
        case Expression::Kind::kIntLiteral:   // fall through
        case Expression::Kind::kSetting:      // fall through
        case Expression::Kind::kVariableReference:
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
    switch ((*expr)->kind()) {
        case Expression::Kind::kBinary: {
            BinaryExpression& b = expr->get()->as<BinaryExpression>();
            if (!this->tryInsertExpression(iter, &b.rightPointer())) {
                return false;
            }

            ++(*iter);
            if (!this->tryInsertExpression(iter, &b.leftPointer())) {
                return false;
            }
            ++(*iter);
            *iter = fNodes.insert(*iter,
                                  BasicBlock::MakeExpression(expr, /*constantPropagation=*/true));
            return true;
        }
        case Expression::Kind::kBoolLiteral:  // fall through
        case Expression::Kind::kFloatLiteral: // fall through
        case Expression::Kind::kIntLiteral:   // fall through
        case Expression::Kind::kVariableReference: {
            *iter = fNodes.insert(*iter,
                                  BasicBlock::MakeExpression(expr, /*constantPropagation=*/true));
            return true;
        }
        case Expression::Kind::kConstructor: {
            Constructor& c = expr->get()->as<Constructor>();
            for (auto& arg : c.arguments()) {
                if (!this->tryInsertExpression(iter, &arg)) {
                    return false;
                }
                ++(*iter);
            }
            *iter = fNodes.insert(*iter,
                                  BasicBlock::MakeExpression(expr, /*constantPropagation=*/true));
            return true;
        }
        case Expression::Kind::kSwizzle: {
            Swizzle& s = expr->get()->as<Swizzle>();
            if (!this->tryInsertExpression(iter, &s.fBase)) {
                return false;
            }
            ++(*iter);
            *iter = fNodes.insert(*iter,
                                  BasicBlock::MakeExpression(expr, /*constantPropagation=*/true));
            return true;
        }
        default:
            return false;
    }
}

void CFGGenerator::addExpression(CFG& cfg, std::unique_ptr<Expression>* e, bool constantPropagate) {
    SkASSERT(e);
    switch ((*e)->kind()) {
        case Expression::Kind::kBinary: {
            BinaryExpression& b = e->get()->as<BinaryExpression>();
            Token::Kind op = b.getOperator();
            switch (op) {
                case Token::Kind::TK_LOGICALAND: // fall through
                case Token::Kind::TK_LOGICALOR: {
                    // this isn't as precise as it could be -- we don't bother to track that if we
                    // early exit from a logical and/or, we know which branch of an 'if' we're going
                    // to hit -- but it won't make much difference in practice.
                    this->addExpression(cfg, &b.leftPointer(), constantPropagate);
                    BlockId start = cfg.fCurrent;
                    cfg.newBlock();
                    this->addExpression(cfg, &b.rightPointer(), constantPropagate);
                    cfg.newBlock();
                    cfg.addExit(start, cfg.fCurrent);
                    cfg.currentBlock().fNodes.push_back(
                            BasicBlock::MakeExpression(e, constantPropagate));
                    break;
                }
                case Token::Kind::TK_EQ: {
                    this->addExpression(cfg, &b.rightPointer(), constantPropagate);
                    this->addLValue(cfg, &b.leftPointer());
                    cfg.currentBlock().fNodes.push_back(
                            BasicBlock::MakeExpression(e, constantPropagate));
                    break;
                }
                default:
                    this->addExpression(cfg, &b.leftPointer(),
                                        !Compiler::IsAssignment(b.getOperator()));
                    this->addExpression(cfg, &b.rightPointer(), constantPropagate);
                    cfg.currentBlock().fNodes.push_back(
                            BasicBlock::MakeExpression(e, constantPropagate));
            }
            break;
        }
        case Expression::Kind::kConstructor: {
            Constructor& c = e->get()->as<Constructor>();
            for (auto& arg : c.arguments()) {
                this->addExpression(cfg, &arg, constantPropagate);
            }
            cfg.currentBlock().fNodes.push_back(BasicBlock::MakeExpression(e, constantPropagate));
            break;
        }
        case Expression::Kind::kExternalFunctionCall: {
            ExternalFunctionCall& c = e->get()->as<ExternalFunctionCall>();
            for (auto& arg : c.arguments()) {
                this->addExpression(cfg, &arg, constantPropagate);
            }
            cfg.currentBlock().fNodes.push_back(BasicBlock::MakeExpression(e, constantPropagate));
            break;
        }
        case Expression::Kind::kFunctionCall: {
            FunctionCall& c = e->get()->as<FunctionCall>();
            for (auto& arg : c.fArguments) {
                this->addExpression(cfg, &arg, constantPropagate);
            }
            cfg.currentBlock().fNodes.push_back(BasicBlock::MakeExpression(e, constantPropagate));
            break;
        }
        case Expression::Kind::kFieldAccess: {
            this->addExpression(cfg, &e->get()->as<FieldAccess>().fBase, constantPropagate);
            cfg.currentBlock().fNodes.push_back(BasicBlock::MakeExpression(e, constantPropagate));
            break;
        }
        case Expression::Kind::kIndex: {
            IndexExpression& indexExpr = e->get()->as<IndexExpression>();

            this->addExpression(cfg, &indexExpr.fBase, constantPropagate);
            this->addExpression(cfg, &indexExpr.fIndex, constantPropagate);
            cfg.currentBlock().fNodes.push_back(BasicBlock::MakeExpression(e, constantPropagate));
            break;
        }
        case Expression::Kind::kPrefix: {
            PrefixExpression& p = e->get()->as<PrefixExpression>();
            this->addExpression(cfg, &p.fOperand, constantPropagate &&
                                                  p.fOperator != Token::Kind::TK_PLUSPLUS &&
                                                  p.fOperator != Token::Kind::TK_MINUSMINUS);
            cfg.currentBlock().fNodes.push_back(BasicBlock::MakeExpression(e, constantPropagate));
            break;
        }
        case Expression::Kind::kPostfix:
            this->addExpression(cfg, &e->get()->as<PostfixExpression>().fOperand, false);
            cfg.currentBlock().fNodes.push_back(BasicBlock::MakeExpression(e, constantPropagate));
            break;
        case Expression::Kind::kSwizzle:
            this->addExpression(cfg, &e->get()->as<Swizzle>().fBase, constantPropagate);
            cfg.currentBlock().fNodes.push_back(BasicBlock::MakeExpression(e, constantPropagate));
            break;
        case Expression::Kind::kBoolLiteral:   // fall through
        case Expression::Kind::kExternalValue: // fall through
        case Expression::Kind::kFloatLiteral:  // fall through
        case Expression::Kind::kIntLiteral:    // fall through
        case Expression::Kind::kNullLiteral:   // fall through
        case Expression::Kind::kSetting:       // fall through
        case Expression::Kind::kVariableReference:
            cfg.currentBlock().fNodes.push_back(BasicBlock::MakeExpression(e, constantPropagate));
            break;
        case Expression::Kind::kTernary: {
            TernaryExpression& t = e->get()->as<TernaryExpression>();
            this->addExpression(cfg, &t.fTest, constantPropagate);
            cfg.currentBlock().fNodes.push_back(BasicBlock::MakeExpression(e, constantPropagate));
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
        case Expression::Kind::kFunctionReference: // fall through
        case Expression::Kind::kTypeReference:     // fall through
        case Expression::Kind::kDefined:
            SkASSERT(false);
            break;
    }
}

// adds expressions that are evaluated as part of resolving an lvalue
void CFGGenerator::addLValue(CFG& cfg, std::unique_ptr<Expression>* e) {
    switch ((*e)->kind()) {
        case Expression::Kind::kFieldAccess:
            this->addLValue(cfg, &e->get()->as<FieldAccess>().fBase);
            break;
        case Expression::Kind::kIndex: {
            IndexExpression& indexExpr = e->get()->as<IndexExpression>();
            this->addLValue(cfg, &indexExpr.fBase);
            this->addExpression(cfg, &indexExpr.fIndex, /*constantPropagate=*/true);
            break;
        }
        case Expression::Kind::kSwizzle:
            this->addLValue(cfg, &e->get()->as<Swizzle>().fBase);
            break;
        case Expression::Kind::kExternalValue: // fall through
        case Expression::Kind::kVariableReference:
            break;
        case Expression::Kind::kTernary: {
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
    return expr.is<BoolLiteral>() && expr.as<BoolLiteral>().value();
}

void CFGGenerator::addStatement(CFG& cfg, std::unique_ptr<Statement>* s) {
    switch ((*s)->kind()) {
        case Statement::Kind::kBlock: {
            Block& block = (*s)->as<Block>();
            for (std::unique_ptr<Statement>& child : block.children()) {
                addStatement(cfg, &child);
            }
            break;
        }
        case Statement::Kind::kIf: {
            IfStatement& ifs = (*s)->as<IfStatement>();
            this->addExpression(cfg, &ifs.fTest, /*constantPropagate=*/true);
            cfg.currentBlock().fNodes.push_back(BasicBlock::MakeStatement(s));
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
        case Statement::Kind::kExpression: {
            this->addExpression(cfg, &(*s)->as<ExpressionStatement>().expression(),
                                /*constantPropagate=*/true);
            cfg.currentBlock().fNodes.push_back(BasicBlock::MakeStatement(s));
            break;
        }
        case Statement::Kind::kVarDeclarations: {
            VarDeclarationsStatement& decls = (*s)->as<VarDeclarationsStatement>();
            for (auto& stmt : decls.fDeclaration->fVars) {
                if (stmt->kind() == Statement::Kind::kNop) {
                    continue;
                }
                VarDeclaration& vd = stmt->as<VarDeclaration>();
                if (vd.fValue) {
                    this->addExpression(cfg, &vd.fValue, /*constantPropagate=*/true);
                }
                cfg.currentBlock().fNodes.push_back(BasicBlock::MakeStatement(&stmt));
            }
            cfg.currentBlock().fNodes.push_back(BasicBlock::MakeStatement(s));
            break;
        }
        case Statement::Kind::kDiscard:
            cfg.currentBlock().fNodes.push_back(BasicBlock::MakeStatement(s));
            cfg.fCurrent = cfg.newIsolatedBlock();
            break;
        case Statement::Kind::kReturn: {
            ReturnStatement& r = (*s)->as<ReturnStatement>();
            if (r.fExpression) {
                this->addExpression(cfg, &r.fExpression, /*constantPropagate=*/true);
            }
            cfg.currentBlock().fNodes.push_back(BasicBlock::MakeStatement(s));
            cfg.fCurrent = cfg.newIsolatedBlock();
            break;
        }
        case Statement::Kind::kBreak:
            cfg.currentBlock().fNodes.push_back(BasicBlock::MakeStatement(s));
            cfg.addExit(cfg.fCurrent, fLoopExits.top());
            cfg.fCurrent = cfg.newIsolatedBlock();
            break;
        case Statement::Kind::kContinue:
            cfg.currentBlock().fNodes.push_back(BasicBlock::MakeStatement(s));
            cfg.addExit(cfg.fCurrent, fLoopContinues.top());
            cfg.fCurrent = cfg.newIsolatedBlock();
            break;
        case Statement::Kind::kWhile: {
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
        case Statement::Kind::kDo: {
            DoStatement& d = (*s)->as<DoStatement>();
            BlockId loopStart = cfg.newBlock();
            fLoopContinues.push(loopStart);
            BlockId loopExit = cfg.newIsolatedBlock();
            fLoopExits.push(loopExit);
            this->addStatement(cfg, &d.statement());
            this->addExpression(cfg, &d.test(), /*constantPropagate=*/true);
            cfg.addExit(cfg.fCurrent, loopExit);
            cfg.addExit(cfg.fCurrent, loopStart);
            fLoopContinues.pop();
            fLoopExits.pop();
            cfg.fCurrent = loopExit;
            break;
        }
        case Statement::Kind::kFor: {
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
        case Statement::Kind::kSwitch: {
            SwitchStatement& ss = (*s)->as<SwitchStatement>();
            this->addExpression(cfg, &ss.fValue, /*constantPropagate=*/true);
            cfg.currentBlock().fNodes.push_back(BasicBlock::MakeStatement(s));
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
            if (ss.fCases.empty() || ss.fCases.back()->fValue) {
                // switch does not have a default clause, mark that it can skip straight to the end
                cfg.addExit(start, switchExit);
            }
            fLoopExits.pop();
            cfg.fCurrent = switchExit;
            break;
        }
        case Statement::Kind::kInlineMarker:
        case Statement::Kind::kNop:
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
