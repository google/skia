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
                   iter->second ? iter->second->description().c_str() : "<undefined>");
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
            printf("Node %d: %s\n", (int) j, fBlocks[i].fNodes[j].fNode->description().c_str());
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

void CFGGenerator::addExpression(CFG& cfg, const Expression* e) {
    switch (e->fKind) {
        case Expression::kBinary_Kind: {
            const BinaryExpression* b = (const BinaryExpression*) e;
            switch (b->fOperator) {
                case Token::LOGICALAND: // fall through
                case Token::LOGICALOR: {
                    // this isn't as precise as it could be -- we don't bother to track that if we
                    // early exit from a logical and/or, we know which branch of an 'if' we're going
                    // to hit -- but it won't make much difference in practice.
                    this->addExpression(cfg, b->fLeft.get());
                    BlockId start = cfg.fCurrent;
                    cfg.newBlock();
                    this->addExpression(cfg, b->fRight.get());
                    cfg.newBlock();
                    cfg.addExit(start, cfg.fCurrent);
                    break;
                }
                case Token::EQ: {
                    this->addExpression(cfg, b->fRight.get());
                    this->addLValue(cfg, b->fLeft.get());
                    cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ 
                        BasicBlock::Node::kExpression_Kind, 
                        b 
                    });
                    break;
                }
                default:
                    this->addExpression(cfg, b->fLeft.get());
                    this->addExpression(cfg, b->fRight.get());
                    cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ 
                        BasicBlock::Node::kExpression_Kind, 
                        b 
                    });
            }
            break;
        }
        case Expression::kConstructor_Kind: {
            const Constructor* c = (const Constructor*) e;
            for (const auto& arg : c->fArguments) {
                this->addExpression(cfg, arg.get());
            }
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kExpression_Kind, c });
            break;
        }
        case Expression::kFunctionCall_Kind: {
            const FunctionCall* c = (const FunctionCall*) e;
            for (const auto& arg : c->fArguments) {
                this->addExpression(cfg, arg.get());
            }
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kExpression_Kind, c });
            break;
        }
        case Expression::kFieldAccess_Kind:
            this->addExpression(cfg, ((const FieldAccess*) e)->fBase.get());
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kExpression_Kind, e });
            break;
        case Expression::kIndex_Kind:
            this->addExpression(cfg, ((const IndexExpression*) e)->fBase.get());
            this->addExpression(cfg, ((const IndexExpression*) e)->fIndex.get());
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kExpression_Kind, e });
            break;
        case Expression::kPrefix_Kind:
            this->addExpression(cfg, ((const PrefixExpression*) e)->fOperand.get());
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kExpression_Kind, e });
            break;
        case Expression::kPostfix_Kind:
            this->addExpression(cfg, ((const PostfixExpression*) e)->fOperand.get());
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kExpression_Kind, e });
            break;
        case Expression::kSwizzle_Kind:
            this->addExpression(cfg, ((const Swizzle*) e)->fBase.get());
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kExpression_Kind, e });
            break;
        case Expression::kBoolLiteral_Kind:  // fall through
        case Expression::kFloatLiteral_Kind: // fall through
        case Expression::kIntLiteral_Kind:   // fall through
        case Expression::kVariableReference_Kind:
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kExpression_Kind, e });
            break;
        case Expression::kTernary_Kind: {
            const TernaryExpression* t = (const TernaryExpression*) e;
            this->addExpression(cfg, t->fTest.get());
            BlockId start = cfg.fCurrent;
            cfg.newBlock();
            this->addExpression(cfg, t->fIfTrue.get());
            BlockId next = cfg.newBlock();
            cfg.fCurrent = start;
            cfg.newBlock();
            this->addExpression(cfg, t->fIfFalse.get());
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
void CFGGenerator::addLValue(CFG& cfg, const Expression* e) {
    switch (e->fKind) {
        case Expression::kFieldAccess_Kind:
            this->addLValue(cfg, ((const FieldAccess*) e)->fBase.get());
            break;
        case Expression::kIndex_Kind:
            this->addLValue(cfg, ((const IndexExpression*) e)->fBase.get());
            this->addExpression(cfg, ((const IndexExpression*) e)->fIndex.get());
            break;
        case Expression::kSwizzle_Kind:
            this->addLValue(cfg, ((const Swizzle*) e)->fBase.get());
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
            const IfStatement* ifs = (const IfStatement*) s;
            this->addExpression(cfg, ifs->fTest.get());
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
            this->addExpression(cfg, ((ExpressionStatement&) *s).fExpression.get());
            break;
        }
        case Statement::kVarDeclarations_Kind: {
            const VarDeclarationsStatement& decls = ((VarDeclarationsStatement&) *s);
            for (const auto& vd : decls.fDeclaration->fVars) {
                if (vd.fValue) {
                    this->addExpression(cfg, vd.fValue.get());
                }
            }
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kStatement_Kind, s });
            break;
        }
        case Statement::kDiscard_Kind:
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kStatement_Kind, s });
            cfg.fCurrent = cfg.newIsolatedBlock();
            break;
        case Statement::kReturn_Kind: {
            const ReturnStatement& r = ((ReturnStatement&) *s);
            if (r.fExpression) {
                this->addExpression(cfg, r.fExpression.get());
            }
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kStatement_Kind, s });
            cfg.fCurrent = cfg.newIsolatedBlock();
            break;
        }
        case Statement::kBreak_Kind:
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kStatement_Kind, s });
            cfg.addExit(cfg.fCurrent, fLoopExits.top());
            cfg.fCurrent = cfg.newIsolatedBlock();
            break;
        case Statement::kContinue_Kind:
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kStatement_Kind, s });
            cfg.addExit(cfg.fCurrent, fLoopContinues.top());
            cfg.fCurrent = cfg.newIsolatedBlock();
            break;
        case Statement::kWhile_Kind: {
            const WhileStatement* w = (const WhileStatement*) s;
            BlockId loopStart = cfg.newBlock();
            fLoopContinues.push(loopStart);
            BlockId loopExit = cfg.newIsolatedBlock();
            fLoopExits.push(loopExit);
            this->addExpression(cfg, w->fTest.get());
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
            const DoStatement* d = (const DoStatement*) s;
            BlockId loopStart = cfg.newBlock();
            fLoopContinues.push(loopStart);
            BlockId loopExit = cfg.newIsolatedBlock();
            fLoopExits.push(loopExit);
            this->addStatement(cfg, d->fStatement.get());
            this->addExpression(cfg, d->fTest.get());
            cfg.addExit(cfg.fCurrent, loopExit);
            cfg.addExit(cfg.fCurrent, loopStart);
            fLoopContinues.pop();
            fLoopExits.pop();
            cfg.fCurrent = loopExit;
            break;
        }
        case Statement::kFor_Kind: {
            const ForStatement* f = (const ForStatement*) s;
            if (f->fInitializer) {
                this->addStatement(cfg, f->fInitializer.get());
            }
            BlockId loopStart = cfg.newBlock();
            BlockId next = cfg.newIsolatedBlock();
            fLoopContinues.push(next);
            BlockId loopExit = cfg.newIsolatedBlock();
            fLoopExits.push(loopExit);
            if (f->fTest) {
                this->addExpression(cfg, f->fTest.get());
                BlockId test = cfg.fCurrent;
                cfg.addExit(test, loopExit);
            }
            cfg.newBlock();
            this->addStatement(cfg, f->fStatement.get());
            cfg.addExit(cfg.fCurrent, next);
            cfg.fCurrent = next;
            if (f->fNext) {
                this->addExpression(cfg, f->fNext.get());
            }
            cfg.addExit(next, loopStart);
            fLoopContinues.pop();
            fLoopExits.pop();
            cfg.fCurrent = loopExit;
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
