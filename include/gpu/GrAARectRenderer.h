/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrAARectRenderer_DEFINED
#define GrAARectRenderer_DEFINED

#include "GrRect.h"
#include "GrRefCnt.h"

class GrGpu;
class GrDrawTarget;
class GrIndexBuffer;
class SkMatrix;

/*
 * This class wraps helper functions that draw AA rects (filled & stroked)
 */
class GrAARectRenderer : public GrRefCnt {
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
                    const GrRect& rect,
                    const SkMatrix& combinedMatrix,
                    const GrRect& devRect,
                    bool useVertexCoverage) {
#ifdef SHADER_AA_FILL_RECT
        if (combinedMatrix.rectStaysRect()) {
            this->shaderFillAlignedAARect(gpu, target,
                                          combinedMatrix, devRect);
        } else {
            this->shaderFillAARect(gpu, target,
                                   rect, combinedMatrix, devRect);
        }
#else
        this->geometryFillAARect(gpu, target,
                                 rect, combinedMatrix,
                                 devRect, useVertexCoverage);
#endif
    }

    void strokeAARect(GrGpu* gpu,
                      GrDrawTarget* target,
                      const GrRect& devRect,
                      const GrVec& devStrokeSize,
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
                            const GrRect& rect,
                            const SkMatrix& combinedMatrix,
                            const GrRect& devRect,
                            bool useVertexCoverage);

    void shaderFillAARect(GrGpu* gpu,
                          GrDrawTarget* target,
                          const GrRect& rect,
                          const SkMatrix& combinedMatrix,
                          const GrRect& devRect);

    void shaderFillAlignedAARect(GrGpu* gpu,
                                 GrDrawTarget* target,
                                 const GrRect& rect,
                                 const SkMatrix& combinedMatrix,
                                 const GrRect& devRect);

    typedef GrRefCnt INHERITED;
};

#endif // GrAARectRenderer_DEFINED
