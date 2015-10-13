//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// The SeparateDeclarations function processes declarations, so that in the end each declaration
// contains only one declarator.
// This is useful as an intermediate step when initialization needs to be separated from declaration,
// or when things need to be unfolded out of the initializer.
// Example:
//     int a[1] = int[1](1), b[1] = int[1](2);
// gets transformed when run through this class into the AST equivalent of:
//     int a[1] = int[1](1);
//     int b[1] = int[1](2);

#include "compiler/translator/SeparateDeclarations.h"

#include "compiler/translator/IntermNode.h"

namespace
{

class SeparateDeclarationsTraverser : private TIntermTraverser
{
  public:
    static void apply(TIntermNode *root);
  private:
    SeparateDeclarationsTraverser();
    bool visitAggregate(Visit, TIntermAggregate *node) override;
};

void SeparateDeclarationsTraverser::apply(TIntermNode *root)
{
    SeparateDeclarationsTraverser separateDecl;
    root->traverse(&separateDecl);
    separateDecl.updateTree();
}

SeparateDeclarationsTraverser::SeparateDeclarationsTraverser()
    : TIntermTraverser(true, false, false)
{
}

bool SeparateDeclarationsTraverser::visitAggregate(Visit, TIntermAggregate *node)
{
    if (node->getOp() == EOpDeclaration)
    {
        TIntermSequence *sequence = node->getSequence();
        if (sequence->size() > 1)
        {
            TIntermAggregate *parentAgg = getParentNode()->getAsAggregate();
            ASSERT(parentAgg != nullptr);

            TIntermSequence replacementDeclarations;
            for (size_t ii = 0; ii < sequence->size(); ++ii)
            {
                TIntermAggregate *replacementDeclaration = new TIntermAggregate;

                replacementDeclaration->setOp(EOpDeclaration);
                replacementDeclaration->getSequence()->push_back(sequence->at(ii));
                replacementDeclaration->setLine(sequence->at(ii)->getLine());
                replacementDeclarations.push_back(replacementDeclaration);
            }

            mMultiReplacements.push_back(NodeReplaceWithMultipleEntry(parentAgg, node, replacementDeclarations));
        }
        return false;
    }
    return true;
}

} // namespace

void SeparateDeclarations(TIntermNode *root)
{
    SeparateDeclarationsTraverser::apply(root);
}
