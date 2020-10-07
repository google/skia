/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSmallPathRenderer_DEFINED
#define GrSmallPathRenderer_DEFINED

#include "src/gpu/GrPathRenderer.h"
#include "src/gpu/ops/GrOp.h"

class GrDrawOp;
class GrRecordingContext;
class GrStyledShape;

class GrSmallPathRenderer : public GrPathRenderer {
public:
    GrSmallPathRenderer();
    ~GrSmallPathRenderer() override;

    const char* name() const final { return "Small"; }

    static GrOp::Owner createOp_TestingOnly(GrRecordingContext*,
                                            GrPaint&&,
                                            const GrStyledShape&,
                                            const SkMatrix& viewMatrix,
                                            bool gammaCorrect,
                                            const GrUserStencilSettings*);

private:
    class SmallPathOp;

    StencilSupport onGetStencilSupport(const GrStyledShape&) const override {
        return GrPathRenderer::kNoSupport_StencilSupport;
    }

    CanDrawPath onCanDrawPath(const CanDrawPathArgs&) const override;

    bool onDrawPath(const DrawPathArgs&) override;

    using INHERITED = GrPathRenderer;
};

#endif
