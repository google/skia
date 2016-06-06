/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDashLinePathRenderer.h"

#include "GrGpu.h"
#include "GrAuditTrail.h"
#include "effects/GrDashingEffect.h"

bool GrDashLinePathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
    SkPoint pts[2];
    if (args.fStyle->isDashed() && args.fPath->isLine(pts)) {
        return GrDashingEffect::CanDrawDashLine(pts, *args.fStyle, *args.fViewMatrix);
    }
    return false;
}

bool GrDashLinePathRenderer::onDrawPath(const DrawPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fDrawContext->auditTrail(),
                              "GrDashLinePathRenderer::onDrawPath");
    bool msaaIsEnabled = args.fDrawContext->isUnifiedMultisampled();
    SkPoint pts[2];
    SkAssertResult(args.fPath->isLine(pts));
    SkAutoTUnref<GrDrawBatch> batch(GrDashingEffect::CreateDashLineBatch(args.fColor,
                                                                         *args.fViewMatrix,
                                                                         pts,
                                                                         args.fAntiAlias,
                                                                         msaaIsEnabled,
                                                                         *args.fStyle));
    if (!batch) {
        return false;
    }

    GrPipelineBuilder pipelineBuilder(*args.fPaint, msaaIsEnabled);
    pipelineBuilder.setUserStencil(args.fUserStencilSettings);

    args.fDrawContext->drawBatch(pipelineBuilder, *args.fClip, batch);
    return true;
}
