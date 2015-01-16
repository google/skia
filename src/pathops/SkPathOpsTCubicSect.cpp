/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPathOpsTSect.h"

int SkIntersections::intersectB(const SkDCubic& cubic1, const SkDCubic& cubic2) {
    SkTSect<SkDCubic> sect1(cubic1 PATH_OPS_DEBUG_PARAMS(1));
    SkTSect<SkDCubic> sect2(cubic2 PATH_OPS_DEBUG_PARAMS(2));
    SkTSect<SkDCubic>::BinarySearch(&sect1, &sect2, this);
    return used();
}
