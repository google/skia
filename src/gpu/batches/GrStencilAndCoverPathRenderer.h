/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBuiltInPathRenderer_DEFINED
#define GrBuiltInPathRenderer_DEFINED

#include "GrPathRenderer.h"

class GrContext;
class GrGpu;

/**
 * Uses GrGpu::stencilPath followed by a cover rectangle. This subclass doesn't apply AA; it relies
 * on the target having MSAA if AA is desired.
 */
class GrStencilAndCoverPathRenderer : public GrPathRenderer {
public:

    static GrPathRenderer* Create(GrResourceProvider*, const GrCaps&);


private:
    StencilSupport onGetStencilSupport(const SkPath&, const GrStrokeInfo&) const override {
        return GrPathRenderer::kStencilOnly_StencilSupport;
    }

    bool onCanDrawPath(const CanDrawPathArgs&) const override;

    bool onDrawPath(const DrawPathArgs&) override;

    void onStencilPath(const StencilPathArgs&) override;

    GrStencilAndCoverPathRenderer(GrResourceProvider*);

    GrResourceProvider* fResourceProvider;

    typedef GrPathRenderer INHERITED;
};

#endif
