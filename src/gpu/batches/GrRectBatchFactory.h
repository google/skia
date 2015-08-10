/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRectBatchFactory_DEFINED
#define GrRectBatchFactory_DEFINED

#include "GrColor.h"

class GrBatch;
class SkMatrix;
struct SkRect;
class SkStrokeRec;

/*
 * A factory for returning batches which can draw rectangles.
 */
namespace GrRectBatchFactory {

GrBatch* CreateFillBW(GrColor color,
                      const SkMatrix& viewMatrix,
                      const SkRect& rect,
                      const SkRect* localRect,
                      const SkMatrix* localMatrix);

GrBatch* CreateFillAA(GrColor color,
                      const SkMatrix& viewMatrix,
                      const SkRect& rect,
                      const SkRect& devRect);

GrBatch* CreateStrokeBW(GrColor color,
                        const SkMatrix& viewMatrix,
                        const SkRect& rect,
                        SkScalar strokeWidth,
                        bool snapToPixelCenters);

GrBatch* CreateStrokeAA(GrColor,
                        const SkMatrix& viewMatrix,
                        const SkRect& rect,
                        const SkRect& devRect,
                        const SkStrokeRec& stroke);

// First rect is outer; second rect is inner
GrBatch* CreateFillNestedRectsAA(GrColor,
                                 const SkMatrix& viewMatrix,
                                 const SkRect rects[2]);

};

#endif
