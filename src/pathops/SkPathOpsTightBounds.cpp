/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkOpEdgeBuilder.h"
#include "SkPathOpsCommon.h"

bool TightBounds(const SkPath& path, SkRect* result) {
    SkOpContour contour;
    SkOpGlobalState globalState( NULL  PATH_OPS_DEBUG_PARAMS(&contour));
    // turn path into list of segments
    SkChunkAlloc allocator(4096);  // FIXME: constant-ize, tune
    SkOpEdgeBuilder builder(path, &contour, &allocator, &globalState);
    if (!builder.finish(&allocator)) {
        return false;
    }
    SkTDArray<SkOpContour* > contourList;
    MakeContourList(&contour, contourList, false, false);
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
