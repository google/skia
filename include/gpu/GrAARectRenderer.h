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
    // btween them by passing in strokeWidth (<0 means fill).

    // TODO: Remove the useVertexCoverage boolean. Just use it all the time
    // since we now have a coverage vertex attribute
    void fillAARect(GrGpu* gpu,
                    GrDrawTarget* target,
                    const GrRect& devRect,
                    bool useVertexCoverage);

    void strokeAARect(GrGpu* gpu,
                      GrDrawTarget* target,
                      const GrRect& devRect,
                      const GrVec& devStrokeSize,
                      bool useVertexCoverage);

private:
    GrIndexBuffer*              fAAFillRectIndexBuffer;
    GrIndexBuffer*              fAAStrokeRectIndexBuffer;

    static const uint16_t       gFillAARectIdx[];
    static const uint16_t       gStrokeAARectIdx[];

    static int aaFillRectIndexCount();
    GrIndexBuffer* aaFillRectIndexBuffer(GrGpu* gpu);

    static int aaStrokeRectIndexCount();
    GrIndexBuffer* aaStrokeRectIndexBuffer(GrGpu* gpu);

    typedef GrRefCnt INHERITED;
};

#endif // GrAARectRenderer_DEFINED
