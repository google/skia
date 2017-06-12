/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRectOpFactory_DEFINED
#define GrRectOpFactory_DEFINED

#include "GrAAFillRectOp.h"
#include "GrAAStrokeRectOp.h"
#include "GrNonAAStrokeRectOp.h"
#include "GrDrawOp.h"

class GrDrawOp;
class GrPaint;
struct SkRect;
class SkMatrix;
class SkStrokeRec;

/**
 * A factory for returning GrDrawOps which can draw rectangles.
 */
namespace GrRectOpFactory {

inline std::unique_ptr<GrDrawOp> MakeNonAAStroke(GrPaint&& paint,
                                                           const SkMatrix& viewMatrix,
                                                           const SkRect& rect,
                                                           const SkStrokeRec& strokeRec,
                                                           bool snapToPixelCenters) {
    return GrNonAAStrokeRectOp::Make(std::move(paint), viewMatrix, rect, strokeRec, snapToPixelCenters);
}

inline std::unique_ptr<GrDrawOp> MakeAAStroke(GrPaint&& paint,
                                                        const SkMatrix& viewMatrix,
                                                        const SkRect& rect,
                                                        const SkStrokeRec& stroke) {
    return GrAAStrokeRectOp::Make(std::move(paint), viewMatrix, rect, stroke);
}

// First rect is outer; second rect is inner
std::unique_ptr<GrDrawOp> MakeAAFillNestedRects(GrPaint&&, const SkMatrix& viewMatrix,
                                                          const SkRect rects[2]);
};

#endif
