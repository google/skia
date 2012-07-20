
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

    static GrPathRenderer* Create(GrContext* context);

    virtual ~GrStencilAndCoverPathRenderer();

    virtual bool canDrawPath(const SkPath& path,
                             GrPathFill fill,
                             const GrDrawTarget* target,
                             bool antiAlias) const SK_OVERRIDE;

    virtual bool requiresStencilPass(const SkPath& path,
                                     GrPathFill fill,
                                     const GrDrawTarget* target) const SK_OVERRIDE;

    virtual void drawPathToStencil(const SkPath& path,
                                   GrPathFill fill,
                                   GrDrawTarget* target) SK_OVERRIDE;

protected:
    virtual bool onDrawPath(const SkPath& path,
                            GrPathFill fill,
                            const GrVec* translate,
                            GrDrawTarget* target,
                            bool antiAlias) SK_OVERRIDE;

private:
    GrStencilAndCoverPathRenderer(GrGpu* gpu);

    GrGpu* fGpu;

    typedef GrPathRenderer INHERITED;
};

#endif
