//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// The PruneEmptyDeclarations function prunes unnecessary empty declarations and declarators from the AST.

#include "compiler/translator/PruneEmptyDeclarations.h"

#include "compiler/translator/IntermNode.h"

namespace
{

class PruneEmptyDeclarationsTraverser : private TIntermTraverser
{
  public:
    static void apply(TIntermNode *root);
  private:
    PruneEmptyDeclarationsTraverser();
    bool visitAggregate(Visit, TIntermAggregate *node) override;
};

void PruneEmptyDeclarationsTraverser::apply(TIntermNode *root)
{
    PruneEmptyDeclarationsTraverser prune;
    root->traverse(&prune);
    prune.updateTree();
}

PruneEmptyDeclarationsTraverser::PruneEmptyDeclarationsTraverser()
    : TIntermTraverser(true, false, false)
{
}

bool PruneEmptyDeclarationsTraverser::visitAggregate(Visit, TIntermAggregate *node)
{
    if (node->getOp() == EOpDeclaration)
    {
        TIntermSequence *sequence = node->getSequence();
        if (sequence->size() >= 1)
        {
            TIntermSymbol *sym = sequence->front()->getAsSymbolNode();
            // Prune declarations without a variable name, unless it's an interface block declaration.
            if (sym != nullptr && sym->getSymbol() == "" && !sym->isInterfaceBlock())
            {
                if (sequence->size() > 1)
                {
                    // Generate a replacement that will remove the empty declarator in the beginning of a declarator
                    // list. Example of a declaration that will be changed:
                    // float, a;
                    // will be changed to
                    // float a;
                    // This applies also to struct declarations.
                    TIntermSequence emptyReplacement;
                    mMultiReplacements.push_back(NodeReplaceWithMultipleEntry(node, sym, emptyReplacement));
                }
                else if (sym->getBasicType() != EbtStruct)
                {
                    // Single struct declarations may just declare the struct type and no variables, so they should
                    // not be pruned. All other single empty declarations can be pruned entirely. Example of an empty
                    // declaration that will be pruned:
                    // float;
                    TIntermSequence emptyReplacement;
                    TIntermAggregate *parentAgg = getParentNode()->getAsAggregate();
                    ASSERT(parentAgg != nullptr);
                    mMultiReplacements.push_back(NodeReplaceWithMultipleEntry(parentAgg, node, emptyReplacement));
                }
            }
        }
        return false;
    }
    return true;
}

} // namespace

void PruneEmptyDeclarations(TIntermNode *root)
{
    PruneEmptyDeclarationsTraverser::apply(root);
}
