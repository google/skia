//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

//
// Build the intermediate representation.
//

#include <float.h>
#include <limits.h>
#include <algorithm>

#include "compiler/translator/Intermediate.h"
#include "compiler/translator/SymbolTable.h"

////////////////////////////////////////////////////////////////////////////
//
// First set of functions are to help build the intermediate representation.
// These functions are not member functions of the nodes.
// They are called from parser productions.
//
/////////////////////////////////////////////////////////////////////////////

//
// Add a terminal node for an identifier in an expression.
//
// Returns the added node.
//
TIntermSymbol *TIntermediate::addSymbol(
    int id, const TString &name, const TType &type, const TSourceLoc &line)
{
    TIntermSymbol *node = new TIntermSymbol(id, name, type);
    node->setLine(line);

    return node;
}

//
// Connect two nodes with a new parent that does a binary operation on the nodes.
//
// Returns the added node.
//
TIntermTyped *TIntermediate::addBinaryMath(
    TOperator op, TIntermTyped *left, TIntermTyped *right, const TSourceLoc &line)
{
    //
    // Need a new node holding things together then.  Make
    // one and promote it to the right type.
    //
    TIntermBinary *node = new TIntermBinary(op);
    node->setLine(line);

    node->setLeft(left);
    node->setRight(right);
    if (!node->promote(mInfoSink))
        return NULL;

    // See if we can fold constants.
    TIntermTyped *foldedNode = node->fold(mInfoSink);
    if (foldedNode)
        return foldedNode;

    return node;
}

//
// Connect two nodes through an assignment.
//
// Returns the added node.
//
TIntermTyped *TIntermediate::addAssign(
    TOperator op, TIntermTyped *left, TIntermTyped *right, const TSourceLoc &line)
{
    if (left->getType().getStruct() || right->getType().getStruct())
    {
        if (left->getType() != right->getType())
        {
            return NULL;
        }
    }

    TIntermBinary *node = new TIntermBinary(op);
    node->setLine(line);

    node->setLeft(left);
    node->setRight(right);
    if (!node->promote(mInfoSink))
        return NULL;

    return node;
}

//
// Connect two nodes through an index operator, where the left node is the base
// of an array or struct, and the right node is a direct or indirect offset.
//
// Returns the added node.
// The caller should set the type of the returned node.
//
TIntermTyped *TIntermediate::addIndex(
    TOperator op, TIntermTyped *base, TIntermTyped *index, const TSourceLoc &line)
{
    TIntermBinary *node = new TIntermBinary(op);
    node->setLine(line);
    node->setLeft(base);
    node->setRight(index);

    // caller should set the type

    return node;
}

//
// Add one node as the parent of another that it operates on.
//
// Returns the added node.
//
TIntermTyped *TIntermediate::addUnaryMath(
    TOperator op, TIntermTyped *child, const TSourceLoc &line, const TType *funcReturnType)
{
    //
    // Make a new node for the operator.
    //
    TIntermUnary *node = new TIntermUnary(op);
    node->setLine(line);
    node->setOperand(child);
    node->promote(funcReturnType);

    TIntermTyped *foldedNode = node->fold(mInfoSink);
    if (foldedNode)
        return foldedNode;

    return node;
}

//
// This is the safe way to change the operator on an aggregate, as it
// does lots of error checking and fixing.  Especially for establishing
// a function call's operation on it's set of parameters.  Sequences
// of instructions are also aggregates, but they just direnctly set
// their operator to EOpSequence.
//
// Returns an aggregate node, which could be the one passed in if
// it was already an aggregate but no operator was set.
//
TIntermAggregate *TIntermediate::setAggregateOperator(
    TIntermNode *node, TOperator op, const TSourceLoc &line)
{
    TIntermAggregate *aggNode;

    //
    // Make sure we have an aggregate.  If not turn it into one.
    //
    if (node)
    {
        aggNode = node->getAsAggregate();
        if (aggNode == NULL || aggNode->getOp() != EOpNull)
        {
            //
            // Make an aggregate containing this node.
            //
            aggNode = new TIntermAggregate();
            aggNode->getSequence()->push_back(node);
        }
    }
    else
    {
        aggNode = new TIntermAggregate();
    }

    //
    // Set the operator.
    //
    aggNode->setOp(op);
    aggNode->setLine(line);

    return aggNode;
}

//
// Safe way to combine two nodes into an aggregate.  Works with null pointers,
// a node that's not a aggregate yet, etc.
//
// Returns the resulting aggregate, unless 0 was passed in for
// both existing nodes.
//
TIntermAggregate *TIntermediate::growAggregate(
    TIntermNode *left, TIntermNode *right, const TSourceLoc &line)
{
    if (left == NULL && right == NULL)
        return NULL;

    TIntermAggregate *aggNode = NULL;
    if (left)
        aggNode = left->getAsAggregate();
    if (!aggNode || aggNode->getOp() != EOpNull)
    {
        aggNode = new TIntermAggregate;
        if (left)
            aggNode->getSequence()->push_back(left);
    }

    if (right)
        aggNode->getSequence()->push_back(right);

    aggNode->setLine(line);

    return aggNode;
}

//
// Turn an existing node into an aggregate.
//
// Returns an aggregate, unless NULL was passed in for the existing node.
//
TIntermAggregate *TIntermediate::makeAggregate(
    TIntermNode *node, const TSourceLoc &line)
{
    if (node == NULL)
        return NULL;

    TIntermAggregate *aggNode = new TIntermAggregate;
    aggNode->getSequence()->push_back(node);

    aggNode->setLine(line);

    return aggNode;
}

// If the input node is nullptr, return nullptr.
// If the input node is a sequence (block) node, return it.
// If the input node is not a sequence node, put it inside a sequence node and return that.
TIntermAggregate *TIntermediate::ensureSequence(TIntermNode *node)
{
    if (node == nullptr)
        return nullptr;
    TIntermAggregate *aggNode = node->getAsAggregate();
    if (aggNode != nullptr && aggNode->getOp() == EOpSequence)
        return aggNode;

    aggNode = makeAggregate(node, node->getLine());
    aggNode->setOp(EOpSequence);
    return aggNode;
}

//
// For "if" test nodes.  There are three children; a condition,
// a true path, and a false path.  The two paths are in the
// nodePair.
//
// Returns the selection node created.
//
TIntermNode *TIntermediate::addSelection(
    TIntermTyped *cond, TIntermNodePair nodePair, const TSourceLoc &line)
{
    //
    // For compile time constant selections, prune the code and
    // test now.
    //

    if (cond->getAsTyped() && cond->getAsTyped()->getAsConstantUnion())
    {
        if (cond->getAsConstantUnion()->getBConst(0) == true)
        {
            return nodePair.node1 ? setAggregateOperator(
                nodePair.node1, EOpSequence, nodePair.node1->getLine()) : NULL;
        }
        else
        {
            return nodePair.node2 ? setAggregateOperator(
                nodePair.node2, EOpSequence, nodePair.node2->getLine()) : NULL;
        }
    }

    TIntermSelection *node = new TIntermSelection(
        cond, ensureSequence(nodePair.node1), ensureSequence(nodePair.node2));
    node->setLine(line);

    return node;
}

TIntermTyped *TIntermediate::addComma(
    TIntermTyped *left, TIntermTyped *right, const TSourceLoc &line)
{
    if (left->getType().getQualifier() == EvqConst &&
        right->getType().getQualifier() == EvqConst)
    {
        return right;
    }
    else
    {
        TIntermTyped *commaAggregate = growAggregate(left, right, line);
        commaAggregate->getAsAggregate()->setOp(EOpComma);
        commaAggregate->setType(right->getType());
        commaAggregate->getTypePointer()->setQualifier(EvqTemporary);
        return commaAggregate;
    }
}

//
// For "?:" test nodes.  There are three children; a condition,
// a true path, and a false path.  The two paths are specified
// as separate parameters.
//
// Returns the selection node created, or one of trueBlock and falseBlock if the expression could be folded.
//
TIntermTyped *TIntermediate::addSelection(TIntermTyped *cond, TIntermTyped *trueBlock, TIntermTyped *falseBlock,
                                          const TSourceLoc &line)
{
    // Right now it's safe to fold ternary operators only when all operands
    // are constant. If only the condition is constant, it's theoretically
    // possible to fold the ternary operator, but that requires making sure
    // that the node returned from here won't be treated as a constant
    // expression in case the node that gets eliminated was not a constant
    // expression.
    if (cond->getAsConstantUnion() &&
        trueBlock->getAsConstantUnion() &&
        falseBlock->getAsConstantUnion())
    {
        if (cond->getAsConstantUnion()->getBConst(0))
            return trueBlock;
        else
            return falseBlock;
    }

    //
    // Make a selection node.
    //
    TIntermSelection *node = new TIntermSelection(cond, trueBlock, falseBlock, trueBlock->getType());
    node->getTypePointer()->setQualifier(EvqTemporary);
    node->setLine(line);

    return node;
}

TIntermSwitch *TIntermediate::addSwitch(
    TIntermTyped *init, TIntermAggregate *statementList, const TSourceLoc &line)
{
    TIntermSwitch *node = new TIntermSwitch(init, statementList);
    node->setLine(line);

    return node;
}

TIntermCase *TIntermediate::addCase(
    TIntermTyped *condition, const TSourceLoc &line)
{
    TIntermCase *node = new TIntermCase(condition);
    node->setLine(line);

    return node;
}

//
// Constant terminal nodes.  Has a union that contains bool, float or int constants
//
// Returns the constant union node created.
//

TIntermConstantUnion *TIntermediate::addConstantUnion(
    TConstantUnion *constantUnion, const TType &type, const TSourceLoc &line)
{
    TIntermConstantUnion *node = new TIntermConstantUnion(constantUnion, type);
    node->setLine(line);

    return node;
}

TIntermTyped *TIntermediate::addSwizzle(
    TVectorFields &fields, const TSourceLoc &line)
{

    TIntermAggregate *node = new TIntermAggregate(EOpSequence);

    node->setLine(line);
    TIntermConstantUnion *constIntNode;
    TIntermSequence *sequenceVector = node->getSequence();
    TConstantUnion *unionArray;

    for (int i = 0; i < fields.num; i++)
    {
        unionArray = new TConstantUnion[1];
        unionArray->setIConst(fields.offsets[i]);
        constIntNode = addConstantUnion(
            unionArray, TType(EbtInt, EbpUndefined, EvqConst), line);
        sequenceVector->push_back(constIntNode);
    }

    return node;
}

//
// Create loop nodes.
//
TIntermNode *TIntermediate::addLoop(
    TLoopType type, TIntermNode *init, TIntermTyped *cond, TIntermTyped *expr,
    TIntermNode *body, const TSourceLoc &line)
{
    TIntermNode *node = new TIntermLoop(type, init, cond, expr, ensureSequence(body));
    node->setLine(line);

    return node;
}

//
// Add branches.
//
TIntermBranch* TIntermediate::addBranch(
    TOperator branchOp, const TSourceLoc &line)
{
    return addBranch(branchOp, 0, line);
}

TIntermBranch* TIntermediate::addBranch(
    TOperator branchOp, TIntermTyped *expression, const TSourceLoc &line)
{
    TIntermBranch *node = new TIntermBranch(branchOp, expression);
    node->setLine(line);

    return node;
}

//
// This is to be executed once the final root is put on top by the parsing
// process.
//
TIntermAggregate *TIntermediate::postProcess(TIntermNode *root)
{
    if (root == nullptr)
        return nullptr;

    //
    // Finish off the top level sequence, if any
    //
    TIntermAggregate *aggRoot = root->getAsAggregate();
    if (aggRoot != nullptr && aggRoot->getOp() == EOpNull)
    {
        aggRoot->setOp(EOpSequence);
    }
    else if (aggRoot == nullptr || aggRoot->getOp() != EOpSequence)
    {
        aggRoot = new TIntermAggregate(EOpSequence);
        aggRoot->setLine(root->getLine());
        aggRoot->getSequence()->push_back(root);
    }

    return aggRoot;
}

TIntermTyped *TIntermediate::foldAggregateBuiltIn(TIntermAggregate *aggregate)
{
    switch (aggregate->getOp())
    {
      case EOpAtan:
      case EOpPow:
      case EOpMod:
      case EOpMin:
      case EOpMax:
      case EOpClamp:
      case EOpMix:
      case EOpStep:
      case EOpSmoothStep:
      case EOpMul:
      case EOpOuterProduct:
      case EOpLessThan:
      case EOpLessThanEqual:
      case EOpGreaterThan:
      case EOpGreaterThanEqual:
      case EOpVectorEqual:
      case EOpVectorNotEqual:
      case EOpDistance:
      case EOpDot:
      case EOpCross:
      case EOpFaceForward:
      case EOpReflect:
      case EOpRefract:
        return aggregate->fold(mInfoSink);
      default:
        // Constant folding not supported for the built-in.
        return nullptr;
    }

    return nullptr;
}
