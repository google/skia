//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RemovePow is an AST traverser to convert pow(x, y) built-in calls where y is a
// constant to exp2(y * log2(x)). This works around an issue in NVIDIA 311 series
// OpenGL drivers.
//

#include "compiler/translator/RemovePow.h"

#include "compiler/translator/InfoSink.h"
#include "compiler/translator/IntermNode.h"

namespace
{

bool IsProblematicPow(TIntermTyped *node)
{
    TIntermAggregate *agg = node->getAsAggregate();
    if (agg != nullptr && agg->getOp() == EOpPow)
    {
        ASSERT(agg->getSequence()->size() == 2);
        return agg->getSequence()->at(1)->getAsConstantUnion() != nullptr;
    }
    return false;
}

// Traverser that converts all pow operations simultaneously.
class RemovePowTraverser : public TIntermTraverser
{
  public:
    RemovePowTraverser();

    bool visitAggregate(Visit visit, TIntermAggregate *node) override;

    void nextIteration() { mNeedAnotherIteration = false; }
    bool needAnotherIteration() const { return mNeedAnotherIteration; }

  protected:
    bool mNeedAnotherIteration;
};

RemovePowTraverser::RemovePowTraverser()
    : TIntermTraverser(true, false, false),
      mNeedAnotherIteration(false)
{
}

bool RemovePowTraverser::visitAggregate(Visit visit, TIntermAggregate *node)
{
    if (IsProblematicPow(node))
    {
        TInfoSink nullSink;

        TIntermTyped *x = node->getSequence()->at(0)->getAsTyped();
        TIntermTyped *y = node->getSequence()->at(1)->getAsTyped();

        TIntermUnary *log = new TIntermUnary(EOpLog2);
        log->setOperand(x);
        log->setLine(node->getLine());
        log->setType(x->getType());

        TIntermBinary *mul = new TIntermBinary(EOpMul);
        mul->setLeft(y);
        mul->setRight(log);
        mul->setLine(node->getLine());
        bool valid = mul->promote(nullSink);
        UNUSED_ASSERTION_VARIABLE(valid);
        ASSERT(valid);

        TIntermUnary *exp = new TIntermUnary(EOpExp2);
        exp->setOperand(mul);
        exp->setLine(node->getLine());
        exp->setType(node->getType());

        NodeUpdateEntry replacePow(getParentNode(), node, exp, false);
        mReplacements.push_back(replacePow);

        // If the x parameter also needs to be replaced, we need to do that in another traversal,
        // since it's parent node will change in a way that's not handled correctly by updateTree().
        if (IsProblematicPow(x))
        {
            mNeedAnotherIteration = true;
            return false;
        }
    }
    return true;
}

} // namespace

void RemovePow(TIntermNode *root)
{
    RemovePowTraverser traverser;
    // Iterate as necessary, and reset the traverser between iterations.
    do
    {
        traverser.nextIteration();
        root->traverse(&traverser);
        traverser.updateTree();
    }
    while (traverser.needAnotherIteration());
}
