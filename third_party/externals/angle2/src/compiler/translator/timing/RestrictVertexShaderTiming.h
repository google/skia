//
// Copyright (c) 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_TIMING_RESTRICTVERTEXSHADERTIMING_H_
#define COMPILER_TRANSLATOR_TIMING_RESTRICTVERTEXSHADERTIMING_H_

#include "compiler/translator/IntermNode.h"
#include "compiler/translator/InfoSink.h"

class TInfoSinkBase;

class RestrictVertexShaderTiming : public TIntermTraverser {
public:
    RestrictVertexShaderTiming(TInfoSinkBase& sink)
        : TIntermTraverser(true, false, false)
        , mSink(sink)
        , mNumErrors(0) {}

    void enforceRestrictions(TIntermNode* root) { root->traverse(this); }
    int numErrors() { return mNumErrors; }

    void visitSymbol(TIntermSymbol *) override;

private:
    TInfoSinkBase& mSink;
    int mNumErrors;
};

#endif  // COMPILER_TRANSLATOR_TIMING_RESTRICTVERTEXSHADERTIMING_H_
