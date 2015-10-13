//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SeparateExpressionsReturningArrays splits array-returning expressions that are not array names from more complex
// expressions, assigning them to a temporary variable a#.
// Examples where a, b and c are all arrays:
// (a = b) == (a = c) is split into a = b; type[n] a1 = a; a = c; type[n] a2 = a; a1 == a2;
// type d = type[n](...)[i]; is split into type[n] a1 = type[n](...); type d = a1[i];

#include "compiler/translator/SeparateExpressionsReturningArrays.h"

#include "compiler/translator/IntermNode.h"

namespace
{

// Traverser that separates one array expression into a statement at a time.
class SeparateExpressionsTraverser : public TIntermTraverser
{
  public:
    SeparateExpressionsTraverser();

    bool visitBinary(Visit visit, TIntermBinary *node) override;
    bool visitAggregate(Visit visit, TIntermAggregate *node) override;

    void nextIteration();
    bool foundArrayExpression() const { return mFoundArrayExpression; }

  protected:
    // Marked to true once an operation that needs to be hoisted out of the expression has been found.
    // After that, no more AST updates are performed on that traversal.
    bool mFoundArrayExpression;
};

SeparateExpressionsTraverser::SeparateExpressionsTraverser()
    : TIntermTraverser(true, false, false),
      mFoundArrayExpression(false)
{
}

// Performs a shallow copy of an assignment node.
// These shallow copies are useful when a node gets inserted into an aggregate node
// and also needs to be replaced in its original location by a different node.
TIntermBinary *CopyAssignmentNode(TIntermBinary *node)
{
    TIntermBinary *copyNode = new TIntermBinary(node->getOp());
    copyNode->setLeft(node->getLeft());
    copyNode->setRight(node->getRight());
    copyNode->setType(node->getType());
    return copyNode;
}

// Performs a shallow copy of a constructor/function call node.
TIntermAggregate *CopyAggregateNode(TIntermAggregate *node)
{
    TIntermAggregate *copyNode = new TIntermAggregate(node->getOp());
    TIntermSequence *copySeq = copyNode->getSequence();
    copySeq->insert(copySeq->begin(), node->getSequence()->begin(), node->getSequence()->end());
    copyNode->setType(node->getType());
    copyNode->setFunctionId(node->getFunctionId());
    if (node->isUserDefined())
    {
        copyNode->setUserDefined();
    }
    copyNode->setName(node->getName());
    return copyNode;
}

bool SeparateExpressionsTraverser::visitBinary(Visit visit, TIntermBinary *node)
{
    if (mFoundArrayExpression)
        return false;

    // Early return if the expression is not an array or if we're not inside a complex expression.
    if (!node->getType().isArray() || parentNodeIsBlock())
        return true;

    switch (node->getOp())
    {
      case EOpAssign:
        {
            mFoundArrayExpression = true;

            TIntermSequence insertions;
            insertions.push_back(CopyAssignmentNode(node));
            // TODO(oetuaho): In some cases it would be more optimal to not add the temporary node, but just use the
            // original target of the assignment. Care must be taken so that this doesn't happen when the same array
            // symbol is a target of assignment more than once in one expression.
            insertions.push_back(createTempInitDeclaration(node->getLeft()));
            insertStatementsInParentBlock(insertions);

            NodeUpdateEntry replaceVariable(getParentNode(), node, createTempSymbol(node->getType()), false);
            mReplacements.push_back(replaceVariable);
        }
        return false;
      default:
        return true;
    }
}

bool SeparateExpressionsTraverser::visitAggregate(Visit visit, TIntermAggregate *node)
{
    if (mFoundArrayExpression)
        return false; // No need to traverse further

    if (getParentNode() != nullptr)
    {
        TIntermBinary *parentBinary = getParentNode()->getAsBinaryNode();
        bool parentIsAssignment = (parentBinary != nullptr &&
            (parentBinary->getOp() == EOpAssign || parentBinary->getOp() == EOpInitialize));

        if (!node->getType().isArray() || parentNodeIsBlock() || parentIsAssignment)
            return true;

        if (node->isConstructor())
        {
            mFoundArrayExpression = true;

            TIntermSequence insertions;
            insertions.push_back(createTempInitDeclaration(CopyAggregateNode(node)));
            insertStatementsInParentBlock(insertions);

            NodeUpdateEntry replaceVariable(getParentNode(), node, createTempSymbol(node->getType()), false);
            mReplacements.push_back(replaceVariable);

            return false;
        }
        else if (node->getOp() == EOpFunctionCall)
        {
            mFoundArrayExpression = true;

            TIntermSequence insertions;
            insertions.push_back(createTempInitDeclaration(CopyAggregateNode(node)));
            insertStatementsInParentBlock(insertions);

            NodeUpdateEntry replaceVariable(getParentNode(), node, createTempSymbol(node->getType()), false);
            mReplacements.push_back(replaceVariable);

            return false;
        }
    }
    return true;
}

void SeparateExpressionsTraverser::nextIteration()
{
    mFoundArrayExpression = false;
    nextTemporaryIndex();
}

} // namespace

void SeparateExpressionsReturningArrays(TIntermNode *root, unsigned int *temporaryIndex)
{
    SeparateExpressionsTraverser traverser;
    ASSERT(temporaryIndex != nullptr);
    traverser.useTemporaryIndex(temporaryIndex);
    // Separate one expression at a time, and reset the traverser between iterations.
    do
    {
        traverser.nextIteration();
        root->traverse(&traverser);
        if (traverser.foundArrayExpression())
            traverser.updateTree();
    }
    while (traverser.foundArrayExpression());
}
