//
// Copyright (c) 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_RENAMEFUNCTION_H_
#define COMPILER_TRANSLATOR_RENAMEFUNCTION_H_

#include "compiler/translator/IntermNode.h"

//
// Renames a function, including its declaration and any calls to it.
//
class RenameFunction : public TIntermTraverser
{
public:
    RenameFunction(const TString& oldFunctionName, const TString& newFunctionName)
    : TIntermTraverser(true, false, false)
    , mOldFunctionName(oldFunctionName)
    , mNewFunctionName(newFunctionName) {}

    bool visitAggregate(Visit visit, TIntermAggregate *node) override
    {
        TOperator op = node->getOp();
        if ((op == EOpFunction || op == EOpFunctionCall) && node->getName() == mOldFunctionName)
            node->setName(mNewFunctionName);
        return true;
    }

private:
    const TString mOldFunctionName;
    const TString mNewFunctionName;
};

#endif  // COMPILER_TRANSLATOR_RENAMEFUNCTION_H_
