
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
                            GrPathFill fill,
                            const GrDrawTarget* target,
                            bool antiAlias) const SK_OVERRIDE;

    static bool staticCanDrawPath(const SkPath& path,
                                  GrPathFill fill,
                                  const GrDrawTarget* target,
                                  bool antiAlias);

protected:
    virtual bool onDrawPath(const SkPath& path,
                            GrPathFill fill,
                            const GrVec* translate,
                            GrDrawTarget* target,
                            GrDrawState::StageMask stageMask,
                            bool antiAlias) SK_OVERRIDE;
 
private:

    GrAAHairLinePathRenderer(const GrContext* context,
                             const GrIndexBuffer* fLinesIndexBuffer,
                             const GrIndexBuffer* fQuadsIndexBuffer);

    bool createGeom(const SkPath& path,
                    const GrVec* translate,
                    GrDrawTarget* target,
                    GrDrawState::StageMask stageMask,
                    int* lineCnt,
                    int* quadCnt,
                    GrDrawTarget::AutoReleaseGeometry* arg);

    const GrIndexBuffer*        fLinesIndexBuffer;
    const GrIndexBuffer*        fQuadsIndexBuffer;

    typedef GrPathRenderer INHERITED;
};


#endif

