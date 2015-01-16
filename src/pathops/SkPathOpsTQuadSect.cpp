/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPathOpsTSect.h"

int SkIntersections::intersectB(const SkDQuad& quad1, const SkDQuad& quad2) {
    SkTSect<SkDQuad> sect1(quad1 PATH_OPS_DEBUG_PARAMS(1));
    SkTSect<SkDQuad> sect2(quad2 PATH_OPS_DEBUG_PARAMS(2));
    SkTSect<SkDQuad>::BinarySearch(&sect1, &sect2, this);
    return used();
}
