/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSLCFGGenerator.h"

#include "ir/SkSLConstructor.h"
#include "ir/SkSLBinaryExpression.h"
#include "ir/SkSLDoStatement.h"
#include "ir/SkSLExpressionStatement.h"
#include "ir/SkSLFieldAccess.h"
#include "ir/SkSLForStatement.h"
#include "ir/SkSLFunctionCall.h"
#include "ir/SkSLIfStatement.h"
#include "ir/SkSLIndexExpression.h"
#include "ir/SkSLPostfixExpression.h"
#include "ir/SkSLPrefixExpression.h"
#include "ir/SkSLReturnStatement.h"
#include "ir/SkSLSwizzle.h"
#include "ir/SkSLSwitchStatement.h"
#include "ir/SkSLTernaryExpression.h"
#include "ir/SkSLVarDeclarationsStatement.h"
#include "ir/SkSLWhileStatement.h"

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

void CFG::dump() {
    for (size_t i = 0; i < fBlocks.size(); i++) {
        printf("Block %d\n-------\nBefore: ", (int) i);
        const char* separator = "";
        for (auto iter = fBlocks[i].fBefore.begin(); iter != fBlocks[i].fBefore.end(); iter++) {
            printf("%s%s = %s", separator, iter->first->description().c_str(),
                   *iter->second ? (*iter->second)->description().c_str() : "<undefined>");
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
            printf("Node %d: %s\n", (int) j, n.fKind == BasicBlock::Node::kExpression_Kind
                                                           ? (*n.fExpression)->description().c_str()
                                                           : n.fStatement->description().c_str());
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

void CFGGenerator::addExpression(CFG& cfg, std::unique_ptr<Expression>* e, bool constantPropagate) {
    ASSERT(e);
    switch ((*e)->fKind) {
        case Expression::kBinary_Kind: {
            BinaryExpression* b = (BinaryExpression*) e->get();
            switch (b->fOperator) {
                case Token::LOGICALAND: // fall through
                case Token::LOGICALOR: {
                    // this isn't as precise as it could be -- we don't bother to track that if we
                    // early exit from a logical and/or, we know which branch of an 'if' we're going
                    // to hit -- but it won't make much difference in practice.
                    this->addExpression(cfg, &b->fLeft, constantPropagate);
                    BlockId start = cfg.fCurrent;
                    cfg.newBlock();
                    this->addExpression(cfg, &b->fRight, constantPropagate);
                    cfg.newBlock();
                    cfg.addExit(start, cfg.fCurrent);
                    break;
                }
                case Token::EQ: {
                    this->addExpression(cfg, &b->fRight, constantPropagate);
                    this->addLValue(cfg, &b->fLeft);
                    cfg.fBlocks[cfg.fCurrent].fNodes.push_back({
                        BasicBlock::Node::kExpression_Kind,
                        constantPropagate,
                        e,
                        nullptr
                    });
                    break;
                }
                default:
                    this->addExpression(cfg, &b->fLeft, !Token::IsAssignment(b->fOperator));
                    this->addExpression(cfg, &b->fRight, constantPropagate);
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
            Constructor* c = (Constructor*) e->get();
            for (auto& arg : c->fArguments) {
                this->addExpression(cfg, &arg, constantPropagate);
            }
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kExpression_Kind,
                                                         constantPropagate, e, nullptr });
            break;
        }
        case Expression::kFunctionCall_Kind: {
            FunctionCall* c = (FunctionCall*) e->get();
            for (auto& arg : c->fArguments) {
                this->addExpression(cfg, &arg, constantPropagate);
            }
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kExpression_Kind,
                                                         constantPropagate, e, nullptr });
            break;
        }
        case Expression::kFieldAccess_Kind:
            this->addExpression(cfg, &((FieldAccess*) e->get())->fBase, constantPropagate);
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kExpression_Kind,
                                                         constantPropagate, e, nullptr });
            break;
        case Expression::kIndex_Kind:
            this->addExpression(cfg, &((IndexExpression*) e->get())->fBase, constantPropagate);
            this->addExpression(cfg, &((IndexExpression*) e->get())->fIndex, constantPropagate);
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kExpression_Kind,
                                                         constantPropagate, e, nullptr });
            break;
        case Expression::kPrefix_Kind: {
            PrefixExpression* p = (PrefixExpression*) e->get();
            this->addExpression(cfg, &p->fOperand, constantPropagate &&
                                                   p->fOperator != Token::PLUSPLUS &&
                                                   p->fOperator != Token::MINUSMINUS);
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kExpression_Kind,
                                                         constantPropagate, e, nullptr });
            break;
        }
        case Expression::kPostfix_Kind:
            this->addExpression(cfg, &((PostfixExpression*) e->get())->fOperand, false);
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kExpression_Kind,
                                                         constantPropagate, e, nullptr });
            break;
        case Expression::kSwizzle_Kind:
            this->addExpression(cfg, &((Swizzle*) e->get())->fBase, constantPropagate);
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kExpression_Kind,
                                                         constantPropagate, e, nullptr });
            break;
        case Expression::kBoolLiteral_Kind:  // fall through
        case Expression::kFloatLiteral_Kind: // fall through
        case Expression::kIntLiteral_Kind:   // fall through
        case Expression::kVariableReference_Kind:
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kExpression_Kind,
                                                         constantPropagate, e, nullptr });
            break;
        case Expression::kTernary_Kind: {
            TernaryExpression* t = (TernaryExpression*) e->get();
            this->addExpression(cfg, &t->fTest, constantPropagate);
            BlockId start = cfg.fCurrent;
            cfg.newBlock();
            this->addExpression(cfg, &t->fIfTrue, constantPropagate);
            BlockId next = cfg.newBlock();
            cfg.fCurrent = start;
            cfg.newBlock();
            this->addExpression(cfg, &t->fIfFalse, constantPropagate);
            cfg.addExit(cfg.fCurrent, next);
            cfg.fCurrent = next;
            break;
        }
        case Expression::kFunctionReference_Kind: // fall through
        case Expression::kTypeReference_Kind:     // fall through
        case Expression::kDefined_Kind:
            ASSERT(false);
            break;
    }
}

// adds expressions that are evaluated as part of resolving an lvalue
void CFGGenerator::addLValue(CFG& cfg, std::unique_ptr<Expression>* e) {
    switch ((*e)->fKind) {
        case Expression::kFieldAccess_Kind:
            this->addLValue(cfg, &((FieldAccess&) **e).fBase);
            break;
        case Expression::kIndex_Kind:
            this->addLValue(cfg, &((IndexExpression&) **e).fBase);
            this->addExpression(cfg, &((IndexExpression&) **e).fIndex, true);
            break;
        case Expression::kSwizzle_Kind:
            this->addLValue(cfg, &((Swizzle&) **e).fBase);
            break;
        case Expression::kVariableReference_Kind:
            break;
        default:
            // not an lvalue, can't happen
            ASSERT(false);
            break;
    }
}

void CFGGenerator::addStatement(CFG& cfg, const Statement* s) {
    switch (s->fKind) {
        case Statement::kBlock_Kind:
            for (const auto& child : ((const Block*) s)->fStatements) {
                addStatement(cfg, child.get());
            }
            break;
        case Statement::kIf_Kind: {
            IfStatement* ifs = (IfStatement*) s;
            this->addExpression(cfg, &ifs->fTest, true);
            BlockId start = cfg.fCurrent;
            cfg.newBlock();
            this->addStatement(cfg, ifs->fIfTrue.get());
            BlockId next = cfg.newBlock();
            if (ifs->fIfFalse) {
                cfg.fCurrent = start;
                cfg.newBlock();
                this->addStatement(cfg, ifs->fIfFalse.get());
                cfg.addExit(cfg.fCurrent, next);
                cfg.fCurrent = next;
            } else {
                cfg.addExit(start, next);
            }
            break;
        }
        case Statement::kExpression_Kind: {
            this->addExpression(cfg, &((ExpressionStatement&) *s).fExpression, true);
            break;
        }
        case Statement::kVarDeclarations_Kind: {
            VarDeclarationsStatement& decls = ((VarDeclarationsStatement&) *s);
            for (auto& vd : decls.fDeclaration->fVars) {
                if (vd.fValue) {
                    this->addExpression(cfg, &vd.fValue, true);
                }
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
            ReturnStatement& r = ((ReturnStatement&) *s);
            if (r.fExpression) {
                this->addExpression(cfg, &r.fExpression, true);
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
            WhileStatement* w = (WhileStatement*) s;
            BlockId loopStart = cfg.newBlock();
            fLoopContinues.push(loopStart);
            BlockId loopExit = cfg.newIsolatedBlock();
            fLoopExits.push(loopExit);
            this->addExpression(cfg, &w->fTest, true);
            BlockId test = cfg.fCurrent;
            cfg.addExit(test, loopExit);
            cfg.newBlock();
            this->addStatement(cfg, w->fStatement.get());
            cfg.addExit(cfg.fCurrent, loopStart);
            fLoopContinues.pop();
            fLoopExits.pop();
            cfg.fCurrent = loopExit;
            break;
        }
        case Statement::kDo_Kind: {
            DoStatement* d = (DoStatement*) s;
            BlockId loopStart = cfg.newBlock();
            fLoopContinues.push(loopStart);
            BlockId loopExit = cfg.newIsolatedBlock();
            fLoopExits.push(loopExit);
            this->addStatement(cfg, d->fStatement.get());
            this->addExpression(cfg, &d->fTest, true);
            cfg.addExit(cfg.fCurrent, loopExit);
            cfg.addExit(cfg.fCurrent, loopStart);
            fLoopContinues.pop();
            fLoopExits.pop();
            cfg.fCurrent = loopExit;
            break;
        }
        case Statement::kFor_Kind: {
            ForStatement* f = (ForStatement*) s;
            if (f->fInitializer) {
                this->addStatement(cfg, f->fInitializer.get());
            }
            BlockId loopStart = cfg.newBlock();
            BlockId next = cfg.newIsolatedBlock();
            fLoopContinues.push(next);
            BlockId loopExit = cfg.newIsolatedBlock();
            fLoopExits.push(loopExit);
            if (f->fTest) {
                this->addExpression(cfg, &f->fTest, true);
                BlockId test = cfg.fCurrent;
                cfg.addExit(test, loopExit);
            }
            cfg.newBlock();
            this->addStatement(cfg, f->fStatement.get());
            cfg.addExit(cfg.fCurrent, next);
            cfg.fCurrent = next;
            if (f->fNext) {
                this->addExpression(cfg, &f->fNext, true);
            }
            cfg.addExit(cfg.fCurrent, loopStart);
            fLoopContinues.pop();
            fLoopExits.pop();
            cfg.fCurrent = loopExit;
            break;
        }
        case Statement::kSwitch_Kind: {
            SwitchStatement* ss = (SwitchStatement*) s;
            this->addExpression(cfg, &ss->fValue, true);
            BlockId start = cfg.fCurrent;
            BlockId switchExit = cfg.newIsolatedBlock();
            fLoopExits.push(switchExit);
            for (const auto& c : ss->fCases) {
                cfg.newBlock();
                cfg.addExit(start, cfg.fCurrent);
                if (c->fValue) {
                    // technically this should go in the start block, but it doesn't actually matter
                    // because it must be constant. Not worth running two loops for.
                    this->addExpression(cfg, &c->fValue, true);
                }
                for (const auto& caseStatement : c->fStatements) {
                    this->addStatement(cfg, caseStatement.get());
                }
            }
            cfg.addExit(cfg.fCurrent, switchExit);
            // note that unlike GLSL, our grammar requires the default case to be last
            if (0 == ss->fCases.size() || ss->fCases[ss->fCases.size() - 1]->fValue) {
                // switch does not have a default clause, mark that it can skip straight to the end
                cfg.addExit(start, switchExit);
            }
            fLoopExits.pop();
            cfg.fCurrent = switchExit;
            break;
        }
        default:
            printf("statement: %s\n", s->description().c_str());
            ABORT("unsupported statement kind");
    }
}

CFG CFGGenerator::getCFG(const FunctionDefinition& f) {
    CFG result;
    result.fStart = result.newBlock();
    result.fCurrent = result.fStart;
    this->addStatement(result, f.fBody.get());
    result.newBlock();
    result.fExit = result.fCurrent;
    return result;
}

} // namespace
