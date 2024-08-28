/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/ganesh/ops/DashLinePathRenderer.h"

#include "include/core/SkPoint.h"
#include "include/gpu/ganesh/GrRecordingContext.h"
#include "include/private/base/SkAssert.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrAuditTrail.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrStyle.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/gpu/ganesh/geometry/GrStyledShape.h"
#include "src/gpu/ganesh/ops/DashOp.h"
#include "src/gpu/ganesh/ops/GrOp.h"

#include <utility>

namespace skgpu::ganesh {

skgpu::ganesh::PathRenderer::CanDrawPath DashLinePathRenderer::onCanDrawPath(
        const CanDrawPathArgs& args) const {
    SkPoint pts[2];
    bool inverted;
    if (args.fShape->style().isDashed() && args.fShape->asLine(pts, &inverted)) {
        // We should never have an inverse dashed case.
        SkASSERT(!inverted);
        if (!DashOp::CanDrawDashLine(pts, args.fShape->style(), *args.fViewMatrix)) {
            return CanDrawPath::kNo;
        }
        return CanDrawPath::kYes;
    }
    return CanDrawPath::kNo;
}

bool DashLinePathRenderer::onDrawPath(const DrawPathArgs& args) {
    GR_AUDIT_TRAIL_AUTO_FRAME(args.fContext->priv().auditTrail(),
                              "DashLinePathRenderer::onDrawPath");
    DashOp::AAMode aaMode;
    switch (args.fAAType) {
        case GrAAType::kNone:
            aaMode = DashOp::AAMode::kNone;
            break;
        case GrAAType::kMSAA:
            // In this mode we will use aa between dashes but the outer border uses MSAA. Otherwise,
            // we can wind up with external edges antialiased and internal edges unantialiased.
            aaMode = DashOp::AAMode::kCoverageWithMSAA;
            break;
        case GrAAType::kCoverage:
            aaMode = DashOp::AAMode::kCoverage;
            break;
    }
    SkPoint pts[2];
    SkAssertResult(args.fShape->asLine(pts, nullptr));
    GrOp::Owner op = DashOp::MakeDashLineOp(args.fContext, std::move(args.fPaint),
                                            *args.fViewMatrix, pts, aaMode, args.fShape->style(),
                                            args.fUserStencilSettings);
    if (!op) {
        return false;
    }
    args.fSurfaceDrawContext->addDrawOp(args.fClip, std::move(op));
    return true;
}

} // namespace skgpu::ganesh
