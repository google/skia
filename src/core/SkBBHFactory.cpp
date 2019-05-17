/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBBHFactory.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "src/core/SkRTree.h"

SkBBoxHierarchy* SkRTreeFactory::operator()() const {
    return new SkRTree;
}
