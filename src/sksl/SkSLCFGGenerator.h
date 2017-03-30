/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CFGGENERATOR
#define SKSL_CFGGENERATOR

#include "ir/SkSLExpression.h"
#include "ir/SkSLFunctionDefinition.h"

#include <set>
#include <stack>

namespace SkSL {

// index of a block within CFG.fBlocks
typedef size_t BlockId;

struct BasicBlock {
    struct Node {
        enum Kind {
            kStatement_Kind,
            kExpression_Kind
        };

        Kind fKind;
        // if false, this node should not be subject to constant propagation. This happens with
        // compound assignment (i.e. x *= 2), in which the value x is used as an rvalue for
        // multiplication by 2 and then as an lvalue for assignment purposes. Since there is only
        // one "x" node, replacing it with a constant would break the assignment and we suppress
        // it. Down the road, we should handle this more elegantly by substituting a regular
        // assignment if the target is constant (i.e. x = 1; x *= 2; should become x = 1; x = 1 * 2;
        // and then collapse down to a simple x = 2;).
        bool fConstantPropagation;
        std::unique_ptr<Expression>* fExpression;
        const Statement* fStatement;
    };

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

    CFG getCFG(const FunctionDefinition& f);

private:
    void addStatement(CFG& cfg, const Statement* s);

    void addExpression(CFG& cfg, std::unique_ptr<Expression>* e, bool constantPropagate);

    void addLValue(CFG& cfg, std::unique_ptr<Expression>* e);

    std::stack<BlockId> fLoopContinues;
    std::stack<BlockId> fLoopExits;
};

}

#endif
