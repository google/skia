
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

    virtual bool canDrawPath(const GrDrawTarget*,
                             const GrPipelineBuilder*,
                             const SkMatrix& viewMatrix,
                             const SkPath&,
                             const GrStrokeInfo&,
                             bool antiAlias) const override;

protected:
    virtual StencilSupport onGetStencilSupport(const GrDrawTarget*,
                                               const GrPipelineBuilder*,
                                               const SkPath&,
                                               const GrStrokeInfo&) const override;

    virtual bool onDrawPath(GrDrawTarget*,
                            GrPipelineBuilder*,
                            GrColor,
                            const SkMatrix& viewMatrix,
                            const SkPath&,
                            const GrStrokeInfo&,
                            bool antiAlias) override;

    virtual void onStencilPath(GrDrawTarget*,
                               GrPipelineBuilder*,
                               const SkMatrix& viewMatrix,
                               const SkPath&,
                               const GrStrokeInfo&) override;

private:
    GrStencilAndCoverPathRenderer(GrGpu*);

    GrGpu* fGpu;

    typedef GrPathRenderer INHERITED;
};

#endif
