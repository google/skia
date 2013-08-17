
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

    virtual bool canDrawPath(const SkPath& path,
                             const SkStrokeRec& stroke,
                             const GrDrawTarget* target,
                             bool antiAlias) const SK_OVERRIDE;

    typedef SkTArray<SkPoint, true> PtArray;
    typedef SkTArray<int, true> IntArray;
    typedef SkTArray<float, true> FloatArray;

protected:
    virtual bool onDrawPath(const SkPath& path,
                            const SkStrokeRec& stroke,
                            GrDrawTarget* target,
                            bool antiAlias) SK_OVERRIDE;

private:
    GrAAHairLinePathRenderer(const GrContext* context,
                             const GrIndexBuffer* fLinesIndexBuffer,
                             const GrIndexBuffer* fQuadsIndexBuffer);

    bool createLineGeom(const SkPath& path,
                        GrDrawTarget* target,
                        const PtArray& lines,
                        int lineCnt,
                        GrDrawTarget::AutoReleaseGeometry* arg,
                        SkRect* devBounds);

    bool createBezierGeom(const SkPath& path,
                          GrDrawTarget* target,
                          const PtArray& quads,
                          int quadCnt,
                          const PtArray& conics,
                          int conicCnt,
                          const IntArray& qSubdivs,
                          const FloatArray& cWeights,
                          GrDrawTarget::AutoReleaseGeometry* arg,
                          SkRect* devBounds);

    const GrIndexBuffer*        fLinesIndexBuffer;
    const GrIndexBuffer*        fQuadsIndexBuffer;

    typedef GrPathRenderer INHERITED;
};


#endif
