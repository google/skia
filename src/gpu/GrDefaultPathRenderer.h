/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDefaultPathRenderer_DEFINED
#define GrDefaultPathRenderer_DEFINED

#include "GrPathRenderer.h"
#include "SkTemplates.h"

/**
 *  Subclass that renders the path using the stencil buffer to resolve fill
 *  rules (e.g. winding, even-odd)
 */
class GR_API GrDefaultPathRenderer : public GrPathRenderer {
public:
    GrDefaultPathRenderer(bool separateStencilSupport,
                          bool stencilWrapOpsSupport);

    virtual bool canDrawPath(const GrDrawTarget::Caps& targetCaps,
                             const SkPath& path,
                             GrPathFill fill,
                             bool antiAlias) const SK_OVERRIDE;

    virtual bool requiresStencilPass(const GrDrawTarget* target,
                                     const SkPath& path,
                                     GrPathFill fill) const SK_OVERRIDE;

    virtual void drawPath(GrDrawTarget::StageBitfield stages) SK_OVERRIDE;
    virtual void drawPathToStencil() SK_OVERRIDE;

protected:
    virtual void pathWillClear();

private:

    void onDrawPath(GrDrawTarget::StageBitfield stages, bool stencilOnly);

    bool createGeom(GrScalar srcSpaceTol,
                    GrDrawTarget::StageBitfield stages);

    bool    fSeparateStencil;
    bool    fStencilWrapOps;

    int                         fSubpathCount;
    SkAutoSTMalloc<8, uint16_t> fSubpathVertCount;
    int                         fIndexCnt;
    int                         fVertexCnt;
    GrScalar                    fPreviousSrcTol;
    GrDrawTarget::StageBitfield fPreviousStages;
    GrPrimitiveType             fPrimitiveType;
    bool                        fUseIndexedDraw;

    typedef GrPathRenderer INHERITED;
};

#endif
