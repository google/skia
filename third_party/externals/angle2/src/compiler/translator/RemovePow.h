//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RemovePow is an AST traverser to convert pow(x, y) built-in calls where y is a
// constant to exp2(y * log2(x)). This works around an issue in NVIDIA 311 series
// OpenGL drivers.
//

#ifndef COMPILER_TRANSLATOR_REMOVEPOW_H_
#define COMPILER_TRANSLATOR_REMOVEPOW_H_

class TIntermNode;

void RemovePow(TIntermNode *root);

#endif   // COMPILER_TRANSLATOR_REMOVEPOW_H_
