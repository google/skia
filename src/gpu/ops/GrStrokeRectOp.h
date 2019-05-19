/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStrokeRectOp_DEFINED
#define GrStrokeRectOp_DEFINED

#include "include/private/GrTypesPriv.h"

class GrDrawOp;
class GrPaint;
class GrRecordingContext;
class SkMatrix;
struct SkRect;
class SkStrokeRec;

/**
 * A set of factory functions for drawing stroked rectangles either coverage-antialiased, or
 * non-antialiased. The non-antialiased ops can be used with MSAA. As with other GrDrawOp factories,
 * the GrPaint is only consumed by these methods if a valid op is returned. If null is returned then
 * the paint is unmodified and may still be used.
 */
namespace GrStrokeRectOp {

std::unique_ptr<GrDrawOp> Make(GrRecordingContext* context,
                               GrPaint&& paint,
                               GrAAType aaType,
                               const SkMatrix& viewMatrix,
                               const SkRect& rect,
                               const SkStrokeRec& stroke);

// rects[0] == outer rectangle, rects[1] == inner rectangle. Null return means there is nothing to
// draw rather than failure. The area between the rectangles will be filled by the paint, and it
// will be anti-aliased with coverage AA. viewMatrix.rectStaysRect() must be true.
std::unique_ptr<GrDrawOp> MakeNested(GrRecordingContext* context,
                                     GrPaint&& paint,
                                     const SkMatrix& viewMatrix,
                                     const SkRect rects[2]);

}  // namespace GrStrokeRectOp

#endif // GrStrokeRectOp_DEFINED
