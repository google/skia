//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_VALIDATEOUTPUTS_H_
#define COMPILER_TRANSLATOR_VALIDATEOUTPUTS_H_

#include "compiler/translator/ExtensionBehavior.h"
#include "compiler/translator/IntermNode.h"

#include <set>

class TInfoSinkBase;

class ValidateOutputs : public TIntermTraverser
{
  public:
    ValidateOutputs(const TExtensionBehavior &extBehavior, int maxDrawBuffers);

    int validateAndCountErrors(TInfoSinkBase &sink) const;

    void visitSymbol(TIntermSymbol *) override;

  private:
    int mMaxDrawBuffers;
    bool mAllowUnspecifiedOutputLocationResolution;

    typedef std::vector<TIntermSymbol *> OutputVector;
    OutputVector mOutputs;
    OutputVector mUnspecifiedLocationOutputs;
    std::set<TString> mVisitedSymbols;
};

#endif // COMPILER_TRANSLATOR_VALIDATEOUTPUTS_H_
