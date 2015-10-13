//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// UnfoldShortCircuitToIf is an AST traverser to convert short-circuiting operators to if-else statements.
// The results are assigned to s# temporaries, which are used by the main translator instead of
// the original expression.
//

#include "compiler/translator/UnfoldShortCircuitToIf.h"

#include "compiler/translator/IntermNode.h"

namespace
{

// Traverser that unfolds one short-circuiting operation at a time.
class UnfoldShortCircuitTraverser : public TIntermTraverser
{
  public:
    UnfoldShortCircuitTraverser();

    bool visitBinary(Visit visit, TIntermBinary *node) override;
    bool visitAggregate(Visit visit, TIntermAggregate *node) override;
    bool visitSelection(Visit visit, TIntermSelection *node) override;

    void nextIteration();
    bool foundShortCircuit() const { return mFoundShortCircuit; }

  protected:
    // Marked to true once an operation that needs to be unfolded has been found.
    // After that, no more unfolding is performed on that traversal.
    bool mFoundShortCircuit;
};

UnfoldShortCircuitTraverser::UnfoldShortCircuitTraverser()
    : TIntermTraverser(true, false, true),
      mFoundShortCircuit(false)
{
}

bool UnfoldShortCircuitTraverser::visitBinary(Visit visit, TIntermBinary *node)
{
    if (mFoundShortCircuit)
        return false;
    // If our right node doesn't have side effects, we know we don't need to unfold this
    // expression: there will be no short-circuiting side effects to avoid
    // (note: unfolding doesn't depend on the left node -- it will always be evaluated)
    if (!node->getRight()->hasSideEffects())
    {
        return true;
    }

    switch (node->getOp())
    {
      case EOpLogicalOr:
        mFoundShortCircuit = true;

        // "x || y" is equivalent to "x ? true : y", which unfolds to "bool s; if(x) s = true; else s = y;",
        // and then further simplifies down to "bool s = x; if(!s) s = y;".
        {
            TIntermSequence insertions;
            TType boolType(EbtBool, EbpUndefined, EvqTemporary);

            ASSERT(node->getLeft()->getType() == boolType);
            insertions.push_back(createTempInitDeclaration(node->getLeft()));

            TIntermAggregate *assignRightBlock = new TIntermAggregate(EOpSequence);
            ASSERT(node->getRight()->getType() == boolType);
            assignRightBlock->getSequence()->push_back(createTempAssignment(node->getRight()));

            TIntermUnary *notTempSymbol = new TIntermUnary(EOpLogicalNot, boolType);
            notTempSymbol->setOperand(createTempSymbol(boolType));
            TIntermSelection *ifNode = new TIntermSelection(notTempSymbol, assignRightBlock, nullptr);
            insertions.push_back(ifNode);

            insertStatementsInParentBlock(insertions);

            NodeUpdateEntry replaceVariable(getParentNode(), node, createTempSymbol(boolType), false);
            mReplacements.push_back(replaceVariable);
        }
        return false;
      case EOpLogicalAnd:
        mFoundShortCircuit = true;

        // "x && y" is equivalent to "x ? y : false", which unfolds to "bool s; if(x) s = y; else s = false;",
        // and then further simplifies down to "bool s = x; if(s) s = y;".
        {
            TIntermSequence insertions;
            TType boolType(EbtBool, EbpUndefined, EvqTemporary);

            ASSERT(node->getLeft()->getType() == boolType);
            insertions.push_back(createTempInitDeclaration(node->getLeft()));

            TIntermAggregate *assignRightBlock = new TIntermAggregate(EOpSequence);
            ASSERT(node->getRight()->getType() == boolType);
            assignRightBlock->getSequence()->push_back(createTempAssignment(node->getRight()));

            TIntermSelection *ifNode = new TIntermSelection(createTempSymbol(boolType), assignRightBlock, nullptr);
            insertions.push_back(ifNode);

            insertStatementsInParentBlock(insertions);

            NodeUpdateEntry replaceVariable(getParentNode(), node, createTempSymbol(boolType), false);
            mReplacements.push_back(replaceVariable);
        }
        return false;
      default:
        return true;
    }
}

bool UnfoldShortCircuitTraverser::visitSelection(Visit visit, TIntermSelection *node)
{
    if (mFoundShortCircuit)
        return false;

    // Unfold "b ? x : y" into "type s; if(b) s = x; else s = y;"
    if (visit == PreVisit && node->usesTernaryOperator())
    {
        mFoundShortCircuit = true;
        TIntermSequence insertions;

        TIntermSymbol *tempSymbol = createTempSymbol(node->getType());
        TIntermAggregate *tempDeclaration = new TIntermAggregate(EOpDeclaration);
        tempDeclaration->getSequence()->push_back(tempSymbol);
        insertions.push_back(tempDeclaration);

        TIntermAggregate *trueBlock = new TIntermAggregate(EOpSequence);
        TIntermBinary *trueAssignment = createTempAssignment(node->getTrueBlock()->getAsTyped());
        trueBlock->getSequence()->push_back(trueAssignment);

        TIntermAggregate *falseBlock = new TIntermAggregate(EOpSequence);
        TIntermBinary *falseAssignment = createTempAssignment(node->getFalseBlock()->getAsTyped());
        falseBlock->getSequence()->push_back(falseAssignment);

        TIntermSelection *ifNode = new TIntermSelection(node->getCondition()->getAsTyped(), trueBlock, falseBlock);
        insertions.push_back(ifNode);

        insertStatementsInParentBlock(insertions);

        TIntermSymbol *ternaryResult = createTempSymbol(node->getType());
        NodeUpdateEntry replaceVariable(getParentNode(), node, ternaryResult, false);
        mReplacements.push_back(replaceVariable);
        return false;
    }

    return true;
}

bool UnfoldShortCircuitTraverser::visitAggregate(Visit visit, TIntermAggregate *node)
{
    if (visit == PreVisit && mFoundShortCircuit)
        return false; // No need to traverse further

    if (node->getOp() == EOpComma)
    {
        ASSERT(visit != PreVisit || !mFoundShortCircuit);

        if (visit == PostVisit && mFoundShortCircuit)
        {
            // We can be sure that we arrived here because there was a short-circuiting operator
            // inside the sequence operator since we only start traversing the sequence operator in
            // case a short-circuiting operator has not been found so far.
            // We need to unfold the sequence (comma) operator, otherwise the evaluation order of
            // statements would be messed up by unfolded operations inside.
            // Don't do any other unfolding on this round of traversal.
            mReplacements.clear();
            mMultiReplacements.clear();
            mInsertions.clear();

            TIntermSequence insertions;
            TIntermSequence *seq = node->getSequence();

            TIntermSequence::size_type i = 0;
            ASSERT(!seq->empty());
            while (i < seq->size() - 1)
            {
                TIntermTyped *child = (*seq)[i]->getAsTyped();
                insertions.push_back(child);
                ++i;
            }

            insertStatementsInParentBlock(insertions);

            NodeUpdateEntry replaceVariable(getParentNode(), node, (*seq)[i], false);
            mReplacements.push_back(replaceVariable);
        }
    }
    return true;
}

void UnfoldShortCircuitTraverser::nextIteration()
{
    mFoundShortCircuit = false;
    nextTemporaryIndex();
}

} // namespace

void UnfoldShortCircuitToIf(TIntermNode *root, unsigned int *temporaryIndex)
{
    UnfoldShortCircuitTraverser traverser;
    ASSERT(temporaryIndex != nullptr);
    traverser.useTemporaryIndex(temporaryIndex);
    // Unfold one operator at a time, and reset the traverser between iterations.
    do
    {
        traverser.nextIteration();
        root->traverse(&traverser);
        if (traverser.foundShortCircuit())
            traverser.updateTree();
    }
    while (traverser.foundShortCircuit());
}
