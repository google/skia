/*
* Copyright 2019 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkClipStackUtils_DEFINED
#define SkClipStackUtils_DEFINED

#include "include/core/SkTypes.h"

class SkClipStack;
class SkPath;

// Return the resolved clipstack as a single path.
// Note: uses SkPathOps as part of its implementation.
//
void SkClipStack_AsPath(const SkClipStack& cs, SkPath* path);

#endif
