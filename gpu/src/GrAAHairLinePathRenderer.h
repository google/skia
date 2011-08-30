
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
    // GrPathRenderer overrides
    virtual bool supportsAA(GrDrawTarget* target,
                            const SkPath& path,
                            GrPathFill fill);
    virtual bool canDrawPath(const GrDrawTarget* target,
                             const SkPath& path,
                             GrPathFill fill) const;
    virtual void drawPath(GrDrawTarget::StageBitfield stages);

protected:

    // GrPathRenderer overrides
    virtual void pathWillClear();

private:
    void resetGeom();

    static bool CanBeUsed(const GrContext* context);
    GrAAHairLinePathRenderer(const GrContext* context,
                             const GrIndexBuffer* fLinesIndexBuffer,
                             const GrIndexBuffer* fQuadsIndexBuffer);

    bool createGeom(GrDrawTarget::StageBitfield stages);

    GrContext*                  fContext;
    const GrIndexBuffer*        fLinesIndexBuffer;
    const GrIndexBuffer*        fQuadsIndexBuffer;

    // have to recreate geometry if stages in use changes :(
    GrDrawTarget::StageBitfield fPreviousStages;
    int                         fPreviousRTHeight;
    GrIRect                     fClipRect;

    // this path renderer draws everything in device coordinates
    GrMatrix                    fPreviousViewMatrix;
    int                         fLineSegmentCnt;
    int                         fQuadCnt;

    typedef GrPathRenderer INHERITED;
};


#endif

