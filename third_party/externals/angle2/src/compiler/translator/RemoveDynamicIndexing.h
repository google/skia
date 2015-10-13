//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RemoveDynamicIndexing is an AST traverser to remove dynamic indexing of vectors and matrices,
// replacing them with calls to functions that choose which component to return or write.
//

#ifndef COMPILER_TRANSLATOR_REMOVEDYNAMICINDEXING_H_
#define COMPILER_TRANSLATOR_REMOVEDYNAMICINDEXING_H_

class TIntermNode;
class TSymbolTable;

void RemoveDynamicIndexing(TIntermNode *root,
                           unsigned int *temporaryIndex,
                           const TSymbolTable &symbolTable,
                           int shaderVersion);

#endif  // COMPILER_TRANSLATOR_REMOVEDYNAMICINDEXING_H_
