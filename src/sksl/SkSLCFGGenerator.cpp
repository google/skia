/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLCFGGenerator.h"

#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLBoolLiteral.h"
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
#include "src/sksl/ir/SkSLVariableReference.h"
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

void CFG::dump() {
    for (size_t i = 0; i < fBlocks.size(); i++) {
        printf("Block %d\n-------\nBefore: ", (int) i);
        const char* separator = "";
        for (auto iter = fBlocks[i].fBefore.begin(); iter != fBlocks[i].fBefore.end(); iter++) {
            printf("%s%s = %s", separator, iter->first.node().description().c_str(),
                   iter->second ? iter->second.node().description().c_str() : "<undefined>");
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
            printf("Node %d (%p): %s\n", (int) j, &n, n.id().node().description().c_str());
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

bool BasicBlock::tryRemoveExpressionBefore(std::vector<BasicBlock::Node>::iterator* iter,
                                           IRNode::ID id) {
    Expression& e = (Expression&) id.expressionNode();
    if (e.fKind == Expression::kTernary_Kind) {
        return false;
    }
    bool result;
    if ((*iter)->fKind == BasicBlock::Node::kExpression_Kind) {
        SkASSERT((*iter)->id() != id);
        IRNode::ID old = (*iter)->id();
        do {
            if ((*iter) == fNodes.begin()) {
                return false;
            }
            --(*iter);
        } while ((*iter)->fKind != BasicBlock::Node::kExpression_Kind ||
                 (*iter)->id() != id);
        result = this->tryRemoveExpression(iter);
        while ((*iter)->fKind != BasicBlock::Node::kExpression_Kind ||
               (*iter)->id() != old) {
            SkASSERT(*iter != fNodes.end());
            ++(*iter);
        }
    } else {
        IRNode::ID old = (*iter)->id();
        do {
            if ((*iter) == fNodes.begin()) {
                return false;
            }
            --(*iter);
        } while ((*iter)->fKind != BasicBlock::Node::kExpression_Kind ||
                 (*iter)->id() !=id);
        result = this->tryRemoveExpression(iter);
        while ((*iter)->fKind != BasicBlock::Node::kStatement_Kind ||
               (*iter)->id() != old) {
            SkASSERT(*iter != fNodes.end());
            ++(*iter);
        }
    }
    return result;
}

bool BasicBlock::tryRemoveLValueBefore(std::vector<BasicBlock::Node>::iterator* iter,
                                       IRNode::ID lvalueID) {
    Expression& lvalue = lvalueID.expressionNode();
    switch (lvalue.fKind) {
        case Expression::kExternalValue_Kind: // fall through
        case Expression::kVariableReference_Kind:
            return true;
        case Expression::kSwizzle_Kind:
            return this->tryRemoveLValueBefore(iter, ((Swizzle&) lvalue).fBase);
        case Expression::kFieldAccess_Kind:
            return this->tryRemoveLValueBefore(iter, ((FieldAccess&) lvalue).fBase);
        case Expression::kIndex_Kind:
            if (!this->tryRemoveLValueBefore(iter, ((IndexExpression&) lvalue).fBase)) {
                return false;
            }
            return this->tryRemoveExpressionBefore(iter, ((IndexExpression&) lvalue).fIndex);
        case Expression::kTernary_Kind:
            if (!this->tryRemoveExpressionBefore(iter, ((TernaryExpression&) lvalue).fTest)) {
                return false;
            }
            if (!this->tryRemoveLValueBefore(iter, ((TernaryExpression&) lvalue).fIfTrue)) {
                return false;
            }
            return this->tryRemoveLValueBefore(iter, ((TernaryExpression&) lvalue).fIfFalse);
        default:
            ABORT("invalid lvalue: %s\n", lvalue.description().c_str());
    }
}

bool BasicBlock::tryRemoveExpression(std::vector<BasicBlock::Node>::iterator* iter) {
    Expression& expr = (*iter)->expression();
    switch (expr.fKind) {
        case Expression::kBinary_Kind: {
            BinaryExpression& b = (BinaryExpression&) expr;
            if (b.fOperator == Token::EQ) {
                if (!this->tryRemoveLValueBefore(iter, b.fLeft)) {
                    return false;
                }
            } else if (!this->tryRemoveExpressionBefore(iter, b.fLeft)) {
                return false;
            }
            if (!this->tryRemoveExpressionBefore(iter, b.fRight)) {
                return false;
            }
            SkASSERT((*iter)->id() == (*iter)->id());
            *iter = fNodes.erase(*iter);
            return true;
        }
        case Expression::kTernary_Kind: {
            // ternaries cross basic block boundaries, must regenerate the CFG to remove it
            return false;
        }
        case Expression::kFieldAccess_Kind: {
            FieldAccess& f = (FieldAccess&) expr;
            if (!this->tryRemoveExpressionBefore(iter, f.fBase)) {
                return false;
            }
            *iter = fNodes.erase(*iter);
            return true;
        }
        case Expression::kSwizzle_Kind: {
            Swizzle& s = (Swizzle&) expr;
            if (!this->tryRemoveExpressionBefore(iter, s.fBase)) {
                return false;
            }
            *iter = fNodes.erase(*iter);
            return true;
        }
        case Expression::kIndex_Kind: {
            IndexExpression& idx = (IndexExpression&) expr;
            if (!this->tryRemoveExpressionBefore(iter, idx.fBase)) {
                return false;
            }
            if (!this->tryRemoveExpressionBefore(iter, idx.fIndex)) {
                return false;
            }
            *iter = fNodes.erase(*iter);
            return true;
        }
        case Expression::kConstructor_Kind: {
            Constructor& c = (Constructor&) expr;
            for (IRNode::ID arg : c.fArguments) {
                if (!this->tryRemoveExpressionBefore(iter, arg)) {
                    return false;
                }
                SkASSERT((*iter)->id() == expr);
            }
            *iter = fNodes.erase(*iter);
            return true;
        }
        case Expression::kFunctionCall_Kind: {
            FunctionCall& f = (FunctionCall&) expr;
            for (IRNode::ID arg : f.fArguments) {
                if (!this->tryRemoveExpressionBefore(iter, arg)) {
                    return false;
                }
                SkASSERT((*iter)->id() == expr);
            }
            *iter = fNodes.erase(*iter);
            return true;
        }
        case Expression::kPrefix_Kind:
            if (!this->tryRemoveExpressionBefore(iter,
                                                 ((PrefixExpression&) expr).fOperand)) {
                return false;
            }
            *iter = fNodes.erase(*iter);
            return true;
        case Expression::kPostfix_Kind:
            if (!this->tryRemoveExpressionBefore(iter,
                                                 ((PrefixExpression&) expr).fOperand)) {
                return false;
            }
            *iter = fNodes.erase(*iter);
            return true;
        case Expression::kBoolLiteral_Kind:  // fall through
        case Expression::kFloatLiteral_Kind: // fall through
        case Expression::kIntLiteral_Kind:   // fall through
        case Expression::kSetting_Kind:
            *iter = fNodes.erase(*iter);
            return true;
        case Expression::kVariableReference_Kind: {
            VariableReference& vr = (VariableReference&) expr;
            printf("DELETING REFERENCE TO %s (%d, %d)\n", vr.fVariable.node().description().c_str(), ((Variable&) vr.fVariable.node()).fReadCount, ((Variable&) vr.fVariable.node()).fWriteCount);
            if (vr.fRefKind != kRead_RefKind) {
                ((Variable&) vr.fVariable.node()).fWriteCount--;
            }
            if (vr.fRefKind != kWrite_RefKind) {
                ((Variable&) vr.fVariable.node()).fReadCount--;
            }
            printf("DELETED REFERENCE TO %s (%d, %d)\n", vr.fVariable.node().description().c_str(), ((Variable&) vr.fVariable.node()).fReadCount, ((Variable&) vr.fVariable.node()).fWriteCount);
            *iter = fNodes.erase(*iter);
            return true;
        }
        default:
            ABORT("unhandled expression: %s\n", expr.description().c_str());
    }
}

#define POINTER(id, node, field) { id, (size_t) ((char*) &(node).field - (char*) &node), -1 }

#define INDEX_POINTER(id, node, field, idx) { id,                                               \
                                              (size_t) ((char*) &(node).field - (char*) &node), \
                                              (int) idx }

bool BasicBlock::tryInsertExpression(std::vector<BasicBlock::Node>::iterator* iter,
                                     IRNodeIDPtr exprID) {
    Expression& expr = exprID.expressionNode();
    switch (expr.fKind) {
        case Expression::kBinary_Kind: {
            BinaryExpression& b = (BinaryExpression&) expr;
            if (!this->tryInsertExpression(iter, POINTER(exprID.id(), b, fRight))) {
                return false;
            }
            ++(*iter);
            if (!this->tryInsertExpression(iter, POINTER(exprID.id(), b, fLeft))) {
                return false;
            }
            ++(*iter);
            BasicBlock::Node node = { BasicBlock::Node::kExpression_Kind, true, exprID };
            *iter = fNodes.insert(*iter, node);
            return true;
        }
        case Expression::kBoolLiteral_Kind:  // fall through
        case Expression::kFloatLiteral_Kind: // fall through
        case Expression::kIntLiteral_Kind:   // fall through
        case Expression::kVariableReference_Kind: {
            BasicBlock::Node node = { BasicBlock::Node::kExpression_Kind, true, exprID };
            *iter = fNodes.insert(*iter, node);
            return true;
        }
        case Expression::kConstructor_Kind: {
            Constructor& c = (Constructor&) expr;
            for (size_t i = 0; i < c.fArguments.size(); ++i) {
                if (!this->tryInsertExpression(iter,
                                               INDEX_POINTER(exprID.id(), c, fArguments, i))) {
                    return false;
                }
                ++(*iter);
            }
            BasicBlock::Node node = { BasicBlock::Node::kExpression_Kind, true, exprID };
            *iter = fNodes.insert(*iter, node);
            return true;
        }
        default:
            return false;
    }
}

void CFGGenerator::addExpression(CFG& cfg, IRNodeIDPtr eID, bool constantPropagate) {
    Expression& e = eID.expressionNode();
    switch (e.fKind) {
        case Expression::kBinary_Kind: {
            BinaryExpression& b = (BinaryExpression&) e;
            switch (b.fOperator) {
                case Token::LOGICALAND: // fall through
                case Token::LOGICALOR: {
                    // this isn't as precise as it could be -- we don't bother to track that if we
                    // early exit from a logical and/or, we know which branch of an 'if' we're going
                    // to hit -- but it won't make much difference in practice.
                    this->addExpression(cfg, POINTER(eID.id(), b, fLeft), constantPropagate);
                    BlockId start = cfg.fCurrent;
                    cfg.newBlock();
                    this->addExpression(cfg, POINTER(eID.id(), b, fRight), constantPropagate);
                    cfg.newBlock();
                    cfg.addExit(start, cfg.fCurrent);
                    cfg.fBlocks[cfg.fCurrent].fNodes.push_back({
                        BasicBlock::Node::kExpression_Kind,
                        constantPropagate,
                        eID
                    });
                    break;
                }
                case Token::EQ: {
                    this->addExpression(cfg, POINTER(eID.id(), b, fRight), constantPropagate);
                    this->addLValue(cfg, POINTER(eID.id(), b, fLeft));
                    cfg.fBlocks[cfg.fCurrent].fNodes.push_back({
                        BasicBlock::Node::kExpression_Kind,
                        constantPropagate,
                        eID
                    });
                    break;
                }
                default:
                    this->addExpression(cfg, POINTER(eID.id(), b, fLeft),
                                        !Compiler::IsAssignment(b.fOperator));
                    this->addExpression(cfg, POINTER(eID.id(), b, fRight), constantPropagate);
                    cfg.fBlocks[cfg.fCurrent].fNodes.push_back({
                        BasicBlock::Node::kExpression_Kind,
                        constantPropagate,
                        eID
                    });
            }
            break;
        }
        case Expression::kConstructor_Kind: {
            Constructor& c = (Constructor&) e;
            for (size_t i = 0; i < c.fArguments.size(); ++i) {
                this->addExpression(cfg, INDEX_POINTER(eID.id(), c, fArguments, i),
                                    constantPropagate);
            }
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kExpression_Kind,
                                                         constantPropagate, eID });
            break;
        }
        case Expression::kExternalFunctionCall_Kind: {
            ExternalFunctionCall& c = (ExternalFunctionCall&) e;
            for (size_t i = 0; i < c.fArguments.size(); ++i) {
                this->addExpression(cfg, INDEX_POINTER(eID.id(), c, fArguments, i),
                                    constantPropagate);
            }
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kExpression_Kind,
                                                         constantPropagate, eID });
            break;
        }
        case Expression::kFunctionCall_Kind: {
            FunctionCall& c = (FunctionCall&) e;
            for (size_t i = 0; i < c.fArguments.size(); ++i) {
                this->addExpression(cfg, INDEX_POINTER(eID.id(), c, fArguments, i),
                                    constantPropagate);
            }
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kExpression_Kind,
                                                         constantPropagate, eID });
            break;
        }
        case Expression::kFieldAccess_Kind:
            this->addExpression(cfg, POINTER(eID.id(), (FieldAccess&) e, fBase), constantPropagate);
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kExpression_Kind,
                                                         constantPropagate, eID });
            break;
        case Expression::kIndex_Kind: {
            IndexExpression& i = (IndexExpression&) e;
            this->addExpression(cfg, POINTER(eID.id(), i, fBase), constantPropagate);
            this->addExpression(cfg, POINTER(eID.id(), i, fIndex), constantPropagate);
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kExpression_Kind,
                                                         constantPropagate, eID });
            break;
        }
        case Expression::kPrefix_Kind: {
            PrefixExpression& p = (PrefixExpression&) e;
            this->addExpression(cfg, POINTER(eID.id(), p, fOperand),
                                constantPropagate && p.fOperator != Token::PLUSPLUS &&
                                p.fOperator != Token::MINUSMINUS);
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kExpression_Kind,
                                                         constantPropagate, eID });
            break;
        }
        case Expression::kPostfix_Kind:
            this->addExpression(cfg, POINTER(eID.id(), ((PostfixExpression&) e), fOperand), false);
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kExpression_Kind,
                                                         constantPropagate, eID });
            break;
        case Expression::kSwizzle_Kind:
            this->addExpression(cfg, POINTER(eID.id(), ((Swizzle&) e), fBase), constantPropagate);
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kExpression_Kind,
                                                         constantPropagate, eID });
            break;
        case Expression::kAppendStage_Kind:   // fall through
        case Expression::kBoolLiteral_Kind:   // fall through
        case Expression::kExternalValue_Kind: // fall through
        case Expression::kFloatLiteral_Kind:  // fall through
        case Expression::kIntLiteral_Kind:    // fall through
        case Expression::kNullLiteral_Kind:   // fall through
        case Expression::kSetting_Kind:       // fall through
        case Expression::kVariableReference_Kind:
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kExpression_Kind,
                                                         constantPropagate, eID });
            break;
        case Expression::kTernary_Kind: {
            TernaryExpression& t = (TernaryExpression&) e;
            this->addExpression(cfg, POINTER(eID.id(), t, fTest), constantPropagate);
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kExpression_Kind,
                                                         constantPropagate, eID });
            BlockId start = cfg.fCurrent;
            cfg.newBlock();
            this->addExpression(cfg, POINTER(eID.id(), t, fIfTrue), constantPropagate);
            BlockId next = cfg.newBlock();
            cfg.fCurrent = start;
            cfg.newBlock();
            this->addExpression(cfg, POINTER(eID.id(), t, fIfFalse), constantPropagate);
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
void CFGGenerator::addLValue(CFG& cfg, IRNodeIDPtr eID) {
    Expression& e = eID.expressionNode();
    switch (e.fKind) {
        case Expression::kFieldAccess_Kind:
            this->addLValue(cfg, POINTER(eID.id(), (FieldAccess&) e, fBase));
            break;
        case Expression::kIndex_Kind:
            this->addLValue(cfg, POINTER(eID.id(), (IndexExpression&) e, fBase));
            this->addExpression(cfg, POINTER(eID.id(), (IndexExpression&) e, fIndex), true);
            break;
        case Expression::kSwizzle_Kind:
            this->addLValue(cfg, POINTER(eID.id(), (Swizzle&) e, fBase));
            break;
        case Expression::kExternalValue_Kind: // fall through
        case Expression::kVariableReference_Kind:
            break;
        case Expression::kTernary_Kind: {
            TernaryExpression& t = (TernaryExpression&) e;
            this->addExpression(cfg, POINTER(eID.id(), t, fTest), true);
            // Technically we will of course only evaluate one or the other, but if the test turns
            // out to be constant, the ternary will get collapsed down to just one branch anyway. So
            // it should be ok to pretend that we always evaluate both branches here.
            this->addLValue(cfg, POINTER(eID.id(), t, fIfTrue));
            this->addLValue(cfg, POINTER(eID.id(), t, fIfFalse));
            break;
        }
        default:
            // not an lvalue, can't happen
            SkASSERT(false);
            break;
    }
}

static bool is_true(Expression& expr) {
    return expr.fKind == Expression::kBoolLiteral_Kind && ((BoolLiteral&) expr).fValue;
}

void CFGGenerator::addStatement(CFG& cfg, IRNodeIDPtr sID) {
    Statement& s = sID.statementNode();
    switch (s.fKind) {
        case Statement::kBlock_Kind: {
            Block& b = (Block&) s;
            for (size_t i = 0; i < b.fStatements.size(); ++i) {
                addStatement(cfg, INDEX_POINTER(sID.id(), b, fStatements, i));
            }
            break;
        }
        case Statement::kIf_Kind: {
            IfStatement& ifs = (IfStatement&) s;
            this->addExpression(cfg, POINTER(sID.id(), ifs, fTest), true);
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kStatement_Kind, false,
                                                         sID });
            BlockId start = cfg.fCurrent;
            cfg.newBlock();
            this->addStatement(cfg, POINTER(sID.id(), ifs, fIfTrue));
            BlockId next = cfg.newBlock();
            if (ifs.fIfFalse) {
                cfg.fCurrent = start;
                cfg.newBlock();
                this->addStatement(cfg, POINTER(sID.id(), ifs, fIfFalse));
                cfg.addExit(cfg.fCurrent, next);
                cfg.fCurrent = next;
            } else {
                cfg.addExit(start, next);
            }
            break;
        }
        case Statement::kExpression_Kind: {
            this->addExpression(cfg, POINTER(sID.id(), (ExpressionStatement&) s, fExpression),
                                true);
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kStatement_Kind, false,
                                                         sID });
            break;
        }
        case Statement::kVarDeclarations_Kind: {
            VarDeclarationsStatement& decls = (VarDeclarationsStatement&) s;
            VarDeclarations& decl = (VarDeclarations&) decls.fDeclaration.node();
            for (size_t i = 0; i < decl.fVars.size(); ++i) {
                Statement& stmt = decl.fVars[i].statementNode();
                if (stmt.fKind == Statement::kNop_Kind) {
                    continue;
                }
                VarDeclaration& vd = (VarDeclaration&) stmt;
                if (vd.fValue) {
                    this->addExpression(cfg, POINTER(decl.fVars[i], vd, fValue), true);
                }
                cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kStatement_Kind,
                                                             false,
                                                             INDEX_POINTER(decls.fDeclaration, decl,
                                                                           fVars, i)  });
            }
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kStatement_Kind, false,
                                                         sID });
            break;
        }
        case Statement::kDiscard_Kind:
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kStatement_Kind, false,
                                                         sID });
            cfg.fCurrent = cfg.newIsolatedBlock();
            break;
        case Statement::kReturn_Kind: {
            ReturnStatement& r = (ReturnStatement&) s;
            if (r.fExpression) {
                this->addExpression(cfg, POINTER(sID.id(), r, fExpression), true);
            }
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kStatement_Kind, false,
                                                         sID });
            cfg.fCurrent = cfg.newIsolatedBlock();
            break;
        }
        case Statement::kBreak_Kind:
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kStatement_Kind, false,
                                                         sID });
            cfg.addExit(cfg.fCurrent, fLoopExits.top());
            cfg.fCurrent = cfg.newIsolatedBlock();
            break;
        case Statement::kContinue_Kind:
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kStatement_Kind, false,
                                                         sID });
            cfg.addExit(cfg.fCurrent, fLoopContinues.top());
            cfg.fCurrent = cfg.newIsolatedBlock();
            break;
        case Statement::kWhile_Kind: {
            WhileStatement& w = (WhileStatement&) s;
            BlockId loopStart = cfg.newBlock();
            fLoopContinues.push(loopStart);
            BlockId loopExit = cfg.newIsolatedBlock();
            fLoopExits.push(loopExit);
            this->addExpression(cfg, POINTER(sID.id(), w, fTest), true);
            BlockId test = cfg.fCurrent;
            if (!is_true(w.fTest.expressionNode())) {
                cfg.addExit(test, loopExit);
            }
            cfg.newBlock();
            this->addStatement(cfg, POINTER(sID.id(), w, fStatement));
            cfg.addExit(cfg.fCurrent, loopStart);
            fLoopContinues.pop();
            fLoopExits.pop();
            cfg.fCurrent = loopExit;
            break;
        }
        case Statement::kDo_Kind: {
            DoStatement& d = (DoStatement&) s;
            BlockId loopStart = cfg.newBlock();
            fLoopContinues.push(loopStart);
            BlockId loopExit = cfg.newIsolatedBlock();
            fLoopExits.push(loopExit);
            this->addStatement(cfg, POINTER(sID.id(), d, fStatement));
            this->addExpression(cfg, POINTER(sID.id(), d, fTest), true);
            cfg.addExit(cfg.fCurrent, loopExit);
            cfg.addExit(cfg.fCurrent, loopStart);
            fLoopContinues.pop();
            fLoopExits.pop();
            cfg.fCurrent = loopExit;
            break;
        }
        case Statement::kFor_Kind: {
            ForStatement& f = (ForStatement&) s;
            if (f.fInitializer) {
                this->addStatement(cfg, POINTER(sID.id(), f, fInitializer));
            }
            BlockId loopStart = cfg.newBlock();
            BlockId next = cfg.newIsolatedBlock();
            fLoopContinues.push(next);
            BlockId loopExit = cfg.newIsolatedBlock();
            fLoopExits.push(loopExit);
            if (f.fTest) {
                this->addExpression(cfg, POINTER(sID.id(), f, fTest), true);
                // this isn't quite right; we should have an exit from here to the loop exit, and
                // remove the exit from the loop body to the loop exit. Structuring it like this
                // forces the optimizer to believe that the loop body is always executed at least
                // once. While not strictly correct, this avoids incorrect "variable not assigned"
                // errors on variables which are assigned within the loop. The correct solution to
                // this is to analyze the loop to see whether or not at least one iteration is
                // guaranteed to happen, but for the time being we take the easy way out.
            }
            cfg.newBlock();
            this->addStatement(cfg, POINTER(sID.id(), f, fStatement));
            cfg.addExit(cfg.fCurrent, next);
            cfg.fCurrent = next;
            if (f.fNextExpression) {
                this->addExpression(cfg, POINTER(sID.id(), f, fNextExpression), true);
            }
            cfg.addExit(cfg.fCurrent, loopStart);
            cfg.addExit(cfg.fCurrent, loopExit);
            fLoopContinues.pop();
            fLoopExits.pop();
            cfg.fCurrent = loopExit;
            break;
        }
        case Statement::kSwitch_Kind: {
            SwitchStatement& ss = (SwitchStatement&) s;
            this->addExpression(cfg, POINTER(sID.id(), ss, fValue), true);
            cfg.fBlocks[cfg.fCurrent].fNodes.push_back({ BasicBlock::Node::kStatement_Kind, false,
                                                         sID });
            BlockId start = cfg.fCurrent;
            BlockId switchExit = cfg.newIsolatedBlock();
            fLoopExits.push(switchExit);
            for (size_t i = 0; i < ss.fCases.size(); ++i) {
                SwitchCase& c = (SwitchCase&) ss.fCases[i].node();
                cfg.newBlock();
                cfg.addExit(start, cfg.fCurrent);
                if (c.fValue) {
                    // technically this should go in the start block, but it doesn't actually matter
                    // because it must be constant. Not worth running two loops for.
                    this->addExpression(cfg, POINTER(ss.fCases[i], c, fValue), true);
                }
                for (size_t j = 0; j < c.fStatements.size(); ++j) {
                    this->addStatement(cfg, INDEX_POINTER(ss.fCases[i], c, fStatements, j));
                }
            }
            cfg.addExit(cfg.fCurrent, switchExit);
            // note that unlike GLSL, our grammar requires the default case to be last
            if (0 == ss.fCases.size() || ss.fCases[ss.fCases.size() - 1].fValue) {
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
            printf("statement: %s\n", s.description().c_str());
            ABORT("unsupported statement kind");
    }
}

CFG CFGGenerator::getCFG(IRNode::ID functionDefinition) {
    CFG result;
    result.fStart = result.newBlock();
    result.fCurrent = result.fStart;
    this->addStatement(result, POINTER(functionDefinition,
                                       (FunctionDefinition&) functionDefinition.node(),
                                       fBody));
    result.newBlock();
    result.fExit = result.fCurrent;
    return result;
}

} // namespace
