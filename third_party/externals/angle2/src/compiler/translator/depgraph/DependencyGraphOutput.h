//
// Copyright (c) 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_DEPGRAPH_DEPENDENCYGRAPHOUTPUT_H_
#define COMPILER_TRANSLATOR_DEPGRAPH_DEPENDENCYGRAPHOUTPUT_H_

#include "compiler/translator/depgraph/DependencyGraph.h"
#include "compiler/translator/InfoSink.h"

class TDependencyGraphOutput : public TDependencyGraphTraverser
{
  public:
    TDependencyGraphOutput(TInfoSinkBase& sink) : mSink(sink) {}
    void visitSymbol(TGraphSymbol* symbol) override;
    void visitArgument(TGraphArgument* parameter) override;
    void visitFunctionCall(TGraphFunctionCall* functionCall) override;
    void visitSelection(TGraphSelection* selection) override;
    void visitLoop(TGraphLoop* loop) override;
    void visitLogicalOp(TGraphLogicalOp* logicalOp) override;

    void outputAllSpanningTrees(TDependencyGraph& graph);
  private:
    void outputIndentation();

    TInfoSinkBase& mSink;
};

#endif  // COMPILER_TRANSLATOR_DEPGRAPH_DEPENDENCYGRAPHOUTPUT_H_
