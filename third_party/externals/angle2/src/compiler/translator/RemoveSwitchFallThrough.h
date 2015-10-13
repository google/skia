//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_REMOVESWITCHFALLTHROUGH_H_
#define COMPILER_TRANSLATOR_REMOVESWITCHFALLTHROUGH_H_

#include "compiler/translator/IntermNode.h"

class RemoveSwitchFallThrough : public TIntermTraverser
{
  public:
    // When given a statementList from a switch AST node, return an updated
    // statementList that has fall-through removed.
    static TIntermAggregate *removeFallThrough(TIntermAggregate *statementList);

  private:
    RemoveSwitchFallThrough(TIntermAggregate *statementList);

    void visitSymbol(TIntermSymbol *node) override;
    void visitConstantUnion(TIntermConstantUnion *node) override;
    bool visitBinary(Visit, TIntermBinary *node) override;
    bool visitUnary(Visit, TIntermUnary *node) override;
    bool visitSelection(Visit visit, TIntermSelection *node) override;
    bool visitSwitch(Visit, TIntermSwitch *node) override;
    bool visitCase(Visit, TIntermCase *node) override;
    bool visitAggregate(Visit, TIntermAggregate *node) override;
    bool visitLoop(Visit, TIntermLoop *node) override;
    bool visitBranch(Visit, TIntermBranch *node) override;

    void outputSequence(TIntermSequence *sequence, size_t startIndex);
    void handlePreviousCase();

    TIntermAggregate *mStatementList;
    TIntermAggregate *mStatementListOut;
    bool mLastStatementWasBreak;
    TIntermAggregate *mPreviousCase;
    std::vector<TIntermAggregate *> mCasesSharingBreak;
};

#endif // COMPILER_TRANSLATOR_REMOVESWITCHFALLTHROUGH_H_
