//
// Copyright (c) 2002-2010 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SearchSymbol is an AST traverser to detect the use of a given symbol name
//

#include "compiler/translator/SearchSymbol.h"

#include "compiler/translator/InfoSink.h"

namespace sh
{
SearchSymbol::SearchSymbol(const TString &symbol)
    : TIntermTraverser(true, false, false),
      mSymbol(symbol)
{
    match = false;
}

void SearchSymbol::traverse(TIntermNode *node)
{
    node->traverse(this);
}

void SearchSymbol::visitSymbol(TIntermSymbol *symbolNode)
{
    if (symbolNode->getSymbol() == mSymbol)
    {
        match = true;
    }
}

bool SearchSymbol::foundMatch() const
{
    return match;
}
}
