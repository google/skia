/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDashLinePathRenderer.h"

#include "GrAuditTrail.h"
#include "GrGpu.h"
#include "ops/GrDashOp.h"
#include "ops/GrMeshDrawOp.h"

bool GrDashLinePathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
    SkPoint pts[2];
    bool inverted;
    if (args.fShape->style().isDashed() && args.fShape->asLine(pts, &inverted)) {
        if (args.fAAType == GrAAType::kMixedSamples) {
            return false;
        }
        // We should never have an inverse dashed case.
        SkASSERT(!inverted);
        return GrDashOp::CanDrawDashLine(pts, args.fShape->style(), *args.fViewMatrix);
    }
    return false;
}

bool GrDashLinePathRenderer::onDrawPath(const DrawPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fRenderTargetContext->auditTrail(),
                              "GrDashLinePathRenderer::onDrawPath");
    GrDashOp::AAMode aaMode = GrDashOp::AAMode::kNone;
    switch (args.fAAType) {
        case GrAAType::kNone:
            break;
        case GrAAType::kCoverage:
        case GrAAType::kMixedSamples:
            aaMode = GrDashOp::AAMode::kCoverage;
            break;
        case GrAAType::kMSAA:
            // In this mode we will use aa between dashes but the outer border uses MSAA. Otherwise,
            // we can wind up with external edges antialiased and internal edges unantialiased.
            aaMode = GrDashOp::AAMode::kCoverageWithMSAA;
            break;
    }
    SkPoint pts[2];
    SkAssertResult(args.fShape->asLine(pts, nullptr));
    std::unique_ptr<GrDrawOp> op =
            GrDashOp::MakeDashLineOp(std::move(args.fPaint), *args.fViewMatrix, pts, aaMode,
                                     args.fShape->style(), args.fUserStencilSettings);
    if (!op) {
        return false;
    }
    args.fRenderTargetContext->addDrawOp(*args.fClip, std::move(op));
    return true;
}
