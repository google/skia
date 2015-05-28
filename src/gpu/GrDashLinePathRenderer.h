
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDashLinePathRenderer_DEFINED
#define GrDashLinePathRenderer_DEFINED

#include "GrPathRenderer.h"

class GrDashLinePathRenderer : public GrPathRenderer {
public:
    GrDashLinePathRenderer(GrContext*);
    ~GrDashLinePathRenderer();

    bool canDrawPath(const GrDrawTarget*,
                     const GrPipelineBuilder*,
                     const SkMatrix& viewMatrix,
                     const SkPath&,
                     const GrStrokeInfo&,
                     bool antiAlias) const override;

protected:
    StencilSupport onGetStencilSupport(const GrDrawTarget*,
                                       const GrPipelineBuilder*,
                                       const SkPath&,
                                       const GrStrokeInfo&) const override {
      return kNoSupport_StencilSupport;
    }

    bool onDrawPath(GrDrawTarget*,
                    GrPipelineBuilder*,
                    GrColor,
                    const SkMatrix& viewMatrix,
                    const SkPath&,
                    const GrStrokeInfo&,
                    bool antiAlias) override;
    SkAutoTUnref<GrGpu> fGpu;
    typedef GrPathRenderer INHERITED;
};


#endif
