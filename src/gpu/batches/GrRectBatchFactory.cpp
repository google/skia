/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrRectBatchFactory.h"

#include "GrAAFillRectBatch.h"
#include "GrAAStrokeRectBatch.h"
#include "GrRectBatch.h"
#include "GrStrokeRectBatch.h"

#include "SkStrokeRec.h"

static GrBatch* create_stroke_aa_batch(GrColor color,
                                       const SkMatrix& viewMatrix,
                                       const SkRect& devOutside,
                                       const SkRect& devOutsideAssist,
                                       const SkRect& devInside,
                                       bool miterStroke) {
    GrAAStrokeRectBatch::Geometry geometry;
    geometry.fColor = color;
    geometry.fDevOutside = devOutside;
    geometry.fDevOutsideAssist = devOutsideAssist;
    geometry.fDevInside = devInside;
    geometry.fMiterStroke = miterStroke;

    return GrAAStrokeRectBatch::Create(geometry, viewMatrix);
}

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

GrBatch* CreateFillAA(GrColor color,
                      const SkMatrix& viewMatrix,
                      const SkRect& rect,
                      const SkRect& devRect) {
    GrAAFillRectBatch::Geometry geometry;
    geometry.fRect = rect;
    geometry.fViewMatrix = viewMatrix;
    geometry.fDevRect = devRect;
    geometry.fColor = color;

    return GrAAFillRectBatch::Create(geometry);
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

GrBatch* CreateStrokeAA(GrColor color,
                        const SkMatrix& viewMatrix,
                        const SkRect& rect,
                        const SkRect& devRect,
                        const SkStrokeRec& stroke) {
    SkVector devStrokeSize;
    SkScalar width = stroke.getWidth();
    if (width > 0) {
        devStrokeSize.set(width, width);
        viewMatrix.mapVectors(&devStrokeSize, 1);
        devStrokeSize.setAbs(devStrokeSize);
    } else {
        devStrokeSize.set(SK_Scalar1, SK_Scalar1);
    }

    const SkScalar dx = devStrokeSize.fX;
    const SkScalar dy = devStrokeSize.fY;
    const SkScalar rx = SkScalarMul(dx, SK_ScalarHalf);
    const SkScalar ry = SkScalarMul(dy, SK_ScalarHalf);

    SkScalar spare;
    {
        SkScalar w = devRect.width() - dx;
        SkScalar h = devRect.height() - dy;
        spare = SkTMin(w, h);
    }

    SkRect devOutside(devRect);
    devOutside.outset(rx, ry);

    bool miterStroke = true;
    // For hairlines, make bevel and round joins appear the same as mitered ones.
    // small miter limit means right angles show bevel...
    if ((width > 0) && (stroke.getJoin() != SkPaint::kMiter_Join ||
                        stroke.getMiter() < SK_ScalarSqrt2)) {
        miterStroke = false;
    }

    if (spare <= 0 && miterStroke) {
        return CreateFillAA(color, viewMatrix, devOutside, devOutside);
    }

    SkRect devInside(devRect);
    devInside.inset(rx, ry);

    SkRect devOutsideAssist(devRect);

    // For bevel-stroke, use 2 SkRect instances(devOutside and devOutsideAssist)
    // to draw the outer of the rect. Because there are 8 vertices on the outer
    // edge, while vertex number of inner edge is 4, the same as miter-stroke.
    if (!miterStroke) {
        devOutside.inset(0, ry);
        devOutsideAssist.outset(0, ry);
    }

    return create_stroke_aa_batch(color, viewMatrix, devOutside, devOutsideAssist, devInside,
                                  miterStroke);
}

GrBatch* CreateFillNestedRectsAA(GrColor color,
                                 const SkMatrix& viewMatrix,
                                 const SkRect rects[2]) {
    SkASSERT(viewMatrix.rectStaysRect());
    SkASSERT(!rects[0].isEmpty() && !rects[1].isEmpty());

    SkRect devOutside, devInside;
    viewMatrix.mapRect(&devOutside, rects[0]);
    viewMatrix.mapRect(&devInside, rects[1]);

    if (devInside.isEmpty()) {
        return CreateFillAA(color, viewMatrix, devOutside, devOutside);
    }

    return create_stroke_aa_batch(color, viewMatrix, devOutside, devOutside, devInside, true);
}

};
