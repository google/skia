//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// UnfoldShortCircuitAST is an AST traverser to replace short-circuiting
// operations with ternary operations.
//

#ifndef COMPILER_TRANSLATOR_UNFOLDSHORTCIRCUITAST_H_
#define COMPILER_TRANSLATOR_UNFOLDSHORTCIRCUITAST_H_

#include "common/angleutils.h"
#include "compiler/translator/IntermNode.h"

// This traverser identifies all the short circuit binary  nodes that need to
// be replaced, and creates the corresponding replacement nodes. However,
// the actual replacements happen after the traverse through updateTree().

class UnfoldShortCircuitAST : public TIntermTraverser
{
  public:
    UnfoldShortCircuitAST()
        : TIntermTraverser(true, false, false)
    {
    }

    bool visitBinary(Visit visit, TIntermBinary *) override;
};

#endif  // COMPILER_TRANSLATOR_UNFOLDSHORTCIRCUITAST_H_
