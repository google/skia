/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMSAAPathRenderer_DEFINED
#define GrMSAAPathRenderer_DEFINED

#include "GrPathRenderer.h"
#include "SkTypes.h"

class SK_API GrMSAAPathRenderer : public GrPathRenderer {
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
