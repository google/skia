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
#include "SkMatrix.h"

class GrGpu;
class GrDrawTarget;
class GrIndexBuffer;

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
                                 useVertexCoverage);
#endif
    }

    void strokeAARect(GrGpu* gpu,
                      GrDrawTarget* target,
                      const GrRect& rect,
                      const SkMatrix& combinedMatrix,
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
                            bool useVertexCoverage);

    void shaderFillAARect(GrGpu* gpu,
                          GrDrawTarget* target,
                          const GrRect& rect,
                          const SkMatrix& combinedMatrix);

    void shaderFillAlignedAARect(GrGpu* gpu,
                                 GrDrawTarget* target,
                                 const GrRect& rect,
                                 const SkMatrix& combinedMatrix);

    typedef GrRefCnt INHERITED;
};

#endif // GrAARectRenderer_DEFINED
