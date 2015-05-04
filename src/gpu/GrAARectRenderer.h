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
#include "SkRefCnt.h"
#include "SkStrokeRec.h"

class GrClip;
class GrDrawTarget;
class GrIndexBuffer;
class GrPipelineBuilder;

/*
 * This class wraps helper functions that draw AA rects (filled & stroked)
 */
class GrAARectRenderer : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(GrAARectRenderer)

    // TODO: potentialy fuse the fill & stroke methods and differentiate
    // between them by passing in stroke (==NULL means fill).

    void fillAARect(GrDrawTarget* target,
                    GrPipelineBuilder* pipelineBuilder,
                    GrColor color,
                    const SkMatrix& viewMatrix,
                    const SkRect& rect,
                    const SkRect& devRect) {
        this->geometryFillAARect(target, pipelineBuilder, color, viewMatrix, rect, devRect);
    }

    void strokeAARect(GrDrawTarget*,
                      GrPipelineBuilder*,
                      GrColor,
                      const SkMatrix& viewMatrix,
                      const SkRect& rect,
                      const SkRect& devRect,
                      const SkStrokeRec& stroke);

    // First rect is outer; second rect is inner
    void fillAANestedRects(GrDrawTarget*,
                           GrPipelineBuilder*,
                           GrColor,
                           const SkMatrix& viewMatrix,
                           const SkRect rects[2]);

private:
    void geometryFillAARect(GrDrawTarget*,
                            GrPipelineBuilder*,
                            GrColor,
                            const SkMatrix& viewMatrix,
                            const SkRect& rect,
                            const SkRect& devRect);

    void geometryStrokeAARect(GrDrawTarget*,
                              GrPipelineBuilder*,
                              GrColor,
                              const SkMatrix& viewMatrix,
                              const SkRect& devOutside,
                              const SkRect& devOutsideAssist,
                              const SkRect& devInside,
                              bool miterStroke);

    typedef SkRefCnt INHERITED;
};

#endif // GrAARectRenderer_DEFINED
