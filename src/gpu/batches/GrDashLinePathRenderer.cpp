/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDashLinePathRenderer.h"

#include "GrAuditTrail.h"
#include "GrGpu.h"
#include "GrPipelineBuilder.h"
#include "effects/GrDashingEffect.h"

bool GrDashLinePathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
    SkPoint pts[2];
    bool inverted;
    if (args.fShape->style().isDashed() && args.fShape->asLine(pts, &inverted)) {
        if (args.fAAType == GrAAType::kMixedSamples) {
            return false;
        }
        // We should never have an inverse dashed case.
        SkASSERT(!inverted);
        return GrDashingEffect::CanDrawDashLine(pts, args.fShape->style(), *args.fViewMatrix);
    }
    return false;
}

bool GrDashLinePathRenderer::onDrawPath(const DrawPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fRenderTargetContext->auditTrail(),
                              "GrDashLinePathRenderer::onDrawPath");
    GrDashingEffect::AAMode aaMode = GrDashingEffect::AAMode::kNone;
    switch (args.fAAType) {
        case GrAAType::kNone:
            break;
        case GrAAType::kCoverage:
        case GrAAType::kMixedSamples:
            aaMode = GrDashingEffect::AAMode::kCoverage;
            break;
        case GrAAType::kMSAA:
            // In this mode we will use aa between dashes but the outer border uses MSAA. Otherwise,
            // we can wind up with external edges antialiased and internal edges unantialiased.
            aaMode = GrDashingEffect::AAMode::kCoverageWithMSAA;
            break;
    }
    SkPoint pts[2];
    SkAssertResult(args.fShape->asLine(pts, nullptr));
    sk_sp<GrDrawOp> op(GrDashingEffect::CreateDashLineBatch(
            args.fPaint->getColor(), *args.fViewMatrix, pts, aaMode, args.fShape->style()));
    if (!op) {
        return false;
    }

    GrPipelineBuilder pipelineBuilder(*args.fPaint, args.fAAType);
    pipelineBuilder.setUserStencil(args.fUserStencilSettings);

    args.fRenderTargetContext->addDrawOp(pipelineBuilder, *args.fClip, std::move(op));
    return true;
}
