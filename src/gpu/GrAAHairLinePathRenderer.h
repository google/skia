
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAAHairLinePathRenderer_DEFINED
#define GrAAHairLinePathRenderer_DEFINED

#include "GrPathRenderer.h"

class GrAAHairLinePathRenderer : public GrPathRenderer {
public:
    virtual ~GrAAHairLinePathRenderer();

    static GrPathRenderer* Create(GrContext* context);

    virtual bool canDrawPath(const GrDrawTarget*,
                             const GrDrawState*,
                             const SkMatrix& viewMatrix,
                             const SkPath&,
                             const SkStrokeRec&,
                             bool antiAlias) const SK_OVERRIDE;

    typedef SkTArray<SkPoint, true> PtArray;
    typedef SkTArray<int, true> IntArray;
    typedef SkTArray<float, true> FloatArray;

protected:
    virtual bool onDrawPath(GrDrawTarget*,
                            GrDrawState*,
                            GrColor,
                            const SkMatrix& viewMatrix,
                            const SkPath&,
                            const SkStrokeRec&,
                            bool antiAlias) SK_OVERRIDE;

private:
    GrAAHairLinePathRenderer(const GrContext* context,
                             const GrIndexBuffer* fLinesIndexBuffer,
                             const GrIndexBuffer* fQuadsIndexBuffer);

    bool createLineGeom(GrDrawTarget* target,
                        GrDrawState*,
                        const SkMatrix& viewMatrix,
                        uint8_t coverage,
                        size_t vertexStride,
                        GrDrawTarget::AutoReleaseGeometry* arg,
                        SkRect* devBounds,
                        const SkPath& path,
                        const PtArray& lines,
                        int lineCnt);

    bool createBezierGeom(GrDrawTarget* target,
                          GrDrawState*,
                          const SkMatrix& viewMatrix,
                          GrDrawTarget::AutoReleaseGeometry* arg,
                          SkRect* devBounds,
                          const SkPath& path,
                          const PtArray& quads,
                          int quadCnt,
                          const PtArray& conics,
                          int conicCnt,
                          const IntArray& qSubdivs,
                          const FloatArray& cWeights,
                          size_t vertexStride);

    const GrIndexBuffer*        fLinesIndexBuffer;
    const GrIndexBuffer*        fQuadsIndexBuffer;

    typedef GrPathRenderer INHERITED;
};


#endif
