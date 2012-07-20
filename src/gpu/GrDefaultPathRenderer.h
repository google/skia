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


    virtual bool requiresStencilPass(const SkPath& path,
                                     GrPathFill fill,
                                     const GrDrawTarget* target) const SK_OVERRIDE;

    virtual bool canDrawPath(const SkPath& path,
                            GrPathFill fill,
                            const GrDrawTarget* target,
                            bool antiAlias) const SK_OVERRIDE;

    virtual void drawPathToStencil(const SkPath& path,
                                   GrPathFill fill,
                                   GrDrawTarget* target) SK_OVERRIDE;

private:

    virtual bool onDrawPath(const SkPath& path,
                            GrPathFill fill,
                            const GrVec* translate,
                            GrDrawTarget* target,
                            bool antiAlias) SK_OVERRIDE;

    bool internalDrawPath(const SkPath& path,
                          GrPathFill fill,
                          const GrVec* translate,
                          GrDrawTarget* target,
                          bool stencilOnly);

    bool createGeom(const SkPath& path,
                    GrPathFill fill,
                    const GrVec* translate,
                    GrScalar srcSpaceTol,
                    GrDrawTarget* target,
                    GrPrimitiveType* primType,
                    int* vertexCnt,
                    int* indexCnt,
                    GrDrawTarget::AutoReleaseGeometry* arg);

    bool    fSeparateStencil;
    bool    fStencilWrapOps;

    typedef GrPathRenderer INHERITED;
};

#endif
