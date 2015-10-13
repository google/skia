//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_VALIDATESWITCH_H_
#define COMPILER_TRANSLATOR_VALIDATESWITCH_H_

#include "compiler/translator/IntermNode.h"

class TParseContext;

class ValidateSwitch : public TIntermTraverser
{
  public:
    // Check for errors and output messages any remaining errors on the context.
    // Returns true if there are no errors.
    static bool validate(TBasicType switchType, TParseContext *context,
        TIntermAggregate *statementList, const TSourceLoc &loc);

    void visitSymbol(TIntermSymbol *) override;
    void visitConstantUnion(TIntermConstantUnion *) override;
    bool visitBinary(Visit, TIntermBinary *) override;
    bool visitUnary(Visit, TIntermUnary *) override;
    bool visitSelection(Visit visit, TIntermSelection *) override;
    bool visitSwitch(Visit, TIntermSwitch *) override;
    bool visitCase(Visit, TIntermCase *node) override;
    bool visitAggregate(Visit, TIntermAggregate *) override;
    bool visitLoop(Visit visit, TIntermLoop *) override;
    bool visitBranch(Visit, TIntermBranch *) override;

  private:
    ValidateSwitch(TBasicType switchType, TParseContext *context);

    bool validateInternal(const TSourceLoc &loc);

    TBasicType mSwitchType;
    TParseContext *mContext;
    bool mCaseTypeMismatch;
    bool mFirstCaseFound;
    bool mStatementBeforeCase;
    bool mLastStatementWasCase;
    int mControlFlowDepth;
    bool mCaseInsideControlFlow;
    int mDefaultCount;
    std::set<int> mCasesSigned;
    std::set<unsigned int> mCasesUnsigned;
    bool mDuplicateCases;
};

#endif // COMPILER_TRANSLATOR_VALIDATESWITCH_H_
