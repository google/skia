/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAARectRenderer_DEFINED
#define GrAARectRenderer_DEFINED

#include "GrColor.h"
#include "SkMatrix.h"
#include "SkRect.h"
#include "SkStrokeRec.h"

class GrClip;
class GrDrawTarget;
class GrIndexBuffer;
class GrPipelineBuilder;

/*
 * This class wraps helper functions that draw AA rects (filled & stroked)
 */
class GrAARectRenderer {
public:
    // TODO: potentialy fuse the fill & stroke methods and differentiate
    // between them by passing in stroke (==NULL means fill).

    static void FillAARect(GrDrawTarget* target,
                           const GrPipelineBuilder& pipelineBuilder,
                           GrColor color,
                           const SkMatrix& viewMatrix,
                           const SkRect& rect,
                           const SkRect& devRect) {
        GeometryFillAARect(target, pipelineBuilder, color, viewMatrix, rect, devRect);
    }

    static void StrokeAARect(GrDrawTarget*,
                             const GrPipelineBuilder&,
                             GrColor,
                             const SkMatrix& viewMatrix,
                             const SkRect& rect,
                             const SkRect& devRect,
                             const SkStrokeRec& stroke);

    // First rect is outer; second rect is inner
    static void FillAANestedRects(GrDrawTarget*,
                                  const GrPipelineBuilder&,
                                  GrColor,
                                  const SkMatrix& viewMatrix,
                                  const SkRect rects[2]);

private:
    GrAARectRenderer();

    static void GeometryFillAARect(GrDrawTarget*,
                                   const GrPipelineBuilder&,
                                   GrColor,
                                   const SkMatrix& viewMatrix,
                                   const SkRect& rect,
                                   const SkRect& devRect);

    static void GeometryStrokeAARect(GrDrawTarget*,
                                     const GrPipelineBuilder&,
                                     GrColor,
                                     const SkMatrix& viewMatrix,
                                     const SkRect& devOutside,
                                     const SkRect& devOutsideAssist,
                                     const SkRect& devInside,
                                     bool miterStroke);
};

#endif // GrAARectRenderer_DEFINED
