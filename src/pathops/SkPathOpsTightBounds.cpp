/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkOpEdgeBuilder.h"
#include "SkPathOpsCommon.h"

bool TightBounds(const SkPath& path, SkRect* result) {
    SkChunkAlloc allocator(4096);  // FIXME: constant-ize, tune
    SkOpContour contour;
    SkOpContourHead* contourList = static_cast<SkOpContourHead*>(&contour);
    SkOpGlobalState globalState(NULL, contourList  SkDEBUGPARAMS(NULL));
    // turn path into list of segments
    SkOpEdgeBuilder builder(path, &contour, &allocator, &globalState);
    if (!builder.finish(&allocator)) {
        return false;
    }
    if (!SortContourList(&contourList, false, false)) {
        result->setEmpty();
        return true;
    }
    SkOpContour* current = contourList;
    SkPathOpsBounds bounds = current->bounds();
    while ((current = current->next())) {
        bounds.add(current->bounds());
    }
    *result = bounds;
    return true;
}
