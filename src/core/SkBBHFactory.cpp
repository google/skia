/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBBHFactory.h"
#include "src/core/SkRTree.h"

sk_sp<SkBBoxHierarchy> SkRTreeFactory::operator()() const {
    return sk_make_sp<SkRTree>();
}

void SkBBoxHierarchy::insert(const SkRect rects[], const Metadata[], int N) {
    // Ignore Metadata.
    this->insert(rects, N);
}
