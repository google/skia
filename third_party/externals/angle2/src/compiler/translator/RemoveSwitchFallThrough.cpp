//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/RemoveSwitchFallThrough.h"

TIntermAggregate *RemoveSwitchFallThrough::removeFallThrough(TIntermAggregate *statementList)
{
    RemoveSwitchFallThrough rm(statementList);
    ASSERT(statementList);
    statementList->traverse(&rm);
    bool lastStatementWasBreak = rm.mLastStatementWasBreak;
    rm.mLastStatementWasBreak = true;
    rm.handlePreviousCase();
    if (!lastStatementWasBreak)
    {
        TIntermBranch *finalBreak = new TIntermBranch(EOpBreak, nullptr);
        rm.mStatementListOut->getSequence()->push_back(finalBreak);
    }
    return rm.mStatementListOut;
}

RemoveSwitchFallThrough::RemoveSwitchFallThrough(TIntermAggregate *statementList)
    : TIntermTraverser(true, false, false),
      mStatementList(statementList),
      mLastStatementWasBreak(false),
      mPreviousCase(nullptr)
{
    mStatementListOut = new TIntermAggregate();
    mStatementListOut->setOp(EOpSequence);
}

void RemoveSwitchFallThrough::visitSymbol(TIntermSymbol *node)
{
    // Note that this assumes that switch statements which don't begin by a case statement
    // have already been weeded out in validation.
    mPreviousCase->getSequence()->push_back(node);
    mLastStatementWasBreak = false;
}

void RemoveSwitchFallThrough::visitConstantUnion(TIntermConstantUnion *node)
{
    // Conditions of case labels are not traversed, so this is some other constant
    // Could be just a statement like "0;"
    mPreviousCase->getSequence()->push_back(node);
    mLastStatementWasBreak = false;
}

bool RemoveSwitchFallThrough::visitBinary(Visit, TIntermBinary *node)
{
    mPreviousCase->getSequence()->push_back(node);
    mLastStatementWasBreak = false;
    return false;
}

bool RemoveSwitchFallThrough::visitUnary(Visit, TIntermUnary *node)
{
    mPreviousCase->getSequence()->push_back(node);
    mLastStatementWasBreak = false;
    return false;
}

bool RemoveSwitchFallThrough::visitSelection(Visit, TIntermSelection *node)
{
    mPreviousCase->getSequence()->push_back(node);
    mLastStatementWasBreak = false;
    return false;
}

bool RemoveSwitchFallThrough::visitSwitch(Visit, TIntermSwitch *node)
{
    mPreviousCase->getSequence()->push_back(node);
    mLastStatementWasBreak = false;
    // Don't go into nested switch statements
    return false;
}

void RemoveSwitchFallThrough::outputSequence(TIntermSequence *sequence, size_t startIndex)
{
    for (size_t i = startIndex; i < sequence->size(); ++i)
    {
        mStatementListOut->getSequence()->push_back(sequence->at(i));
    }
}

void RemoveSwitchFallThrough::handlePreviousCase()
{
    if (mPreviousCase)
        mCasesSharingBreak.push_back(mPreviousCase);
    if (mLastStatementWasBreak)
    {
        bool labelsWithNoStatements = true;
        for (size_t i = 0; i < mCasesSharingBreak.size(); ++i)
        {
            if (mCasesSharingBreak.at(i)->getSequence()->size() > 1)
            {
                labelsWithNoStatements = false;
            }
            if (labelsWithNoStatements)
            {
                // Fall-through is allowed in case the label has no statements.
                outputSequence(mCasesSharingBreak.at(i)->getSequence(), 0);
            }
            else
            {
                // Include all the statements that this case can fall through under the same label.
                for (size_t j = i; j < mCasesSharingBreak.size(); ++j)
                {
                    size_t startIndex = j > i ? 1 : 0; // Add the label only from the first sequence.
                    outputSequence(mCasesSharingBreak.at(j)->getSequence(), startIndex);

                }
            }
        }
        mCasesSharingBreak.clear();
    }
    mLastStatementWasBreak = false;
    mPreviousCase = nullptr;
}

bool RemoveSwitchFallThrough::visitCase(Visit, TIntermCase *node)
{
    handlePreviousCase();
    mPreviousCase = new TIntermAggregate();
    mPreviousCase->setOp(EOpSequence);
    mPreviousCase->getSequence()->push_back(node);
    // Don't traverse the condition of the case statement
    return false;
}

bool RemoveSwitchFallThrough::visitAggregate(Visit, TIntermAggregate *node)
{
    if (node != mStatementList)
    {
        mPreviousCase->getSequence()->push_back(node);
        mLastStatementWasBreak = false;
        return false;
    }
    return true;
}

bool RemoveSwitchFallThrough::visitLoop(Visit, TIntermLoop *node)
{
    mPreviousCase->getSequence()->push_back(node);
    mLastStatementWasBreak = false;
    return false;
}

bool RemoveSwitchFallThrough::visitBranch(Visit, TIntermBranch *node)
{
    mPreviousCase->getSequence()->push_back(node);
    // TODO: Verify that accepting return or continue statements here doesn't cause problems.
    mLastStatementWasBreak = true;
    return false;
}
