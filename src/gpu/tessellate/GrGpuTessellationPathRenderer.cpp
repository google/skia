/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrGpuTessellationPathRenderer.h"

#include "src/core/SkPathPriv.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/geometry/GrShape.h"
#include "src/gpu/tessellate/GrTessellatePathOp.h"

GrPathRenderer::CanDrawPath GrGpuTessellationPathRenderer::onCanDrawPath(
        const CanDrawPathArgs& args) const {
    // This class should not have been added to the chain without tessellation support.
    SkASSERT(args.fCaps->shaderCaps()->tessellationSupport());
    if (!args.fShape->style().isSimpleFill() || args.fShape->inverseFilled() ||
        args.fViewMatrix->hasPerspective()) {
        return CanDrawPath::kNo;
    }
    if (GrAAType::kCoverage == args.fAAType) {
        SkASSERT(1 == args.fProxy->numSamples());
        if (!args.fProxy->canUseMixedSamples(*args.fCaps)) {
            return CanDrawPath::kNo;
        }
    }
    SkPath path;
    args.fShape->asPath(&path);
    if (SkPathPriv::ConicWeightCnt(path)) {
        return CanDrawPath::kNo;
    }
    return CanDrawPath::kYes;
}

bool GrGpuTessellationPathRenderer::onDrawPath(const DrawPathArgs& args) {
    SkPath path;
    args.fShape->asPath(&path);

    auto op = args.fContext->priv().opMemoryPool()->allocate<GrTessellatePathOp>(
            *args.fViewMatrix, path, std::move(args.fPaint), args.fAAType);
    args.fRenderTargetContext->addDrawOp(*args.fClip, std::move(op));

    return true;
}

void GrGpuTessellationPathRenderer::onStencilPath(const StencilPathArgs& args) {
    SkPath path;
    args.fShape->asPath(&path);

    GrAAType aaType = (GrAA::kYes == args.fDoStencilMSAA) ? GrAAType::kMSAA : GrAAType::kNone;

    auto op = args.fContext->priv().opMemoryPool()->allocate<GrTessellatePathOp>(
            *args.fViewMatrix, path, GrPaint(), aaType, GrTessellatePathOp::Flags::kStencilOnly);
    args.fRenderTargetContext->addDrawOp(*args.fClip, std::move(op));
}
