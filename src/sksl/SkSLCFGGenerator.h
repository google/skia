/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CFGGENERATOR
#define SKSL_CFGGENERATOR

#include "src/sksl/ir/SkSLFunctionDefinition.h"

#include <set>
#include <stack>

namespace SkSL {

struct Expression;
struct Statement;

// index of a block within CFG.fBlocks
typedef size_t BlockId;

struct BasicBlock {
    struct Node {
        enum Kind {
            kStatement_Kind,
            kExpression_Kind
        };

        Node(Kind kind, bool constantPropagation, IRNodeIDPtr id)
        : fKind(kind)
        , fConstantPropagation(constantPropagation)
        , fID(id) {}

        IRNode::ID id() const {
            return *fID.ptr();
        }

        IRNodeIDPtr idPtr() const {
            return fID;
        }

        void setID(IRNode::ID id) {
            SkASSERT(id != fID.fParent);
            *fID.ptr() = id;
        }

        Expression& expression() const {
            return fID.ptr()->expression();
        }

        Statement& statement() const {
            return fID.ptr()->statement();
        }

        String description() const {
            return fID.ptr()->node().description();
        }

        Kind fKind;
        // if false, this node should not be subject to constant propagation. This happens with
        // compound assignment (i.e. x *= 2), in which the value x is used as an rvalue for
        // multiplication by 2 and then as an lvalue for assignment purposes. Since there is only
        // one "x" node, replacing it with a constant would break the assignment and we suppress
        // it. Down the road, we should handle this more elegantly by substituting a regular
        // assignment if the target is constant (i.e. x = 1; x *= 2; should become x = 1; x = 1 * 2;
        // and then collapse down to a simple x = 2;).
        bool fConstantPropagation;

    private:
        // we store pointers to the ids so that we can replace expressions or statements during
        // optimization without having to regenerate the entire tree
        IRNodeIDPtr fID;
    };

    /**
     * Attempts to remove the expression (and its subexpressions) pointed to by the iterator. If the
     * expression can be cleanly removed, returns true and updates the iterator to point to the
     * expression after the deleted expression. Otherwise returns false (and the CFG will need to be
     * regenerated).
     */
    bool tryRemoveExpression(std::vector<BasicBlock::Node>::iterator* iter);

    /**
     * Locates and attempts to remove an expression occurring before the expression pointed to by
     * iter. If the expression can be cleanly removed, returns true and resets iter to a valid
     * iterator pointing to the same expression it did initially. Otherwise returns false (and the
     * CFG will need to be regenerated).
     */
    bool tryRemoveExpressionBefore(std::vector<BasicBlock::Node>::iterator* iter, IRNode::ID e);

    /**
     * As tryRemoveExpressionBefore, but for lvalues. As lvalues are at most partially evaluated
     * (for instance, x[i] = 0 evaluates i but not x) this will only look for the parts of the
     * lvalue that are actually evaluated.
     */
    bool tryRemoveLValueBefore(std::vector<BasicBlock::Node>::iterator* iter, IRNode::ID lvalue);

    /**
     * Attempts to inserts a new expression before the node pointed to by iter. If the
     * expression can be cleanly inserted, returns true and updates the iterator to point to the
     * newly inserted expression. Otherwise returns false (and the CFG will need to be regenerated).
     */
    bool tryInsertExpression(std::vector<BasicBlock::Node>::iterator* iter, IRNodeIDPtr expr);

    std::vector<Node> fNodes;
    std::set<BlockId> fEntrances;
    std::set<BlockId> fExits;
    // variable definitions upon entering this basic block (null expression = undefined)
    DefinitionMap fBefore;
};

struct CFG {
    BlockId fStart;
    BlockId fExit;
    std::vector<BasicBlock> fBlocks;

    void dump();

private:
    BlockId fCurrent;

    // Adds a new block, adds an exit* from the current block to the new block, then marks the new
    // block as the current block
    // *see note in addExit()
    BlockId newBlock();

    // Adds a new block, but does not mark it current or add an exit from the current block
    BlockId newIsolatedBlock();

    // Adds an exit from the 'from' block to the 'to' block
    // Note that we skip adding the exit if the 'from' block is itself unreachable; this means that
    // we don't actually have to trace the tree to see if a particular block is unreachable, we can
    // just check to see if it has any entrances. This does require a bit of care in the order in
    // which we set the CFG up.
    void addExit(BlockId from, BlockId to);

    friend class CFGGenerator;
};

/**
 * Converts functions into control flow graphs.
 */
class CFGGenerator {
public:
    CFGGenerator() {}

    CFG getCFG(IRNode::ID functionDefinition);

private:
    void addStatement(CFG& cfg, IRNodeIDPtr id);

    void addExpression(CFG& cfg, IRNodeIDPtr id, bool constantPropagate);

    void addLValue(CFG& cfg, IRNodeIDPtr e);

    std::stack<BlockId> fLoopContinues;
    std::stack<BlockId> fLoopExits;
};

}

#endif
