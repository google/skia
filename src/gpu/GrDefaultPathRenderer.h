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
 *  Subclass that renders the path using the stencil buffer to resolve fill rules
 * (e.g. winding, even-odd)
 */
class GR_API GrDefaultPathRenderer : public GrPathRenderer {
public:
    GrDefaultPathRenderer(bool separateStencilSupport, bool stencilWrapOpsSupport);

    virtual bool canDrawPath(const SkPath&,
                             const SkStroke&,
                             const GrDrawTarget*,
                             bool antiAlias) const SK_OVERRIDE;

private:

    virtual StencilSupport onGetStencilSupport(const SkPath&,
                                               const SkStroke&,
                                               const GrDrawTarget*) const SK_OVERRIDE;

    virtual bool onDrawPath(const SkPath&,
                            const SkStroke&,
                            GrDrawTarget*,
                            bool antiAlias) SK_OVERRIDE;

    virtual void onStencilPath(const SkPath&,
                               const SkStroke&,
                               GrDrawTarget*) SK_OVERRIDE;

    bool internalDrawPath(const SkPath&,
                          const SkStroke&,
                          GrDrawTarget*,
                          bool stencilOnly);

    bool createGeom(const SkPath&,
                    const SkStroke&,
                    SkScalar srcSpaceTol,
                    GrDrawTarget*,
                    GrPrimitiveType*,
                    int* vertexCnt,
                    int* indexCnt,
                    GrDrawTarget::AutoReleaseGeometry*);

    bool    fSeparateStencil;
    bool    fStencilWrapOps;

    typedef GrPathRenderer INHERITED;
};

#endif
