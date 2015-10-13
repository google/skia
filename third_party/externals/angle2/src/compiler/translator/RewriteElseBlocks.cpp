//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RewriteElseBlocks.cpp: Implementation for tree transform to change
//   all if-else blocks to if-if blocks.
//

#include "compiler/translator/RewriteElseBlocks.h"
#include "compiler/translator/NodeSearch.h"
#include "compiler/translator/SymbolTable.h"

namespace sh
{

namespace
{

class ElseBlockRewriter : public TIntermTraverser
{
  public:
    ElseBlockRewriter();

  protected:
    bool visitAggregate(Visit visit, TIntermAggregate *aggregate) override;

  private:
    const TType *mFunctionType;

    TIntermNode *rewriteSelection(TIntermSelection *selection);
};

TIntermUnary *MakeNewUnary(TOperator op, TIntermTyped *operand)
{
    TIntermUnary *unary = new TIntermUnary(op, operand->getType());
    unary->setOperand(operand);
    return unary;
}

ElseBlockRewriter::ElseBlockRewriter()
    : TIntermTraverser(true, false, true),
      mFunctionType(NULL)
{}

bool ElseBlockRewriter::visitAggregate(Visit visit, TIntermAggregate *node)
{
    switch (node->getOp())
    {
      case EOpSequence:
        if (visit == PostVisit)
        {
            for (size_t statementIndex = 0; statementIndex != node->getSequence()->size(); statementIndex++)
            {
                TIntermNode *statement = (*node->getSequence())[statementIndex];
                TIntermSelection *selection = statement->getAsSelectionNode();
                if (selection && selection->getFalseBlock() != nullptr)
                {
                    // Check for if / else if
                    TIntermSelection *elseIfBranch = selection->getFalseBlock()->getAsSelectionNode();
                    if (elseIfBranch)
                    {
                        selection->replaceChildNode(elseIfBranch, rewriteSelection(elseIfBranch));
                        delete elseIfBranch;
                    }

                    (*node->getSequence())[statementIndex] = rewriteSelection(selection);
                    delete selection;
                }
            }
        }
        break;

      case EOpFunction:
        // Store the current function context (see comment below)
        mFunctionType = ((visit == PreVisit) ? &node->getType() : NULL);
        break;

      default: break;
    }

    return true;
}

TIntermNode *ElseBlockRewriter::rewriteSelection(TIntermSelection *selection)
{
    ASSERT(selection != nullptr);

    nextTemporaryIndex();

    TIntermTyped *typedCondition = selection->getCondition()->getAsTyped();
    TIntermAggregate *storeCondition = createTempInitDeclaration(typedCondition);

    TIntermSelection *falseBlock = nullptr;

    TType boolType(EbtBool, EbpUndefined, EvqTemporary);

    if (selection->getFalseBlock())
    {
        TIntermAggregate *negatedElse = nullptr;
        // crbug.com/346463
        // D3D generates error messages claiming a function has no return value, when rewriting
        // an if-else clause that returns something non-void in a function. By appending dummy
        // returns (that are unreachable) we can silence this compile error.
        if (mFunctionType && mFunctionType->getBasicType() != EbtVoid)
        {
            TString typeString = mFunctionType->getStruct() ? mFunctionType->getStruct()->name() :
                mFunctionType->getBasicString();
            TString rawText = "return (" + typeString + ")0";
            TIntermRaw *returnNode = new TIntermRaw(*mFunctionType, rawText);
            negatedElse = new TIntermAggregate(EOpSequence);
            negatedElse->getSequence()->push_back(returnNode);
        }

        TIntermSymbol *conditionSymbolElse = createTempSymbol(boolType);
        TIntermUnary *negatedCondition = MakeNewUnary(EOpLogicalNot, conditionSymbolElse);
        falseBlock = new TIntermSelection(negatedCondition,
                                          selection->getFalseBlock(), negatedElse);
    }

    TIntermSymbol *conditionSymbolSel = createTempSymbol(boolType);
    TIntermSelection *newSelection = new TIntermSelection(conditionSymbolSel, selection->getTrueBlock(), falseBlock);

    TIntermAggregate *block = new TIntermAggregate(EOpSequence);
    block->getSequence()->push_back(storeCondition);
    block->getSequence()->push_back(newSelection);

    return block;
}

}

void RewriteElseBlocks(TIntermNode *node, unsigned int *temporaryIndex)
{
    ElseBlockRewriter rewriter;
    rewriter.useTemporaryIndex(temporaryIndex);
    node->traverse(&rewriter);
}

}
