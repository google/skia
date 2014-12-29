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

class GrGpu;
class GrDrawState;
class GrDrawTarget;
class GrIndexBuffer;

/*
 * This class wraps helper functions that draw AA rects (filled & stroked)
 */
class GrAARectRenderer : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(GrAARectRenderer)

    GrAARectRenderer(GrGpu* gpu)
    : fGpu(gpu)
    , fAAFillRectIndexBuffer(NULL)
    , fAAMiterStrokeRectIndexBuffer(NULL)
    , fAABevelStrokeRectIndexBuffer(NULL) {
    }

    void reset();

    ~GrAARectRenderer() {
        this->reset();
    }

    // TODO: potentialy fuse the fill & stroke methods and differentiate
    // between them by passing in stroke (==NULL means fill).

    void fillAARect(GrDrawTarget* target,
                    GrDrawState* ds,
                    GrColor color,
                    const SkMatrix& localMatrix,
                    const SkRect& rect,
                    const SkMatrix& combinedMatrix,
                    const SkRect& devRect) {
        this->geometryFillAARect(target, ds, color, localMatrix, rect, combinedMatrix, devRect);
    }

    void strokeAARect(GrDrawTarget*,
                      GrDrawState*,
                      GrColor,
                      const SkMatrix& localMatrix,
                      const SkRect& rect,
                      const SkMatrix& combinedMatrix,
                      const SkRect& devRect,
                      const SkStrokeRec& stroke);

    // First rect is outer; second rect is inner
    void fillAANestedRects(GrDrawTarget*,
                           GrDrawState*,
                           GrColor,
                           const SkMatrix& localMatrix,
                           const SkRect rects[2],
                           const SkMatrix& combinedMatrix);

private:
    GrIndexBuffer* aaStrokeRectIndexBuffer(bool miterStroke);

    void geometryFillAARect(GrDrawTarget*,
                            GrDrawState*,
                            GrColor,
                            const SkMatrix& localMatrix,
                            const SkRect& rect,
                            const SkMatrix& combinedMatrix,
                            const SkRect& devRect);

    void geometryStrokeAARect(GrDrawTarget*,
                              GrDrawState*,
                              GrColor,
                              const SkMatrix& localMatrix,
                              const SkRect& devOutside,
                              const SkRect& devOutsideAssist,
                              const SkRect& devInside,
                              bool miterStroke);

    GrGpu*                      fGpu;
    GrIndexBuffer*              fAAFillRectIndexBuffer;
    GrIndexBuffer*              fAAMiterStrokeRectIndexBuffer;
    GrIndexBuffer*              fAABevelStrokeRectIndexBuffer;

    typedef SkRefCnt INHERITED;
};

#endif // GrAARectRenderer_DEFINED
