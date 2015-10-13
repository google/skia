//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// The ArrayReturnValueToOutParameter function changes return values of an array type to out parameters in
// function definitions, prototypes, and call sites.

#include "compiler/translator/ArrayReturnValueToOutParameter.h"

#include "compiler/translator/IntermNode.h"

namespace
{

void CopyAggregateChildren(TIntermAggregate *from, TIntermAggregate *to)
{
    const TIntermSequence *fromSequence = from->getSequence();
    for (size_t ii = 0; ii < fromSequence->size(); ++ii)
    {
        to->getSequence()->push_back(fromSequence->at(ii));
    }
}

TIntermSymbol *CreateReturnValueSymbol(const TType &type)
{
    TIntermSymbol *node = new TIntermSymbol(0, "angle_return", type);
    node->setInternal(true);
    return node;
}

TIntermSymbol *CreateReturnValueOutSymbol(const TType &type)
{
    TType outType(type);
    outType.setQualifier(EvqOut);
    return CreateReturnValueSymbol(outType);
}

TIntermAggregate *CreateReplacementCall(TIntermAggregate *originalCall, TIntermTyped *returnValueTarget)
{
    TIntermAggregate *replacementCall = new TIntermAggregate(EOpFunctionCall);
    replacementCall->setType(TType(EbtVoid));
    replacementCall->setUserDefined();
    replacementCall->setName(originalCall->getName());
    replacementCall->setFunctionId(originalCall->getFunctionId());
    replacementCall->setLine(originalCall->getLine());
    TIntermSequence *replacementParameters = replacementCall->getSequence();
    TIntermSequence *originalParameters = originalCall->getSequence();
    for (auto &param : *originalParameters)
    {
        replacementParameters->push_back(param);
    }
    replacementParameters->push_back(returnValueTarget);
    return replacementCall;
}

class ArrayReturnValueToOutParameterTraverser : private TIntermTraverser
{
  public:
    static void apply(TIntermNode *root, unsigned int *temporaryIndex);
  private:
    ArrayReturnValueToOutParameterTraverser();

    bool visitAggregate(Visit visit, TIntermAggregate *node) override;
    bool visitBranch(Visit visit, TIntermBranch *node) override;
    bool visitBinary(Visit visit, TIntermBinary *node) override;

    bool mInFunctionWithArrayReturnValue;
};

void ArrayReturnValueToOutParameterTraverser::apply(TIntermNode *root, unsigned int *temporaryIndex)
{
    ArrayReturnValueToOutParameterTraverser arrayReturnValueToOutParam;
    arrayReturnValueToOutParam.useTemporaryIndex(temporaryIndex);
    root->traverse(&arrayReturnValueToOutParam);
    arrayReturnValueToOutParam.updateTree();
}

ArrayReturnValueToOutParameterTraverser::ArrayReturnValueToOutParameterTraverser()
    : TIntermTraverser(true, false, true),
      mInFunctionWithArrayReturnValue(false)
{
}

bool ArrayReturnValueToOutParameterTraverser::visitAggregate(Visit visit, TIntermAggregate *node)
{
    if (visit == PreVisit)
    {
        if (node->isArray())
        {
            if (node->getOp() == EOpFunction)
            {
                // Replace the parameters child node of the function definition with another node
                // that has the out parameter added.
                // Also set the function to return void.

                TIntermAggregate *params = node->getSequence()->front()->getAsAggregate();
                ASSERT(params != nullptr && params->getOp() == EOpParameters);

                TIntermAggregate *replacementParams = new TIntermAggregate;
                replacementParams->setOp(EOpParameters);
                CopyAggregateChildren(params, replacementParams);
                replacementParams->getSequence()->push_back(CreateReturnValueOutSymbol(node->getType()));
                replacementParams->setLine(params->getLine());

                mReplacements.push_back(NodeUpdateEntry(node, params, replacementParams, false));

                node->setType(TType(EbtVoid));

                mInFunctionWithArrayReturnValue = true;
            }
            else if (node->getOp() == EOpPrototype)
            {
                // Replace the whole prototype node with another node that has the out parameter added.
                TIntermAggregate *replacement = new TIntermAggregate;
                replacement->setOp(EOpPrototype);
                CopyAggregateChildren(node, replacement);
                replacement->getSequence()->push_back(CreateReturnValueOutSymbol(node->getType()));
                replacement->setUserDefined();
                replacement->setName(node->getName());
                replacement->setFunctionId(node->getFunctionId());
                replacement->setLine(node->getLine());
                replacement->setType(TType(EbtVoid));

                mReplacements.push_back(NodeUpdateEntry(getParentNode(), node, replacement, false));
            }
            else if (node->getOp() == EOpFunctionCall)
            {
                // Handle call sites where the returned array is not assigned.
                // Examples where f() is a function returning an array:
                // 1. f();
                // 2. another_array == f();
                // 3. another_function(f());
                // 4. return f();
                // Cases 2 to 4 are already converted to simpler cases by SeparateExpressionsReturningArrays, so we
                // only need to worry about the case where a function call returning an array forms an expression by
                // itself.
                TIntermAggregate *parentAgg = getParentNode()->getAsAggregate();
                if (parentAgg != nullptr && parentAgg->getOp() == EOpSequence)
                {
                    nextTemporaryIndex();
                    TIntermSequence replacements;
                    replacements.push_back(createTempDeclaration(node->getType()));
                    TIntermSymbol *returnSymbol = createTempSymbol(node->getType());
                    replacements.push_back(CreateReplacementCall(node, returnSymbol));
                    mMultiReplacements.push_back(NodeReplaceWithMultipleEntry(parentAgg, node, replacements));
                }
                return false;
            }
        }
    }
    else if (visit == PostVisit)
    {
        if (node->getOp() == EOpFunction)
        {
            mInFunctionWithArrayReturnValue = false;
        }
    }
    return true;
}

bool ArrayReturnValueToOutParameterTraverser::visitBranch(Visit visit, TIntermBranch *node)
{
    if (mInFunctionWithArrayReturnValue && node->getFlowOp() == EOpReturn)
    {
        // Instead of returning a value, assign to the out parameter and then return.
        TIntermSequence replacements;

        TIntermBinary *replacementAssignment = new TIntermBinary(EOpAssign);
        TIntermTyped *expression = node->getExpression();
        ASSERT(expression != nullptr);
        replacementAssignment->setLeft(CreateReturnValueSymbol(expression->getType()));
        replacementAssignment->setRight(node->getExpression());
        replacementAssignment->setType(expression->getType());
        replacementAssignment->setLine(expression->getLine());
        replacements.push_back(replacementAssignment);

        TIntermBranch *replacementBranch = new TIntermBranch(EOpReturn, nullptr);
        replacementBranch->setLine(node->getLine());
        replacements.push_back(replacementBranch);

        mMultiReplacements.push_back(NodeReplaceWithMultipleEntry(getParentNode()->getAsAggregate(), node, replacements));
    }
    return false;
}

bool ArrayReturnValueToOutParameterTraverser::visitBinary(Visit visit, TIntermBinary *node)
{
    if (node->getOp() == EOpAssign && node->getLeft()->isArray())
    {
        TIntermAggregate *rightAgg = node->getRight()->getAsAggregate();
        if (rightAgg != nullptr && rightAgg->getOp() == EOpFunctionCall && rightAgg->isUserDefined())
        {
            TIntermAggregate *replacementCall = CreateReplacementCall(rightAgg, node->getLeft());
            mReplacements.push_back(NodeUpdateEntry(getParentNode(), node, replacementCall, false));
        }
    }
    return false;
}

} // namespace

void ArrayReturnValueToOutParameter(TIntermNode *root, unsigned int *temporaryIndex)
{
    ArrayReturnValueToOutParameterTraverser::apply(root, temporaryIndex);
}
