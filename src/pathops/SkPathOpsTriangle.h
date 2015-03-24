/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathOpsTriangle_DEFINED
#define SkPathOpsTriangle_DEFINED

#include "SkPathOpsPoint.h"

struct SkDTriangle {
    SkDPoint fPts[3];

    bool contains(const SkDPoint& pt) const;

};

#endif
