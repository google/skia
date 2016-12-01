/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRectBatchFactory_DEFINED
#define GrRectBatchFactory_DEFINED

#include "GrAAFillRectBatch.h"
#include "GrAAStrokeRectBatch.h"
#include "GrAnalyticRectBatch.h"
#include "GrColor.h"
#include "GrNonAAFillRectBatch.h"
#include "GrNonAAStrokeRectBatch.h"
#include "GrPaint.h"
#include "SkMatrix.h"

class GrOp;
struct SkRect;
class SkStrokeRec;

/*
 * A factory for returning batches which can draw rectangles.
 */
namespace GrRectBatchFactory {

inline GrDrawOp* CreateNonAAFill(GrColor color,
                                 const SkMatrix& viewMatrix,
                                 const SkRect& rect,
                                 const SkRect* localRect,
                                 const SkMatrix* localMatrix) {
    if (viewMatrix.hasPerspective() || (localMatrix && localMatrix->hasPerspective())) {
        return GrNonAAFillRectBatch::CreateWithPerspective(color, viewMatrix, rect, localRect,
                                                           localMatrix);
    } else {
        return GrNonAAFillRectBatch::Create(color, viewMatrix, rect, localRect, localMatrix);
    }
}

inline GrDrawOp* CreateAAFill(const GrPaint& paint,
                              const SkMatrix& viewMatrix,
                              const SkRect& rect,
                              const SkRect& croppedRect,
                              const SkRect& devRect) {
    if (!paint.usesDistanceVectorField()) {
        return GrAAFillRectBatch::Create(paint.getColor(), viewMatrix, croppedRect, devRect);
    } else {
        return GrAnalyticRectBatch::CreateAnalyticRectBatch(paint.getColor(), viewMatrix, rect,
                                                            croppedRect, devRect);
    }
}

inline GrDrawOp* CreateAAFill(GrColor color,
                              const SkMatrix& viewMatrix,
                              const SkMatrix& localMatrix,
                              const SkRect& rect,
                              const SkRect& devRect) {
    return GrAAFillRectBatch::Create(color, viewMatrix, localMatrix, rect, devRect);
}

inline GrDrawOp* CreateNonAAStroke(GrColor color,
                                   const SkMatrix& viewMatrix,
                                   const SkRect& rect,
                                   const SkStrokeRec& strokeRec,
                                   bool snapToPixelCenters) {
    return GrNonAAStrokeRectBatch::Create(color, viewMatrix, rect, strokeRec, snapToPixelCenters);
}

inline GrDrawOp* CreateAAStroke(GrColor color,
                                const SkMatrix& viewMatrix,
                                const SkRect& rect,
                                const SkStrokeRec& stroke) {
    return GrAAStrokeRectBatch::Create(color, viewMatrix, rect, stroke);
}

// First rect is outer; second rect is inner
GrDrawOp* CreateAAFillNestedRects(GrColor, const SkMatrix& viewMatrix, const SkRect rects[2]);

};

#endif
