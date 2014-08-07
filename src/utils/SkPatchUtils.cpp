/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPatchUtils.h"

// size in pixels of each partition per axis, adjust this knob
static const int kPartitionSize = 15;

/**
 * Calculate the approximate arc length given a bezier curve's control points.
 */
static SkScalar approx_arc_length(SkPoint* points, int count) {
    if (count < 2) {
        return 0;
    }
    SkScalar arcLength = 0;
    for (int i = 0; i < count - 1; i++) {
        arcLength += SkPoint::Distance(points[i], points[i + 1]);
    }
    return arcLength;
}

SkISize SkPatchUtils::GetLevelOfDetail(const SkPatch& patch, const SkMatrix* matrix) {
    
    SkPoint mapPts[12];
    matrix->mapPoints(mapPts, patch.getControlPoints(), 12);
    
    // Approximate length of each cubic.
    SkPoint pts[4];
    patch.getTopPoints(pts);
    matrix->mapPoints(pts, 4);
    SkScalar topLength = approx_arc_length(pts, 4);
    
    patch.getBottomPoints(pts);
    matrix->mapPoints(pts, 4);
    SkScalar bottomLength = approx_arc_length(pts, 4);
    
    patch.getLeftPoints(pts);
    matrix->mapPoints(pts, 4);
    SkScalar leftLength = approx_arc_length(pts, 4);
    
    patch.getRightPoints(pts);
    matrix->mapPoints(pts, 4);
    SkScalar rightLength = approx_arc_length(pts, 4);
    
    // Level of detail per axis, based on the larger side between top and bottom or left and right
    int lodX = static_cast<int>(SkMaxScalar(topLength, bottomLength) / kPartitionSize);
    int lodY = static_cast<int>(SkMaxScalar(leftLength, rightLength) / kPartitionSize);
    
    return SkISize::Make(SkMax32(4, lodX), SkMax32(4, lodY));
}
