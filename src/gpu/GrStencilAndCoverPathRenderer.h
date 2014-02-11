
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

    static GrPathRenderer* Create(GrContext*);

    virtual ~GrStencilAndCoverPathRenderer();

    virtual bool canDrawPath(const SkPath&,
                             const SkStrokeRec&,
                             const GrDrawTarget*,
                             bool antiAlias) const SK_OVERRIDE;

protected:
    virtual StencilSupport onGetStencilSupport(const SkPath&,
                                               const SkStrokeRec&,
                                               const GrDrawTarget*) const SK_OVERRIDE;

    virtual bool onDrawPath(const SkPath&,
                            const SkStrokeRec&,
                            GrDrawTarget*,
                            bool antiAlias) SK_OVERRIDE;

    virtual void onStencilPath(const SkPath&,
                               const SkStrokeRec&,
                               GrDrawTarget*) SK_OVERRIDE;

private:
    GrStencilAndCoverPathRenderer(GrGpu*);

    GrGpu* fGpu;

    typedef GrPathRenderer INHERITED;
};

#endif
