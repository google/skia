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

namespace SkSL {

void BasicBlock::Node::setExpression(std::unique_ptr<Expression> expr, ProgramUsage* usage) {
    SkASSERT(!this->isStatement());
    usage->remove(fExpression->get());
    *fExpression = std::move(expr);
}

void BasicBlock::Node::setStatement(std::unique_ptr<Statement> stmt, ProgramUsage* usage) {
    SkASSERT(!this->isExpression());
    // See comment in header - we assume that stmt was already counted in usage (it was a subset
    // of fStatement). There is no way to verify that, unfortunately.
    usage->remove(fStatement->get());
    *fStatement = std::move(stmt);
}

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
    BasicBlock::ExitArray& exits = fBlocks[from].fExits;
    if (std::find(exits.begin(), exits.end(), to) == exits.end()) {
        exits.push_back(to);
    }
    if (from == 0 || fBlocks[from].fIsReachable) {
        fBlocks[to].fIsReachable = true;
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
    fBefore.foreach([&](const Variable* var, std::unique_ptr<Expression>* expr) {
        printf("%s%s = %s", separator,
                            var->description().c_str(),
                            expr ? (*expr)->description().c_str() : "<undefined>");
        separator = ", ";
    });
    printf("]\nIs Reachable: [%s]\n", fIsReachable ? "yes" : "no");
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
    if (e->is<TernaryExpression>()) {
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
            return this->tryRemoveLValueBefore(iter, lvalue->as<Swizzle>().base().get());
        case Expression::Kind::kFieldAccess:
            return this->tryRemoveLValueBefore(iter, lvalue->as<FieldAccess>().base().get());
        case Expression::Kind::kIndex: {
            IndexExpression& indexExpr = lvalue->as<IndexExpression>();
            if (!this->tryRemoveLValueBefore(iter, indexExpr.base().get())) {
                return false;
            }
            return this->tryRemoveExpressionBefore(iter, indexExpr.index().get());
        }
        case Expression::Kind::kTernary: {
            TernaryExpression& ternary = lvalue->as<TernaryExpression>();
            if (!this->tryRemoveExpressionBefore(iter, ternary.test().get())) {
                return false;
            }
            if (!this->tryRemoveLValueBefore(iter, ternary.ifTrue().get())) {
                return false;
            }
            return this->tryRemoveLValueBefore(iter, ternary.ifFalse().get());
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
                if (!this->tryRemoveLValueBefore(iter, b.left().get())) {
                    return false;
                }
            } else if (!this->tryRemoveExpressionBefore(iter, b.left().get())) {
                return false;
            }
            if (!this->tryRemoveExpressionBefore(iter, b.right().get())) {
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
            if (!this->tryRemoveExpressionBefore(iter, f.base().get())) {
                return false;
            }
            *iter = fNodes.erase(*iter);
            return true;
        }
        case Expression::Kind::kSwizzle: {
            Swizzle& s = expr->as<Swizzle>();
            if (s.base() && !this->tryRemoveExpressionBefore(iter, s.base().get())) {
                return false;
            }
            *iter = fNodes.erase(*iter);
            return true;
        }
        case Expression::Kind::kIndex: {
            IndexExpression& idx = expr->as<IndexExpression>();
            if (!this->tryRemoveExpressionBefore(iter, idx.base().get())) {
                return false;
            }
            if (!this->tryRemoveExpressionBefore(iter, idx.index().get())) {
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
            for (auto& arg : f.arguments()) {
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
                                                 expr->as<PrefixExpression>().operand().get())) {
                return false;
            }
            *iter = fNodes.erase(*iter);
            return true;
        case Expression::Kind::kPostfix:
            if (!this->tryRemoveExpressionBefore(iter,
                                                 expr->as<PostfixExpression>().operand().get())) {
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
            if (!this->tryInsertExpression(iter, &b.right())) {
                return false;
            }

            ++(*iter);
            if (!this->tryInsertExpression(iter, &b.left())) {
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
            if (!this->tryInsertExpression(iter, &s.base())) {
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
                    this->addExpression(cfg, &b.left(), constantPropagate);
                    BlockId start = cfg.fCurrent;
                    cfg.newBlock();
                    this->addExpression(cfg, &b.right(), constantPropagate);
                    cfg.newBlock();
                    cfg.addExit(start, cfg.fCurrent);
                    cfg.currentBlock().fNodes.push_back(
                            BasicBlock::MakeExpression(e, constantPropagate));
                    break;
                }
                case Token::Kind::TK_EQ: {
                    this->addExpression(cfg, &b.right(), constantPropagate);
                    this->addLValue(cfg, &b.left());
                    cfg.currentBlock().fNodes.push_back(
                            BasicBlock::MakeExpression(e, constantPropagate));
                    break;
                }
                default:
                    this->addExpression(cfg, &b.left(),
                                        !Compiler::IsAssignment(b.getOperator()));
                    this->addExpression(cfg, &b.right(), constantPropagate);
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
            for (auto& arg : c.arguments()) {
                this->addExpression(cfg, &arg, constantPropagate);
            }
            cfg.currentBlock().fNodes.push_back(BasicBlock::MakeExpression(e, constantPropagate));
            break;
        }
        case Expression::Kind::kFieldAccess: {
            this->addExpression(cfg, &e->get()->as<FieldAccess>().base(), constantPropagate);
            cfg.currentBlock().fNodes.push_back(BasicBlock::MakeExpression(e, constantPropagate));
            break;
        }
        case Expression::Kind::kIndex: {
            IndexExpression& indexExpr = e->get()->as<IndexExpression>();

            this->addExpression(cfg, &indexExpr.base(), constantPropagate);
            this->addExpression(cfg, &indexExpr.index(), constantPropagate);
            cfg.currentBlock().fNodes.push_back(BasicBlock::MakeExpression(e, constantPropagate));
            break;
        }
        case Expression::Kind::kPrefix: {
            PrefixExpression& p = e->get()->as<PrefixExpression>();
            this->addExpression(cfg, &p.operand(), constantPropagate &&
                                                  p.getOperator() != Token::Kind::TK_PLUSPLUS &&
                                                  p.getOperator() != Token::Kind::TK_MINUSMINUS);
            cfg.currentBlock().fNodes.push_back(BasicBlock::MakeExpression(e, constantPropagate));
            break;
        }
        case Expression::Kind::kPostfix:
            this->addExpression(cfg, &e->get()->as<PostfixExpression>().operand(), false);
            cfg.currentBlock().fNodes.push_back(BasicBlock::MakeExpression(e, constantPropagate));
            break;
        case Expression::Kind::kSwizzle:
            this->addExpression(cfg, &e->get()->as<Swizzle>().base(), constantPropagate);
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
            this->addExpression(cfg, &t.test(), constantPropagate);
            cfg.currentBlock().fNodes.push_back(BasicBlock::MakeExpression(e, constantPropagate));
            BlockId start = cfg.fCurrent;
            cfg.newBlock();
            this->addExpression(cfg, &t.ifTrue(), constantPropagate);
            BlockId next = cfg.newBlock();
            cfg.fCurrent = start;
            cfg.newBlock();
            this->addExpression(cfg, &t.ifFalse(), constantPropagate);
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
            this->addLValue(cfg, &e->get()->as<FieldAccess>().base());
            break;
        case Expression::Kind::kIndex: {
            IndexExpression& indexExpr = e->get()->as<IndexExpression>();
            this->addLValue(cfg, &indexExpr.base());
            this->addExpression(cfg, &indexExpr.index(), /*constantPropagate=*/true);
            break;
        }
        case Expression::Kind::kSwizzle:
            this->addLValue(cfg, &e->get()->as<Swizzle>().base());
            break;
        case Expression::Kind::kExternalValue: // fall through
        case Expression::Kind::kVariableReference:
            break;
        case Expression::Kind::kTernary: {
            TernaryExpression& ternary = e->get()->as<TernaryExpression>();
            this->addExpression(cfg, &ternary.test(), /*constantPropagate=*/true);
            // Technically we will of course only evaluate one or the other, but if the test turns
            // out to be constant, the ternary will get collapsed down to just one branch anyway. So
            // it should be ok to pretend that we always evaluate both branches here.
            this->addLValue(cfg, &ternary.ifTrue());
            this->addLValue(cfg, &ternary.ifFalse());
            break;
        }
        default:
            // not an lvalue, can't happen
            SkASSERT(false);
            break;
    }
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
            this->addExpression(cfg, &ifs.test(), /*constantPropagate=*/true);
            cfg.currentBlock().fNodes.push_back(BasicBlock::MakeStatement(s));
            BlockId start = cfg.fCurrent;
            cfg.newBlock();
            this->addStatement(cfg, &ifs.ifTrue());
            BlockId next = cfg.newBlock();
            if (ifs.ifFalse()) {
                cfg.fCurrent = start;
                cfg.newBlock();
                this->addStatement(cfg, &ifs.ifFalse());
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
        case Statement::Kind::kVarDeclaration: {
            VarDeclaration& vd = (*s)->as<VarDeclaration>();
            if (vd.value()) {
                this->addExpression(cfg, &vd.value(), /*constantPropagate=*/true);
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
            if (r.expression()) {
                this->addExpression(cfg, &r.expression(), /*constantPropagate=*/true);
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
            if (f.initializer()) {
                this->addStatement(cfg, &f.initializer());
            }
            BlockId loopStart = cfg.newBlock();
            BlockId next = cfg.newIsolatedBlock();
            fLoopContinues.push(next);
            BlockId loopExit = cfg.newIsolatedBlock();
            fLoopExits.push(loopExit);
            if (f.test()) {
                this->addExpression(cfg, &f.test(), /*constantPropagate=*/true);
                // this isn't quite right; we should have an exit from here to the loop exit, and
                // remove the exit from the loop body to the loop exit. Structuring it like this
                // forces the optimizer to believe that the loop body is always executed at least
                // once. While not strictly correct, this avoids incorrect "variable not assigned"
                // errors on variables which are assigned within the loop. The correct solution to
                // this is to analyze the loop to see whether or not at least one iteration is
                // guaranteed to happen, but for the time being we take the easy way out.
            }
            cfg.newBlock();
            this->addStatement(cfg, &f.statement());
            cfg.addExit(cfg.fCurrent, next);
            cfg.fCurrent = next;
            if (f.next()) {
                this->addExpression(cfg, &f.next(), /*constantPropagate=*/true);
            }
            // The increment expression of a for loop is allowed to be unreachable, because GLSL
            // ES2 requires us to provide an increment expression for our for-loops whether or not
            // it can be reached. Reporting it as "unreachable" isn't helpful if the alternative
            // is an invalid program.
            cfg.currentBlock().fAllowUnreachable = true;
            cfg.addExit(cfg.fCurrent, loopStart);
            cfg.addExit(cfg.fCurrent, loopExit);
            fLoopContinues.pop();
            fLoopExits.pop();
            cfg.fCurrent = loopExit;
            break;
        }
        case Statement::Kind::kSwitch: {
            SwitchStatement& ss = (*s)->as<SwitchStatement>();
            this->addExpression(cfg, &ss.value(), /*constantPropagate=*/true);
            cfg.currentBlock().fNodes.push_back(BasicBlock::MakeStatement(s));
            BlockId start = cfg.fCurrent;
            BlockId switchExit = cfg.newIsolatedBlock();
            fLoopExits.push(switchExit);
            for (auto& c : ss.cases()) {
                cfg.newBlock();
                cfg.addExit(start, cfg.fCurrent);
                if (c->value()) {
                    // technically this should go in the start block, but it doesn't actually matter
                    // because it must be constant. Not worth running two loops for.
                    this->addExpression(cfg, &c->value(), /*constantPropagate=*/true);
                }
                for (auto& caseStatement : c->statements()) {
                    this->addStatement(cfg, &caseStatement);
                }
            }
            cfg.addExit(cfg.fCurrent, switchExit);
            // note that unlike GLSL, our grammar requires the default case to be last
            if (ss.cases().empty() || ss.cases().back()->value()) {
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
    // The starting block is "reached" implicitly, even if nothing points to it.
    result.currentBlock().fAllowUnreachable = true;
    this->addStatement(result, &f.body());
    result.newBlock();
    result.fExit = result.fCurrent;
    return result;
}

}  // namespace SkSL
