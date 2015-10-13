//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_INTERMEDIATE_H_
#define COMPILER_TRANSLATOR_INTERMEDIATE_H_

#include "compiler/translator/IntermNode.h"

struct TVectorFields
{
    int offsets[4];
    int num;
};

//
// Set of helper functions to help parse and build the tree.
//
class TInfoSink;
class TIntermediate
{
  public:
    POOL_ALLOCATOR_NEW_DELETE();
    TIntermediate(TInfoSink &i)
        : mInfoSink(i) { }

    TIntermSymbol *addSymbol(
        int id, const TString &, const TType &, const TSourceLoc &);
    TIntermTyped *addBinaryMath(
        TOperator op, TIntermTyped *left, TIntermTyped *right, const TSourceLoc &);
    TIntermTyped *addAssign(
        TOperator op, TIntermTyped *left, TIntermTyped *right, const TSourceLoc &);
    TIntermTyped *addIndex(
        TOperator op, TIntermTyped *base, TIntermTyped *index, const TSourceLoc &);
    TIntermTyped *addUnaryMath(
        TOperator op, TIntermTyped *child, const TSourceLoc &line, const TType *funcReturnType);
    TIntermAggregate *growAggregate(
        TIntermNode *left, TIntermNode *right, const TSourceLoc &);
    TIntermAggregate *makeAggregate(TIntermNode *node, const TSourceLoc &);
    TIntermAggregate *ensureSequence(TIntermNode *node);
    TIntermAggregate *setAggregateOperator(TIntermNode *, TOperator, const TSourceLoc &);
    TIntermNode *addSelection(TIntermTyped *cond, TIntermNodePair code, const TSourceLoc &);
    TIntermTyped *addSelection(TIntermTyped *cond, TIntermTyped *trueBlock, TIntermTyped *falseBlock,
                               const TSourceLoc &line);
    TIntermSwitch *addSwitch(
        TIntermTyped *init, TIntermAggregate *statementList, const TSourceLoc &line);
    TIntermCase *addCase(
        TIntermTyped *condition, const TSourceLoc &line);
    TIntermTyped *addComma(
        TIntermTyped *left, TIntermTyped *right, const TSourceLoc &);
    TIntermConstantUnion *addConstantUnion(
        TConstantUnion *constantUnion, const TType &type, const TSourceLoc &line);
    // TODO(zmo): Get rid of default value.
    bool parseConstTree(const TSourceLoc &, TIntermNode *, TConstantUnion *,
                        TOperator, TType, bool singleConstantParam = false);
    TIntermNode *addLoop(TLoopType, TIntermNode *, TIntermTyped *, TIntermTyped *,
                         TIntermNode *, const TSourceLoc &);
    TIntermBranch *addBranch(TOperator, const TSourceLoc &);
    TIntermBranch *addBranch(TOperator, TIntermTyped *, const TSourceLoc &);
    TIntermTyped *addSwizzle(TVectorFields &, const TSourceLoc &);
    TIntermAggregate *postProcess(TIntermNode *root);

    static void outputTree(TIntermNode *, TInfoSinkBase &);

    TIntermTyped *foldAggregateBuiltIn(TIntermAggregate *aggregate);

  private:
    void operator=(TIntermediate &); // prevent assignments

    TInfoSink & mInfoSink;
};

#endif  // COMPILER_TRANSLATOR_INTERMEDIATE_H_
