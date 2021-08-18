/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef StencilMaskHelper_DEFINED
#define StencilMaskHelper_DEFINED

#include "src/gpu/v1/StencilClip.h"

class GrShape;
class GrRecordingContext;
class SkMatrix;
struct SkRect;
class SkRRect;

namespace skgpu::v1 {

class SurfaceDrawContext;

/**
 * The StencilMaskHelper helps generate clip masks using the stencil buffer.
 * It is intended to be used as:
 *
 *   StencilMaskHelper helper;
 *   helper.init(...);
 *
 *      draw one or more paths/rects specifying the required boolean ops
 *
 *   helper.finish();
 *
 * The result of this process will be the mask stored in the clip bits of the stencil buffer.
 */
class StencilMaskHelper : SkNoncopyable {
public:
    // Configure the helper to update the stencil mask within the given rectangle, respecting the
    // set window rectangles. It will use the provided context and render target to draw into, both
    // of which must outlive the helper.
    StencilMaskHelper(GrRecordingContext*, SurfaceDrawContext*);

    // Returns true if the stencil mask must be redrawn
    bool init(const SkIRect& maskBounds, uint32_t genID,
              const GrWindowRectangles& windowRects, int numFPs);

    // Draw a single rect into the stencil clip using the specified op
    void drawRect(const SkRect& rect, const SkMatrix& matrix, SkRegion::Op, GrAA);

    // Draw a single filled path into the stencil clip with the specified op
    bool drawPath(const SkPath& path, const SkMatrix& matrix, SkRegion::Op, GrAA);

    // Draw a single shape into the stencil clip assuming a simple fill style, with the specified op
    bool drawShape(const GrShape& shape, const SkMatrix& matrix, SkRegion::Op, GrAA);

    // Reset the stencil buffer's clip bit to in or out.
    void clear(bool insideStencil);

    // Marks the last rendered stencil mask on the render target context
    void finish();

private:
    GrRecordingContext* fContext;
    SurfaceDrawContext* fSDC;
    StencilClip         fClip;
    int                 fNumFPs;

    using INHERITED = SkNoncopyable;
};

} // namespace skgpu::v1

#endif // StencilMaskHelper_DEFINED
