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
    bool inverted;
    if (args.fShape->style().isDashed() && args.fShape->asLine(pts, &inverted)) {
        // We should never have an inverse dashed case.
        SkASSERT(!inverted);
        return GrDashingEffect::CanDrawDashLine(pts, args.fShape->style(), *args.fViewMatrix);
    }
    return false;
}

bool GrDashLinePathRenderer::onDrawPath(const DrawPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fDrawContext->auditTrail(),
                              "GrDashLinePathRenderer::onDrawPath");
    bool useHWAA = args.fDrawContext->isUnifiedMultisampled();
    GrDashingEffect::AAMode aaMode;
    if (useHWAA) {
        // We ignore args.fAntiAlias here and force anti aliasing when using MSAA. Otherwise,
        // we can wind up with external edges antialiased and internal edges unantialiased.
        aaMode = GrDashingEffect::AAMode::kCoverageWithMSAA;
    } else if (args.fAntiAlias) {
        aaMode = GrDashingEffect::AAMode::kCoverage;
    } else {
        aaMode = GrDashingEffect::AAMode::kNone;
    }
    SkPoint pts[2];
    SkAssertResult(args.fShape->asLine(pts, nullptr));
    SkAutoTUnref<GrDrawBatch> batch(GrDashingEffect::CreateDashLineBatch(args.fColor,
                                                                         *args.fViewMatrix,
                                                                         pts,
                                                                         aaMode,
                                                                         args.fShape->style()));
    if (!batch) {
        return false;
    }

    GrPipelineBuilder pipelineBuilder(*args.fPaint, useHWAA);
    pipelineBuilder.setUserStencil(args.fUserStencilSettings);

    args.fDrawContext->drawBatch(pipelineBuilder, *args.fClip, batch);
    return true;
}
