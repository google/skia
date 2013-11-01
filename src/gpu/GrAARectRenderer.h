/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAARectRenderer_DEFINED
#define GrAARectRenderer_DEFINED

#include "SkMatrix.h"
#include "SkRect.h"
#include "SkRefCnt.h"

class GrGpu;
class GrDrawTarget;
class GrIndexBuffer;

/*
 * This class wraps helper functions that draw AA rects (filled & stroked)
 */
class GrAARectRenderer : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(GrAARectRenderer)

    GrAARectRenderer()
    : fAAFillRectIndexBuffer(NULL)
    , fAAStrokeRectIndexBuffer(NULL) {
    }

    void reset();

    ~GrAARectRenderer() {
        this->reset();
    }

    // TODO: potentialy fuse the fill & stroke methods and differentiate
    // between them by passing in strokeWidth (<0 means fill).

    void fillAARect(GrGpu* gpu,
                    GrDrawTarget* target,
                    const SkRect& rect,
                    const SkMatrix& combinedMatrix,
                    const SkRect& devRect,
                    bool useVertexCoverage) {
#ifdef SHADER_AA_FILL_RECT
        if (combinedMatrix.rectStaysRect()) {
            this->shaderFillAlignedAARect(gpu, target,
                                          rect, combinedMatrix);
        } else {
            this->shaderFillAARect(gpu, target,
                                   rect, combinedMatrix);
        }
#else
        this->geometryFillAARect(gpu, target,
                                 rect, combinedMatrix,
                                 devRect, useVertexCoverage);
#endif
    }

    void strokeAARect(GrGpu* gpu,
                      GrDrawTarget* target,
                      const SkRect& rect,
                      const SkMatrix& combinedMatrix,
                      const SkRect& devRect,
                      SkScalar width,
                      bool useVertexCoverage);

    // First rect is outer; second rect is inner
    void fillAANestedRects(GrGpu* gpu,
                           GrDrawTarget* target,
                           const SkRect rects[2],
                           const SkMatrix& combinedMatrix,
                           bool useVertexCoverage);

private:
    GrIndexBuffer*              fAAFillRectIndexBuffer;
    GrIndexBuffer*              fAAStrokeRectIndexBuffer;

    GrIndexBuffer* aaFillRectIndexBuffer(GrGpu* gpu);

    static int aaStrokeRectIndexCount();
    GrIndexBuffer* aaStrokeRectIndexBuffer(GrGpu* gpu);

    // TODO: Remove the useVertexCoverage boolean. Just use it all the time
    // since we now have a coverage vertex attribute
    void geometryFillAARect(GrGpu* gpu,
                            GrDrawTarget* target,
                            const SkRect& rect,
                            const SkMatrix& combinedMatrix,
                            const SkRect& devRect,
                            bool useVertexCoverage);

    void shaderFillAARect(GrGpu* gpu,
                          GrDrawTarget* target,
                          const SkRect& rect,
                          const SkMatrix& combinedMatrix);

    void shaderFillAlignedAARect(GrGpu* gpu,
                                 GrDrawTarget* target,
                                 const SkRect& rect,
                                 const SkMatrix& combinedMatrix);

    void geometryStrokeAARect(GrGpu* gpu,
                              GrDrawTarget* target,
                              const SkRect& devOutside,
                              const SkRect& devInside,
                              bool useVertexCoverage);

    typedef SkRefCnt INHERITED;
};

#endif // GrAARectRenderer_DEFINED
