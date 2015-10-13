//
// Copyright (c) 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_TIMING_RESTRICTFRAGMENTSHADERTIMING_H_
#define COMPILER_TRANSLATOR_TIMING_RESTRICTFRAGMENTSHADERTIMING_H_

#include "compiler/translator/IntermNode.h"
#include "compiler/translator/depgraph/DependencyGraph.h"

class TInfoSinkBase;

class RestrictFragmentShaderTiming : TDependencyGraphTraverser
{
  public:
    RestrictFragmentShaderTiming(TInfoSinkBase &sink);
    void enforceRestrictions(const TDependencyGraph &graph);
    int numErrors() const { return mNumErrors; }

    void visitArgument(TGraphArgument *parameter) override;
    void visitSelection(TGraphSelection *selection) override;
    void visitLoop(TGraphLoop *loop) override;
    void visitLogicalOp(TGraphLogicalOp *logicalOp) override;

  private:
    void beginError(const TIntermNode *node);
    void validateUserDefinedFunctionCallUsage(const TDependencyGraph &graph);
    bool isSamplingOp(const TIntermAggregate *intermFunctionCall) const;

    TInfoSinkBase &mSink;
    int mNumErrors;

    typedef std::set<TString> StringSet;
    StringSet mSamplingOps;
};

#endif  // COMPILER_TRANSLATOR_TIMING_RESTRICTFRAGMENTSHADERTIMING_H_
