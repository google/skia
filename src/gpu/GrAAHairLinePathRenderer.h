
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

protected:
    virtual bool onDrawPath(const SkPath& path,
                            const SkStrokeRec& stroke,
                            GrDrawTarget* target,
                            bool antiAlias) SK_OVERRIDE;

private:

    GrAAHairLinePathRenderer(const GrContext* context,
                             const GrIndexBuffer* fLinesIndexBuffer,
                             const GrIndexBuffer* fQuadsIndexBuffer);

    bool createGeom(const SkPath& path,
                    GrDrawTarget* target,
                    int* lineCnt,
                    int* quadCnt,
                    GrDrawTarget::AutoReleaseGeometry* arg);

    const GrIndexBuffer*        fLinesIndexBuffer;
    const GrIndexBuffer*        fQuadsIndexBuffer;

    typedef GrPathRenderer INHERITED;
};


#endif
