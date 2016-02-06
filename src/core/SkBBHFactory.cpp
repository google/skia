/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBBHFactory.h"
#include "SkRect.h"
#include "SkRTree.h"
#include "SkScalar.h"

SkBBoxHierarchy* SkRTreeFactory::operator()(const SkRect& bounds) const {
    SkScalar aspectRatio = bounds.width() / bounds.height();
    return new SkRTree(aspectRatio);
}

// TODO(thakis@chromium): remove once HTTP://llvm.org/26506 is fixed
SkRTreeFactory::SkRTreeFactory() {
}
