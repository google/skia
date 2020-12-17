/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CFGGENERATOR
#define SKSL_CFGGENERATOR

#include "include/private/SkTArray.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"

#include <stack>

namespace SkSL {

class ProgramUsage;

// index of a block within CFG.fBlocks
typedef size_t BlockId;

struct BasicBlock {
    struct Node {
        Node(std::unique_ptr<Statement>* statement)
                : fConstantPropagation(false)
                , fExpression(nullptr)
                , fStatement(statement) {}

        Node(std::unique_ptr<Expression>* expression, bool constantPropagation)
                : fConstantPropagation(constantPropagation)
                , fExpression(expression)
                , fStatement(nullptr) {}

        bool isExpression() const {
            return fExpression != nullptr;
        }

        std::unique_ptr<Expression>* expression() const {
            SkASSERT(!this->isStatement());
            return fExpression;
        }

        // See comment below on setStatement. Assumption is that 'expr' is a strict subset of the
        // existing expression.
        void setExpression(std::unique_ptr<Expression> expr, ProgramUsage* usage);

        bool isStatement() const {
            return fStatement != nullptr;
        }

        std::unique_ptr<Statement>* statement() const {
            SkASSERT(!this->isExpression());
            return fStatement;
        }

        // Replaces the pointed-to statement with 'stmt'. The assumption is that 'stmt' is a strict
        // subset of the existing statement, or a Nop. For example: just the True or False of an if,
        // or a single Case from a Switch. To maintain usage's bookkeeping, we remove references in
        // this node's pointed-to statement. By the time this is called, there is no path from our
        // statement to 'stmt', because it's been moved to the argument.
        void setStatement(std::unique_ptr<Statement> stmt, ProgramUsage* usage);

#ifdef SK_DEBUG
        String description() const {
            SkASSERT(fStatement || fExpression);
            return fStatement ? (*fStatement)->description() : (*fExpression)->description();
        }
#endif

        // if false, this node should not be subject to constant propagation. This happens with
        // compound assignment (i.e. x *= 2), in which the value x is used as an rvalue for
        // multiplication by 2 and then as an lvalue for assignment purposes. Since there is only
        // one "x" node, replacing it with a constant would break the assignment and we suppress
        // it. Down the road, we should handle this more elegantly by substituting a regular
        // assignment if the target is constant (i.e. x = 1; x *= 2; should become x = 1; x = 1 * 2;
        // and then collapse down to a simple x = 2;).
        bool fConstantPropagation;

    private:
        // we store pointers to the unique_ptrs so that we can replace expressions or statements
        // during optimization without having to regenerate the entire tree
        std::unique_ptr<Expression>* fExpression;
        std::unique_ptr<Statement>* fStatement;
    };

    static Node MakeStatement(std::unique_ptr<Statement>* stmt) {
        return Node{stmt};
    }

    static Node MakeExpression(std::unique_ptr<Expression>* expr, bool constantPropagation) {
        return Node{expr, constantPropagation};
    }

    /**
     * Attempts to remove the expression (and its subexpressions) pointed to by the iterator. If the
     * expression can be cleanly removed, returns true and updates the iterator to point to the
     * expression after the deleted expression. Otherwise returns false (and the CFG will need to be
     * regenerated).
     */
    bool tryRemoveExpression(std::vector<BasicBlock::Node>::iterator* iter);

    /**
     * Locates and attempts remove an expression occurring before the expression pointed to by iter.
     * If the expression can be cleanly removed, returns true and resets iter to a valid iterator
     * pointing to the same expression it did initially. Otherwise returns false (and the CFG will
     * need to be regenerated).
     */
    bool tryRemoveExpressionBefore(std::vector<BasicBlock::Node>::iterator* iter, Expression* e);

    /**
     * As tryRemoveExpressionBefore, but for lvalues. As lvalues are at most partially evaluated
     * (for instance, x[i] = 0 evaluates i but not x) this will only look for the parts of the
     * lvalue that are actually evaluated.
     */
    bool tryRemoveLValueBefore(std::vector<BasicBlock::Node>::iterator* iter, Expression* lvalue);

    /**
     * Attempts to inserts a new expression before the node pointed to by iter. If the
     * expression can be cleanly inserted, returns true and updates the iterator to point to the
     * newly inserted expression. Otherwise returns false (and the CFG will need to be regenerated).
     */
    bool tryInsertExpression(std::vector<BasicBlock::Node>::iterator* iter,
                             std::unique_ptr<Expression>* expr);

#ifdef SK_DEBUG
    void dump() const;
#endif

    std::vector<Node> fNodes;
    bool fIsReachable = false;
    bool fAllowUnreachable = false;
    using ExitArray = SkSTArray<4, BlockId>;
    ExitArray fExits;
    // variable definitions upon entering this basic block (null expression = undefined)
    DefinitionMap fBefore;
};

struct CFG {
    BlockId fStart;
    BlockId fExit;
    std::vector<BasicBlock> fBlocks;

#ifdef SK_DEBUG
    void dump() const;
#endif

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

    // Convenience method to return the CFG's current block.
    BasicBlock& currentBlock() { return fBlocks[fCurrent]; }

    friend class CFGGenerator;
};

/**
 * Converts functions into control flow graphs.
 */
class CFGGenerator {
public:
    CFGGenerator() {}

    CFG getCFG(FunctionDefinition& f);

private:
    void addStatement(CFG& cfg, std::unique_ptr<Statement>* s);

    void addExpression(CFG& cfg, std::unique_ptr<Expression>* e, bool constantPropagate);

    void addLValue(CFG& cfg, std::unique_ptr<Expression>* e);

    std::stack<BlockId> fLoopContinues;
    std::stack<BlockId> fLoopExits;
};

}  // namespace SkSL

#endif
