/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CFGGENERATOR
#define SKSL_CFGGENERATOR

#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"

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

        Node(Kind kind, bool constantPropagation, std::unique_ptr<Expression>* expression,
             std::unique_ptr<Statement>* statement)
        : fKind(kind)
        , fConstantPropagation(constantPropagation)
        , fExpression(expression)
        , fStatement(statement) {}

        std::unique_ptr<Expression>* expression() const {
            SkASSERT(fKind == kExpression_Kind);
            return fExpression;
        }

        void setExpression(std::unique_ptr<Expression> expr) {
            SkASSERT(fKind == kExpression_Kind);
            *fExpression = std::move(expr);
        }

        std::unique_ptr<Statement>* statement() const {
            SkASSERT(fKind == kStatement_Kind);
            return fStatement;
        }

        void setStatement(std::unique_ptr<Statement> stmt) {
            SkASSERT(fKind == kStatement_Kind);
            *fStatement = std::move(stmt);
        }

        String description() const {
            if (fKind == kStatement_Kind) {
                return (*fStatement)->description();
            } else {
                SkASSERT(fKind == kExpression_Kind);
                return (*fExpression)->description();
            }
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
        // we store pointers to the unique_ptrs so that we can replace expressions or statements
        // during optimization without having to regenerate the entire tree
        std::unique_ptr<Expression>* fExpression;
        std::unique_ptr<Statement>* fStatement;
    };

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

    CFG getCFG(FunctionDefinition& f);

private:
    void addStatement(CFG& cfg, std::unique_ptr<Statement>* s);

    void addExpression(CFG& cfg, std::unique_ptr<Expression>* e, bool constantPropagate);

    void addLValue(CFG& cfg, std::unique_ptr<Expression>* e);

    std::stack<BlockId> fLoopContinues;
    std::stack<BlockId> fLoopExits;
};

}

#endif
