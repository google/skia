/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkOpEdgeBuilder.h"
#include "SkPathOpsCommon.h"

bool TightBounds(const SkPath& path, SkRect* result) {
    // turn path into list of segments
    SkTArray<SkOpContour> contours;
    SkOpEdgeBuilder builder(path, contours);
    if (!builder.finish()) {
        return false;
    }
    SkTArray<SkOpContour*, true> contourList;
    MakeContourList(contours, contourList, false, false);
    SkOpContour** currentPtr = contourList.begin();
    result->setEmpty();
    if (!currentPtr) {
        return true;
    }
    SkOpContour** listEnd = contourList.end();
    SkOpContour* current = *currentPtr++;
    SkPathOpsBounds bounds = current->bounds();
    while (currentPtr != listEnd) {
        current = *currentPtr++;
        bounds.add(current->bounds());
    }
    *result = bounds;
    return true;
}
