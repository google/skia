//
// Copyright (c) 2011 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_FORLOOPUNROLL_H_
#define COMPILER_TRANSLATOR_FORLOOPUNROLL_H_

#include "compiler/translator/LoopInfo.h"

// This class detects for-loops that needs to be unrolled.
// Currently we support two unroll conditions:
//   1) kForLoopWithIntegerIndex: unroll if the index type is integer.
//   2) kForLoopWithSamplerArrayIndex: unroll where a sampler array index
//      is also the loop integer index, and reject and fail a compile
//      where a sampler array index is also the loop float index.
class ForLoopUnrollMarker : public TIntermTraverser
{
  public:
    enum UnrollCondition
    {
        kIntegerIndex,
        kSamplerArrayIndex
    };

    ForLoopUnrollMarker(UnrollCondition condition)
        : TIntermTraverser(true, false, false),
          mUnrollCondition(condition),
          mSamplerArrayIndexIsFloatLoopIndex(false),
          mVisitSamplerArrayIndexNodeInsideLoop(false)
    {
    }

    bool visitBinary(Visit, TIntermBinary *node) override;
    bool visitLoop(Visit, TIntermLoop *node) override;
    void visitSymbol(TIntermSymbol *node) override;

    bool samplerArrayIndexIsFloatLoopIndex() const
    {
        return mSamplerArrayIndexIsFloatLoopIndex;
    }

  private:
    UnrollCondition mUnrollCondition;
    TLoopStack mLoopStack;
    bool mSamplerArrayIndexIsFloatLoopIndex;
    bool mVisitSamplerArrayIndexNodeInsideLoop;
};

#endif // COMPILER_TRANSLATOR_FORLOOPUNROLL_H_
