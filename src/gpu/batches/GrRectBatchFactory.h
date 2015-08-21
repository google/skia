/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRectBatchFactory_DEFINED
#define GrRectBatchFactory_DEFINED

#include "GrAAFillRectBatch.h"
#include "GrBWFillRectBatch.h"
#include "GrColor.h"

class GrBatch;
class SkMatrix;
struct SkRect;
class SkStrokeRec;

/*
 * A factory for returning batches which can draw rectangles.
 */
namespace GrRectBatchFactory {

inline GrDrawBatch* CreateNonAAFill(GrColor color,
                                    const SkMatrix& viewMatrix,
                                    const SkRect& rect,
                                    const SkRect* localRect,
                                    const SkMatrix* localMatrix) {
    return GrBWFillRectBatch::Create(color, viewMatrix, rect, localRect, localMatrix);
}

inline GrDrawBatch* CreateAAFill(GrColor color,
                                 const SkMatrix& viewMatrix,
                                 const SkRect& rect,
                                 const SkRect& devRect) {
    return GrAAFillRectBatch::Create(color, viewMatrix, rect, devRect);
}

inline GrDrawBatch* CreateAAFill(GrColor color,
                                 const SkMatrix& viewMatrix,
                                 const SkMatrix& localMatrix,
                                 const SkRect& rect,
                                 const SkRect& devRect) {
    return GrAAFillRectBatch::Create(color, viewMatrix, localMatrix, rect, devRect);
}

GrDrawBatch* CreateNonAAStroke(GrColor color,
                               const SkMatrix& viewMatrix,
                               const SkRect& rect,
                               SkScalar strokeWidth,
                               bool snapToPixelCenters);

GrDrawBatch* CreateAAStroke(GrColor,
                            const SkMatrix& viewMatrix,
                            const SkRect& rect,
                            const SkRect& devRect,
                            const SkStrokeRec& stroke);

// First rect is outer; second rect is inner
GrDrawBatch* CreateAAFillNestedRects(GrColor,
                                     const SkMatrix& viewMatrix,
                                     const SkRect rects[2]);

};

#endif
