//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// The SeparateArrayInitialization function splits each array initialization into a declaration and an assignment.
// Example:
//     type[n] a = initializer;
// will effectively become
//     type[n] a;
//     a = initializer;

#include "compiler/translator/SeparateArrayInitialization.h"

#include "compiler/translator/IntermNode.h"

namespace
{

class SeparateArrayInitTraverser : private TIntermTraverser
{
  public:
    static void apply(TIntermNode *root);
  private:
    SeparateArrayInitTraverser();
    bool visitAggregate(Visit, TIntermAggregate *node) override;
};

void SeparateArrayInitTraverser::apply(TIntermNode *root)
{
    SeparateArrayInitTraverser separateInit;
    root->traverse(&separateInit);
    separateInit.updateTree();
}

SeparateArrayInitTraverser::SeparateArrayInitTraverser()
    : TIntermTraverser(true, false, false)
{
}

bool SeparateArrayInitTraverser::visitAggregate(Visit, TIntermAggregate *node)
{
    if (node->getOp() == EOpDeclaration)
    {
        TIntermSequence *sequence = node->getSequence();
        TIntermBinary *initNode = sequence->back()->getAsBinaryNode();
        if (initNode != nullptr && initNode->getOp() == EOpInitialize)
        {
            TIntermTyped *initializer = initNode->getRight();
            if (initializer->isArray())
            {
                // We rely on that array declarations have been isolated to single declarations.
                ASSERT(sequence->size() == 1);
                TIntermTyped *symbol = initNode->getLeft();
                TIntermAggregate *parentAgg = getParentNode()->getAsAggregate();
                ASSERT(parentAgg != nullptr);

                TIntermSequence replacements;

                TIntermAggregate *replacementDeclaration = new TIntermAggregate;
                replacementDeclaration->setOp(EOpDeclaration);
                replacementDeclaration->getSequence()->push_back(symbol);
                replacementDeclaration->setLine(symbol->getLine());
                replacements.push_back(replacementDeclaration);

                TIntermBinary *replacementAssignment = new TIntermBinary(EOpAssign);
                replacementAssignment->setLeft(symbol);
                replacementAssignment->setRight(initializer);
                replacementAssignment->setType(initializer->getType());
                replacementAssignment->setLine(symbol->getLine());
                replacements.push_back(replacementAssignment);

                mMultiReplacements.push_back(NodeReplaceWithMultipleEntry(parentAgg, node, replacements));
            }
        }
        return false;
    }
    return true;
}

} // namespace

void SeparateArrayInitialization(TIntermNode *root)
{
    SeparateArrayInitTraverser::apply(root);
}
