//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_INITIALIZEVARIABLES_H_
#define COMPILER_TRANSLATOR_INITIALIZEVARIABLES_H_

#include "compiler/translator/IntermNode.h"

class InitializeVariables : public TIntermTraverser
{
  public:
    struct InitVariableInfo
    {
        TString name;
        TType type;

        InitVariableInfo(const TString &_name, const TType &_type)
            : name(_name),
              type(_type)
        {
        }
    };
    typedef TVector<InitVariableInfo> InitVariableInfoList;

    InitializeVariables(const InitVariableInfoList &vars)
        : TIntermTraverser(true, false, false),
          mVariables(vars),
          mCodeInserted(false)
    {
    }

  protected:
    bool visitBinary(Visit, TIntermBinary *node) override { return false; }
    bool visitUnary(Visit, TIntermUnary *node) override { return false; }
    bool visitSelection(Visit, TIntermSelection *node) override { return false; }
    bool visitLoop(Visit, TIntermLoop *node) override { return false; }
    bool visitBranch(Visit, TIntermBranch *node) override { return false; }

    bool visitAggregate(Visit visit, TIntermAggregate *node) override;

  private:
    void insertInitCode(TIntermSequence *sequence);

    InitVariableInfoList mVariables;
    bool mCodeInserted;
};

#endif  // COMPILER_TRANSLATOR_INITIALIZEVARIABLES_H_
