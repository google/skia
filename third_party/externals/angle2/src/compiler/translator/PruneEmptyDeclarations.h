//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// The PruneEmptyDeclarations function prunes unnecessary empty declarations and declarators from the AST.

#ifndef COMPILER_TRANSLATOR_PRUNEEMPTYDECLARATIONS_H_
#define COMPILER_TRANSLATOR_PRUNEEMPTYDECLARATIONS_H_

class TIntermNode;

void PruneEmptyDeclarations(TIntermNode *root);

#endif  // COMPILER_TRANSLATOR_PRUNEEMPTYDECLARATIONS_H_
