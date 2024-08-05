/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_ganesh_StrokeRectOp_DEFINED
#define skgpu_ganesh_StrokeRectOp_DEFINED

#include "src/gpu/ganesh/ops/GrOp.h"

class GrPaint;
class GrRecordingContext;
class SkMatrix;
class SkStrokeRec;
enum class GrAAType : unsigned int;
struct SkRect;

/**
 * A set of factory functions for drawing stroked rectangles either coverage-antialiased, or
 * non-antialiased. The non-antialiased ops can be used with MSAA. As with other GrDrawOp factories,
 * the GrPaint is only consumed by these methods if a valid op is returned. If null is returned then
 * the paint is unmodified and may still be used.
 */
namespace skgpu::ganesh::StrokeRectOp {

GrOp::Owner Make(GrRecordingContext*,
                 GrPaint&&,
                 GrAAType,
                 const SkMatrix& viewMatrix,
                 const SkRect&,
                 const SkStrokeRec&);

// rects[0] == outer rectangle, rects[1] == inner rectangle. Null return means there is nothing to
// draw rather than failure. The area between the rectangles will be filled by the paint, and it
// will be anti-aliased with coverage AA. viewMatrix.rectStaysRect() must be true.
GrOp::Owner MakeNested(GrRecordingContext*,
                       GrPaint&&,
                       const SkMatrix& viewMatrix,
                       const SkRect rects[2]);

} // namespace skgpu::ganesh::StrokeRectOp

#endif // skgpu_ganesh_StrokeRectOp_DEFINED
