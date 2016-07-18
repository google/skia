/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDefaultPathRenderer_DEFINED
#define GrDefaultPathRenderer_DEFINED

#include "GrPathRenderer.h"
#include "GrPathStencilSettings.h"
#include "SkTypes.h"

/**
 *  Subclass that renders the path using the stencil buffer to resolve fill rules
 * (e.g. winding, even-odd)
 */
class SK_API GrDefaultPathRenderer : public GrPathRenderer {
public:
    GrDefaultPathRenderer(bool separateStencilSupport, bool stencilWrapOpsSupport);

private:

    StencilSupport onGetStencilSupport(const GrShape&) const override;

    bool onCanDrawPath(const CanDrawPathArgs&) const override;

    bool onDrawPath(const DrawPathArgs&) override;

    void onStencilPath(const StencilPathArgs&) override;

    bool internalDrawPath(GrDrawContext*,
                          const GrPaint&,
                          const GrUserStencilSettings*,
                          const GrClip&,
                          GrColor,
                          const SkMatrix& viewMatrix,
                          const GrShape&,
                          bool stencilOnly);

    bool    fSeparateStencil;
    bool    fStencilWrapOps;

    typedef GrPathRenderer INHERITED;
};

#endif
