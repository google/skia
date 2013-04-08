/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkPathOpsPoint.h"

SkDVector operator-(const SkDPoint& a, const SkDPoint& b) {
    SkDVector v = {a.fX - b.fX, a.fY - b.fY};
    return v;
}

SkDPoint operator+(const SkDPoint& a, const SkDVector& b) {
    SkDPoint v = {a.fX + b.fX, a.fY + b.fY};
    return v;
}
