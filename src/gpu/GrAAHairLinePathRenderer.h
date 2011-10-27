
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
    virtual bool canDrawPath(const GrDrawTarget::Caps& targetCaps,
                             const SkPath& path,
                             GrPathFill fill,
                             bool antiAlias) const  SK_OVERRIDE;
    virtual void drawPath(GrDrawTarget::StageBitfield stages) SK_OVERRIDE;

protected:

    // GrPathRenderer overrides
    virtual void pathWillClear()  SK_OVERRIDE;

private:
    void resetGeom();

    GrAAHairLinePathRenderer(const GrContext* context,
                             const GrIndexBuffer* fLinesIndexBuffer,
                             const GrIndexBuffer* fQuadsIndexBuffer);

    bool createGeom(GrDrawTarget::StageBitfield stages);

    const GrIndexBuffer*        fLinesIndexBuffer;
    const GrIndexBuffer*        fQuadsIndexBuffer;

    // have to recreate geometry if stages in use changes :(
    GrDrawTarget::StageBitfield fPreviousStages;
    int                         fPreviousRTHeight;
    SkVector                    fPreviousTranslate;
    GrIRect                     fClipRect;

    // this path renderer draws everything in device coordinates
    GrMatrix                    fPreviousViewMatrix;
    int                         fLineSegmentCnt;
    int                         fQuadCnt;

    typedef GrPathRenderer INHERITED;
};


#endif

