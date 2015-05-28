/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDashLinePathRenderer.h"

#include "GrGpu.h"
#include "effects/GrDashingEffect.h"

GrDashLinePathRenderer::GrDashLinePathRenderer(GrContext* context)
        : fGpu(SkRef(context->getGpu())) {
}

GrDashLinePathRenderer::~GrDashLinePathRenderer() {
}

bool GrDashLinePathRenderer::canDrawPath(const GrDrawTarget* target,
                                         const GrPipelineBuilder* pipelineBuilder,
                                         const SkMatrix& viewMatrix,
                                         const SkPath& path,
                                         const GrStrokeInfo& stroke,
                                         bool antiAlias) const {
    SkPoint pts[2];
    if (stroke.isDashed() && path.isLine(pts)) {
        return GrDashingEffect::CanDrawDashLine(pts, stroke, viewMatrix);
    }
    return false;
}

bool GrDashLinePathRenderer::onDrawPath(GrDrawTarget* target,
                                        GrPipelineBuilder* pipelineBuilder,
                                        GrColor color,
                                        const SkMatrix& viewMatrix,
                                        const SkPath& path,
                                        const GrStrokeInfo& stroke,
                                        bool useAA) {
    SkPoint pts[2];
    SkAssertResult(path.isLine(pts));
    return GrDashingEffect::DrawDashLine(target, pipelineBuilder, color,
                                         viewMatrix, pts, useAA, stroke);
}
