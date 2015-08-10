/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrRectBatchFactory.h"

#include "GrRectBatch.h"
#include "GrStrokeRectBatch.h"

namespace GrRectBatchFactory {

GrBatch* CreateFillBW(GrColor color,
                      const SkMatrix& viewMatrix,
                      const SkRect& rect,
                      const SkRect* localRect,
                      const SkMatrix* localMatrix) {
    GrRectBatch::Geometry geometry;
    geometry.fColor = color;
    geometry.fViewMatrix = viewMatrix;
    geometry.fRect = rect;

    if (localRect) {
        geometry.fHasLocalRect = true;
        geometry.fLocalRect = *localRect;
    } else {
        geometry.fHasLocalRect = false;
    }

    if (localMatrix) {
        geometry.fHasLocalMatrix = true;
        geometry.fLocalMatrix = *localMatrix;
    } else {
        geometry.fHasLocalMatrix = false;
    }

    return GrRectBatch::Create(geometry);
}

GrBatch* CreateStrokeBW(GrColor color,
                        const SkMatrix& viewMatrix,
                        const SkRect& rect,
                        SkScalar strokeWidth,
                        bool snapToPixelCenters) {
    GrStrokeRectBatch::Geometry geometry;
    geometry.fColor = color;
    geometry.fViewMatrix = viewMatrix;
    geometry.fRect = rect;
    geometry.fStrokeWidth = strokeWidth;
    return GrStrokeRectBatch::Create(geometry, snapToPixelCenters);
}

};
