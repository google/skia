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
    GrDefaultPathRenderer();

private:
    StencilSupport onGetStencilSupport(const GrShape&) const override;

    bool onCanDrawPath(const CanDrawPathArgs&) const override;

    bool onDrawPath(const DrawPathArgs&) override;

    void onStencilPath(const StencilPathArgs&) override;

    bool internalDrawPath(GrRenderTargetContext*,
                          GrPaint&&,
                          GrAAType,
                          const GrUserStencilSettings&,
                          const GrClip&,
                          const SkMatrix& viewMatrix,
                          const GrShape&,
                          bool stencilOnly);

    typedef GrPathRenderer INHERITED;
};

#endif
