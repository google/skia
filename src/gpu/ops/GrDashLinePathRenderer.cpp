/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrAuditTrail.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/geometry/GrShape.h"
#include "src/gpu/ops/GrDashLinePathRenderer.h"
#include "src/gpu/ops/GrDashOp.h"
#include "src/gpu/ops/GrMeshDrawOp.h"

GrPathRenderer::CanDrawPath
GrDashLinePathRenderer::onCanDrawPath(const CanDrawPathArgs& args) const {
    SkPoint pts[2];
    bool inverted;
    if (args.fShape->style().isDashed() && args.fShape->asLine(pts, &inverted)) {
        if (args.fAATypeFlags == AATypeFlags::kMixedSampledStencilThenCover) {
            return CanDrawPath::kNo;
        }
        // We should never have an inverse dashed case.
        SkASSERT(!inverted);
        if (!GrDashOp::CanDrawDashLine(pts, args.fShape->style(), *args.fViewMatrix)) {
            return CanDrawPath::kNo;
        }
        return CanDrawPath::kYes;
    }
    return CanDrawPath::kNo;
}

bool GrDashLinePathRenderer::onDrawPath(const DrawPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fRenderTargetContext->auditTrail(),
                              "GrDashLinePathRenderer::onDrawPath");
    GrDashOp::AAMode aaMode = GrDashOp::AAMode::kNone;
    if (AATypeFlags::kNone != args.fAATypeFlags) {
        if (AATypeFlags::kMSAA & args.fAATypeFlags) {
            // In this mode we will use aa between dashes but the outer border uses MSAA. Otherwise,
            // we can wind up with external edges antialiased and internal edges unantialiased.
            aaMode = GrDashOp::AAMode::kCoverageWithMSAA;
        } else {
            SkASSERT(AATypeFlags::kCoverage & args.fAATypeFlags);
            aaMode = GrDashOp::AAMode::kCoverage;
        }
    }
    SkPoint pts[2];
    SkAssertResult(args.fShape->asLine(pts, nullptr));
    std::unique_ptr<GrDrawOp> op =
            GrDashOp::MakeDashLineOp(args.fContext, std::move(args.fPaint), *args.fViewMatrix, pts,
                                     aaMode, args.fShape->style(), args.fUserStencilSettings);
    if (!op) {
        return false;
    }
    args.fRenderTargetContext->addDrawOp(*args.fClip, std::move(op));
    return true;
}
